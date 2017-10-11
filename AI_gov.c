/**
 * @file AI_gov.c
 * @author Alex Hoffman
 * @date 11 Oct 2017
 */

#include <asm/uaccess.h>

#include <linux/kobject.h>
#include <linux/kernel.h>
#include <linux/kernel_stat.h>

#include <linux/cpufreq.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/percpu-defs.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/tick.h>
#include <linux/types.h>
#include <linux/cpu.h>

#include <linux/cpumask.h>
#include <linux/moduleparam.h>
#include <linux/rwsem.h>
#include <linux/sched.h>
#include <linux/sched/rt.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <asm/cputime.h>

#ifdef CONFIG_ANDROID
#include <asm/uaccess.h>
#include <linux/syscalls.h>
#include <linux/android_aid.h>
#endif
#ifdef CONFIG_ARM_EXYNOS_MP_CPUFREQ
#include <mach/cpufreq.h>
#endif
#include "cpu_load_metric.h"

#include <linux/cpufreq.h>

#include "AI_gov_hardware.h"
#include "AI_gov_sched.h"
#include "AI_gov_ioctl.h"
#include "AI_gov.h"
#include "AI_gov_types.h"
#include "AI_gov_kernel_write.h"
#include "AI_gov_power_manager.h"
#include "AI_gov_task_handling.h"
#include "AI_touch_notifier.h"
#include "AI_gov_task_handling.h"
#include "AI_gov_sysfs.h"

//TODO fix this doxy


#ifndef AID_SYSTEM
#define AID_SYSTEM	(1000)
#endif

#define DEFAULT_TIMER_RATE (20 * USEC_PER_MSEC)
#define DEFAULT_TARGET_LOAD 90

static unsigned int default_target_loads_AI[] = { DEFAULT_TARGET_LOAD };

static DEFINE_PER_CPU(struct cpufreq_AI_governor_cpuinfo, cpuinfo);

static cpumask_t speedchange_cpumask_AI;
static spinlock_t speedchange_cpumask_lock_AI;
static struct mutex gov_lock_AI;

extern uint8_t AI_shutdownCpu;

struct cpufreq_AI_gov_tunables *common_tunables_AI;
struct cpufreq_AI_gov_tunables *tuned_parameters_AI = NULL;
struct AI_gov_info* AI_gov;

/**
* @defgroup initialisation_flags Initialisation flags
*
* These variables are used to make sure that the various subsystems that
* the governor uses are only called once during governor initialisation.
* This is as the governor initialised once per core and this can be hazardous
* to various subsystems such as Sysfs.
*/
/** 
* @brief Governor task initialisation flag
*
* Makes sure that the governor task is only created and initialised once
*
* @ingroup initialisation_flags
*/
static bool gov_started = 0;
/** 
* @brief Profile initialisation flag
*
* Makes sure that the phase profiles and more specifically their sysfs kobjects
* are only initialised once
*
* @ingroup initialisation_flags
*/
bool profiles_initd = false;
/** 
* @brief IOcontrol initialisation flag
* @ingroup initialisation_flags
*/
bool ioctl_initd = false;

/**
 * @brief Called to reschedule the main task's time upon expiration
 *
 **/
void cpufreq_AI_governor_timer_resched(void)
{
	unsigned long expires;
	unsigned long flags;
	struct cpufreq_AI_governor_cpuinfo *pcpu = &per_cpu(cpuinfo, 0);
	struct cpufreq_AI_gov_tunables *tunables =
			pcpu->policy->governor_data;

	if (!tunables->speedchange_task)
		return;

	if (!timer_pending(&pcpu->cpu_timer)) {
		spin_lock_irqsave(&pcpu->load_lock, flags);
		expires = jiffies + usecs_to_jiffies(tunables->timer_rate);
		mod_timer_pinned(&pcpu->cpu_timer, expires);
		spin_unlock_irqrestore(&pcpu->load_lock, flags);
	} else {
		spin_lock_irqsave(&pcpu->load_lock, flags);
		expires = jiffies + usecs_to_jiffies(tunables->timer_rate);
		mod_timer_pending(&pcpu->cpu_timer, expires);
		spin_unlock_irqrestore(&pcpu->load_lock, flags);
	}
}

/**
 * @brief Timer which is called when power management should be
 * re-evaluated
 *
 * @param data - required parameter, not used
 **/
void cpufreq_AI_governor_timer(unsigned long data) {
	struct cpufreq_AI_governor_cpuinfo *pcpu = &per_cpu(cpuinfo, data);
	struct cpufreq_AI_gov_tunables *tunables =
			pcpu->policy->governor_data;

	//KERNEL_DEBUG_MSG(" [GOVERNOR] Timer expired\n");

	if (!down_read_trylock(&pcpu->enable_sem))
		return;
	if (!pcpu->governor_enabled)
		goto exit;

	//KERNEL_DEBUG_MSG(" [GOVERNOR] Timer expired\n");
	wake_up_process(tunables->speedchange_task);

//	KERNEL_DEBUG_MSG(" [GOVERNOR] IN TIMER \n");

	cpufreq_AI_governor_timer_resched();
	exit: up_read(&pcpu->enable_sem);
	return;
}

/**
* @brief Start routine for AI governor's main task timer.
*
* The period that is used to reschedule the main task's timer can be set
* by modifying the timer_rate in the cpufreq_AI_gov_tunables structure
* passed to the function.

@return void
*/
static void cpufreq_AI_governor_timer_start(
	struct cpufreq_AI_gov_tunables *tunables, int cpu) {
	struct cpufreq_AI_governor_cpuinfo *pcpu = &per_cpu(cpuinfo, cpu);
	unsigned long expires = jiffies + usecs_to_jiffies(tunables->timer_rate);

	KERNEL_DEBUG_MSG(
			" [GOVERNOR] AI_Governor: enter timer start, cpu: %d \n", cpu);

	if (!tunables->speedchange_task) {
		KERNEL_DEBUG_MSG(
				" [GOVERNOR] AI_Governor: No tunables->speedchange task \n");
		return;
	}

	pcpu->cpu_timer.expires = expires;

	add_timer_on(&pcpu->cpu_timer, cpu);

	KERNEL_DEBUG_MSG(" [GOVERNOR] AI_Governor: timer start finished \n");
}


/**
 * @brief Main task of the AI governor.
 *
 * Task is responsible for rescheduling itself as well as updating the CPU's
 * frequencies and or status in relation to the currently active backend logic.
 * Also invokes the CPU frequency values to be recalculated such that they can
 * be applied during the next execution of the task.
 *
 * @param data - required parameter, not used
 *
 * @return On sucess returns 0
 **/
static int cpufreq_AI_governor_speedchange_task(void* data){
	cpumask_t tmp_mask;
	unsigned long flags;
	/* 
	* Initialisation flag to ensure that all active CPU's have an initial
	* frequency assigned to them.
	*/
	unsigned long init = 0;

	while(!kthread_should_stop()){
		set_current_state(TASK_INTERRUPTIBLE);
		spin_lock_irqsave(&speedchange_cpumask_lock_AI, flags);

		if (cpumask_empty(&speedchange_cpumask_AI)) {
			spin_unlock_irqrestore(&speedchange_cpumask_lock_AI, flags);
			schedule();

			if (kthread_should_stop())
				break;

			spin_lock_irqsave(&speedchange_cpumask_lock_AI, flags);
		}

		set_current_state(TASK_RUNNING);
		tmp_mask = speedchange_cpumask_AI;
		cpumask_clear(&speedchange_cpumask_AI);
		spin_unlock_irqrestore(&speedchange_cpumask_lock_AI, flags);

		//CARRY OUT GOVERNOR'S CHANGES
		if(init == 0){
#ifdef CPU_IS_BIG_LITTLE
			//update both cores
#else
			//update single core
#endif
			init = 1;
		}

		//CALCULATE GOVERNOR's ACTIONS
		AI_coordinator();

		//rearm timer
		cpufreq_AI_governor_timer_resched();
	}
	return 0;
}


#ifdef CONFIG_ANDROID
static void change_sysfs_owner(struct cpufreq_policy *policy)
{
//	char buf[NAME_MAX];
//		mm_segment_t oldfs;
//		int i;
//		char *path = kobject_get_path(cpufreq_global_kobject,
//				GFP_KERNEL);
//
//		oldfs = get_fs();
//		set_fs(get_ds());
//
//		for (i = 0; i < ARRAY_SIZE(AI_governor_sysfs); i++) {
//			snprintf(buf, sizeof(buf), "/sys%s/AI_governor/%s", path,
//					AI_governor_sysfs[i]);
//			sys_chown(buf, AID_SYSTEM, AID_SYSTEM);
//			KERNEL_DEBUG_MSG("[GOVERNOR] AI_Governor: path is: %s for cpu %d \n", buf, policy->cpu);
//		}
//
//		set_fs(oldfs);
//		kfree(path);
}
#else
static inline void change_sysfs_owner(struct cpufreq_policy *policy) {
}
#endif


static int cpufreq_AI_governor_notifier(struct notifier_block *nfb,
		unsigned long action, void *hcpu) {
	unsigned int cpu = (unsigned long) hcpu;
	struct cpufreq_policy *policy = NULL;

//	printk(KERN_WARNING "[GOVERNOR] successfully starting %d %d\n", cpu, action);
	switch (action) {
	case CPU_ONLINE:
	case CPU_ONLINE_FROZEN:
		policy = cpufreq_cpu_get(cpu);
		if (policy)
			policy->governor_data = common_tunables_AI;
		else
//			printk(KERN_WARNING "[CPUFREQ] could not get policy %d\n", hcpu);
		break;
	case CPU_DOWN_PREPARE:
	case CPU_DOWN_PREPARE_FROZEN:
		break;
	case CPU_DOWN_FAILED:
	case CPU_DOWN_FAILED_FROZEN:
		break;
	}
	return NOTIFY_OK;
}


//static int AI_touch_nb_callback(void)
//{
//	AI_phases_touch_nb();
//	cpufreq_AI_governor_timer_resched(FAST_RESCHEDULE);
//	return 0;
//}

//static struct notifier_block AI_touch_nb = { .notifier_call =
//		AI_touch_nb_callback,
//};



static struct notifier_block cpufreq_notifier_block = { .notifier_call =
		cpufreq_AI_governor_notifier, };

/**
* @brief The main routine of the governor. 
*
* The governor enters and exits through the cases within this function.
*
* @param policy
* @param event
* @return On sucess returns 0
*/
static int cpufreq_governor_AI(struct cpufreq_policy *policy,
					unsigned int event)
{
	int rc = 0;
	int error_ret = 0;
	unsigned int j;
	struct cpufreq_AI_governor_cpuinfo *pcpu = {0};
	struct cpufreq_frequency_table *freq_table;
	struct cpufreq_AI_gov_tunables *tunables =  common_tunables_AI;
	char speedchange_task_name[TASK_NAME_LEN];
	struct sched_param param = { .sched_priority = MAX_RT_PRIO - 1 };

	//TODO CHECK FOR CPUS

	switch (event) {
		/// - Start:
		/// Called after the governor has been initialized.
		/// Creates task, sets task priority and starts task.
	case CPUFREQ_GOV_START:
		mutex_lock(&gov_lock_AI);

		//TODO SAVE INTO STRUCTS
//		freq_table = cpufreq_frequency_get_table(policy->cpu);

		for_each_cpu(j, policy->cpus) {
			pcpu = &per_cpu(cpuinfo, j);
			pcpu->policy = policy;
			pcpu->target_freq = policy->cur;
			pcpu->freq_table = freq_table;
			pcpu->floor_freq = pcpu->target_freq;
			down_write(&pcpu->enable_sem);
			pcpu->governor_enabled = 1;
			up_write(&pcpu->enable_sem);
		}

		//TODO Conditional regarding hardware
		if(tunables && gov_started == 0){

			//create task
			snprintf(speedchange_task_name, TASK_NAME_LEN,
					"AI_governor%d\n", policy->cpu);

			tunables->speedchange_task =  kthread_create(
					cpufreq_AI_governor_speedchange_task, NULL,
					speedchange_task_name);

			//if task errors (THIS CRASHES)
			if (IS_ERR(tunables->speedchange_task)) {
					mutex_unlock(&gov_lock_AI);
					KERNEL_DEBUG_MSG("[GOVERNOR] AI_governor: error with speed change task init\n");
					return PTR_ERR(tunables->speedchange_task);
			}

			//if task is set then set priority
			sched_setscheduler_nocheck(tunables->speedchange_task, SCHED_FIFO,
											&param);

			get_task_struct(tunables->speedchange_task);

			// kthread_bind(tunables->speedchange_task, policy->cpu);
			/* NB: wake up so the thread does not look hung to the freezer */
			wake_up_process(tunables->speedchange_task);

			down_write(&pcpu->enable_sem);

			cpufreq_AI_governor_timer_start(tunables, 0);

			up_write(&pcpu->enable_sem);

			gov_started = 1;
		}
		mutex_unlock(&gov_lock_AI);
		break;
		/// - Stop:
		/// Called when the governor is stopping.
		/// Retsponsible for deinitialising the governor's task and 
		/// removing timers
	case CPUFREQ_GOV_STOP:
		AI_sched_removeCoresFromManaged(0x0F << (policy->cpu / 4) * 4);

		mutex_lock(&gov_lock_AI);
		for_each_cpu(j, policy->cpus) {
			pcpu = &per_cpu(cpuinfo, j);
			down_write(&pcpu->enable_sem);
			pcpu->governor_enabled = 0;
			up_write(&pcpu->enable_sem);
		}
		mutex_unlock(&gov_lock_AI);

		//HARDWARE CLEANUP (TODO)
		if (policy->cpu == L0) {
			mutex_lock(&gov_lock_AI);

			for_each_cpu(j, policy->cpus) {
				pcpu = &per_cpu(cpuinfo, j);
				down_write(&pcpu->enable_sem);
				// we have initialized timers for all cores, so we
				//have to delete them all
				del_timer_sync(&pcpu->cpu_timer);
				up_write(&pcpu->enable_sem);
			}
			// Make sure that the task is stopped only once.
			// Remember: We have started only one task.
			if (j == 0 && tunables && tunables->speedchange_task) {
				kthread_stop(tunables->speedchange_task);
				put_task_struct(tunables->speedchange_task);
				tunables->speedchange_task = NULL;
			}

			mutex_unlock(&gov_lock_AI);

			if (AI_sched_getManagedCores() == 0)
				AI_gov_ioctl_exit();
		}
		break;
	case CPUFREQ_GOV_LIMITS:
		break;
		/// - Init:
		/// Called when the governor is loaded into the kernel.
		/// Initialises governor subsystems and sets up the governor's
		/// HERE
	case CPUFREQ_GOV_POLICY_INIT:

		if(common_tunables_AI && AI_sched_getManagedCores() != 0){
			tunables->usage_count++;
			policy->governor_data = common_tunables_AI;
			AI_sched_addCoresToManaged(0x0F << (policy->cpu / 4) * 4);
			return 0;
		}

		if (AI_sched_getManagedCores() == 0) {

			AI_pm_init_wma_buffers();
			AI_sched_addCoresToManaged(0x0F << (policy->cpu / 4) * 4);
			tunables = kzalloc(sizeof(*tunables), GFP_KERNEL);
			if(!tunables){
				pr_err("%s: POLICY_INIT: kzalloc failed\n", __func__);
				return -ENOMEM;
			}
			KERNEL_DEBUG_MSG(
					"[GOVERNOR] AI_Governor: tuned_parameters_cg before init %x\n",
					(int) tuned_parameters_AI);
			if(tuned_parameters_AI == NULL){
				KERNEL_DEBUG_MSG(
						"[GOVERNOR] AI_Governor: Writing kernel parameters\n");
				tunables->target_loads = default_target_loads_AI;
				tunables->timer_rate = DEFAULT_TIMER_RATE;
			} else {
				// this is only ok if the tunables have been saved before
				// saving happens at exit
				memcpy(tunables, tuned_parameters_AI, sizeof(*tunables));
				kfree(tuned_parameters_AI);
			}
			tunables->usage_count = 1;

			/* update handle for get cpufreq_policy */
			tunables->policy = &policy->policy;
			// we use only one governor for all cpus/cores
			common_tunables_AI = tunables;

			spin_lock_init(&tunables->target_loads_lock);

			if(!profiles_initd){
				//AIGOV INIT
				rc = AI_gov_init(&AI_gov);

				if(rc) KERNEL_DEBUG_MSG(
						"[GOVERNOR] AI_gov_init failed: %d\n", rc);

				AI_gov->cpu_freq_policy = policy;

				rc = AI_phases_init_profiles();

				if(rc) KERNEL_DEBUG_MSG(
						"[GOVERNOR] AI_phases_init_profiles failed: %d\n", rc);

				rc = AI_gov_sysfs_init();

				if(rc) KERNEL_DEBUG_MSG(
						"[GOVERNOR] AI_gov_sysfs_init failed: %d\n", rc);

				profiles_initd = true;
			}


	//		if (!policy->governor->initialized) {
	//			AI_touch_register_notify(&AI_touch_nb);
	//		}

			if (!policy->governor->initialized) {
//				AI_touch_register_notify(&AI_touch_nb);
				//				idle_notifier_register(&cpufreq_AI_governor_idle_nb);
				//register_hotcpu_notifier(&cpufreq_notifier_block);
			}

			policy->governor_data = tunables;

			//initialize char device and phases
			if(ioctl_initd == false){
				if ((error_ret = AI_gov_ioctl_init()) < 0)
					KERNEL_ERROR_MSG("[GOVERNOR] AI_Governor: "
						"Error initializing char device! Code: %d\n", error_ret);
				ioctl_initd = true;
			}
		}
		KERNEL_DEBUG_MSG(
							" [GOVERNOR] AI_Governor: finished initialization\n");
		break;
	case CPUFREQ_GOV_POLICY_EXIT:

		if(policy->cpu == L0){
//			//CHECK FOR VALID CPU SOMEHOW
			KERNEL_DEBUG_MSG("Entering exit routine, usage count: %d\n",
								tunables->usage_count);
//			KERNEL_DEBUG_MSG( "[GOVERNOR] 6\n");
			if (AI_sched_getManagedCores() == 0) {
//				if (policy->governor->initialized == 1) {
////					//unregister_hotcpu_notifier(&cpufreq_notifier_block);
////					//				idle_notifier_unregister(&cpufreq_interactive_idle_nb);
//				}

				//TODO ADD DEINIT
				struct phase_profile* temp_phase;
				FOR_EACH_PHASE(DEINIT_PHASE_KOBJECT);

				AI_gov_ioctl_exit();

				tuned_parameters_AI = kzalloc(sizeof(*tunables), GFP_KERNEL);
				if (!tuned_parameters_AI) {
					pr_err("%s: POLICY_EXIT: kzalloc failed\n", __func__);
					return -ENOMEM;
				}
				memcpy(tuned_parameters_AI, tunables, sizeof(*tunables));
				kfree(tunables);
				common_tunables_AI = NULL;
			}
			policy->governor_data = NULL;
		}
		KERNEL_DEBUG_MSG( "[GOVERNOR] 9\n");
		break;
	default:
		break;
	}
	return 0;
}


// #ifndef CONFIG_CPU_FREQ_DEFAULT_GOV_AI
// static
// #endif
// struct cpufreq_governor cpufreq_gov_AI = {
// 	.name		= "AI",
// 	.governor	= cpufreq_governor_AI,
// 	.owner		= THIS_MODULE,
// };


// static int __init cpufreq_gov_AI_init(void)
// {
// 	unsigned int i;
// 	struct cpufreq_AI_governor_cpuinfo *pcpu;

// 	KERNEL_DEBUG_MSG(" [GOVERNOR] AI_Governor: entering init routine\n");

// 	/* Initalize per-cpu timers */
// 	// do we really need per-core timers?
// 	// The function iterates through all eight cores
// 	for_each_possible_cpu(i) {
// 		pcpu = &per_cpu(cpuinfo, i);
// 		init_timer_deferrable(&pcpu->cpu_timer);
// 		pcpu->cpu_timer.function = cpufreq_AI_governor_timer;
// 		pcpu->cpu_timer.data = i;
// 		//		init_timer(&pcpu->cpu_slack_timer);
// 		spin_lock_init(&pcpu->load_lock);
// 		init_rwsem(&pcpu->enable_sem);
// 		KERNEL_DEBUG_MSG(" [GOVERNOR] AI_Governor: init, show i: %d \n", i);
// 	}

// 	spin_lock_init(&speedchange_cpumask_lock_AI);
// 	mutex_init(&gov_lock_AI);

// 	return cpufreq_register_governor(&cpufreq_gov_AI);
// }


// static void __exit cpufreq_gov_AI_exit(void)
// {
// //	AI_touch_unregister_notify(&AI_touch_nb);
// 	cpufreq_unregister_governor(&cpufreq_gov_AI);
// }

// /// @private
// MODULE_AUTHOR("Alex Hoffman <alxhoff@gmail.com>");
// /// @private
// MODULE_AUTHOR("Tobias Fuchs <tobias.fuchs@tum.de>");
// /// @private
// MODULE_AUTHOR("Nadja Peters <peters@rcs.ei.tum.de>");
// /// @private
// MODULE_DESCRIPTION("CPUfreq policy governor 'AI'");
// /// @private
// MODULE_LICENSE("GPL");

// #ifdef CONFIG_CPU_FREQ_DEFAULT_GOV_AI
// /// @private
// fs_initcall(cpufreq_gov_AI_init);
// #else
// /// @private
// module_init(cpufreq_gov_AI_init);
// #endif
// /// @private
// module_exit(cpufreq_gov_AI_exit);
