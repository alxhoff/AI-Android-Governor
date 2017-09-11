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

#include "AI_gov_hardware.h"
#include "AI_gov_phases.h"
#include "AI_gov_sched.h"
#include "AI_gov_ioctl.h"
#include "AI_gov.h"
#include "AI_gov_kernel_write.h"
#include "AI_gov_power_manager.h"
#include "AI_gov_task_handling.h"
#include "AI_touch_notifier.h"
#include "AI_gov_task_handling.h"


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

static bool gov_started = 0;

struct cpufreq_AI_governor_tunables *common_tunables_AI;
static struct cpufreq_AI_governor_tunables *tuned_parameters_AI = NULL;
struct AI_gov_info AI_gov = {{0}};

//HARDWARE

//TIMER
void cpufreq_AI_governor_timer_resched(unsigned long expires)
{
	unsigned long flags;
	// load cpu 0
	struct cpufreq_AI_governor_cpuinfo *pcpu = &per_cpu(cpuinfo, 0);
	struct cpufreq_AI_governor_tunables *tunables =
			pcpu->policy->governor_data;

	// rearm the timer
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
 * Timer which is called when power management should be
 * re-evaluated
 *
 * @param data - required parameter, not used
 **/
void cpufreq_AI_governor_timer(unsigned long data) {
	struct cpufreq_AI_governor_cpuinfo *pcpu = &per_cpu(cpuinfo, data);
	struct cpufreq_AI_governor_tunables *tunables =
			pcpu->policy->governor_data;

	//KERNEL_DEBUG_MSG(" [GOVERNOR] Timer expired\n");

	if (!down_read_trylock(&pcpu->enable_sem))
		return;
	if (!pcpu->governor_enabled)
		goto exit;

	//KERNEL_DEBUG_MSG(" [GOVERNOR] Timer expired\n");
	wake_up_process(tunables->speedchange_task);

	cpufreq_AI_governor_timer_resched(tunables->timer_rate);
	exit: up_read(&pcpu->enable_sem);
	return;
}

/* The caller shall take enable_sem write semaphore to avoid any timer race.
 * The cpu_timer and cpu_slack_timer must be deactivated when calling this
 * function.
 */
static void cpufreq_AI_governor_timer_start(
	struct cpufreq_AI_governor_tunables *tunables, int cpu) {
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

	KERNEL_DEBUG_MSG(" [GOVERNOR] AI_Governor: timer start end \n");
}

static ssize_t show_phase_state(
		struct cpufreq_AI_governor_tunables *tunables, char *buf) {
	int phase = AI_phases_getBrowsingPhase();
	switch (phase) {
	case response:
		return sprintf(buf, "%s\n", "RESPONSE");
		break;
	case animation:
		return sprintf(buf, "%s\n", "ANIMATION");
		break;
	case idle:
		return sprintf(buf, "%s\n", "IDLE");
		break;
	case load:
		return sprintf(buf, "%s\n", "LOAD");
		break;
	default:
		return sprintf(buf, "%s\n", "INVALID");
		break;
	}
}


void AI_phase_change(void)
{
	AI_gov.prev_phase = AI_gov.phase;

	//do stuff
}

void AI_coordinator(void)
{
	//phase change
	if (AI_gov.prev_phase!=AI_gov.phase)
		AI_phase_change();

	//get cpu freq
	uint32_t little_freq = AI_gov.hardware.little_freq;

#ifdef CPU_IS_BIG_LITTLE
	uint32_t big_freq = AI_gov.hardware.big_freq;
#endif

	switch(AI_gov.phase){
	case response:
		break;
	case animation:
		break;
	case idle:
		break;
	case load:
		break;
	}
}

static ssize_t store_phase_state(
		struct cpufreq_AI_governor_tunables *tunables, char *buf,
		size_t count) {

	tunables->phase_state = buf;
	return count;
}

static ssize_t show_timer_rate(
		struct cpufreq_AI_governor_tunables *tunables, char *buf) {
	return sprintf(buf, "%lu\n", tunables->timer_rate);
}

static ssize_t store_timer_rate(
		struct cpufreq_AI_governor_tunables *tunables, const char *buf,
		size_t count) {
	int ret;
	unsigned long val;

	ret = strict_strtoul(buf, 0, &val);
	if (ret < 0)
		return ret;
	tunables->timer_rate = val;
	return count;
}

static ssize_t show_io_is_busy(
		struct cpufreq_AI_governor_tunables *tunables, char *buf) {
	return sprintf(buf, "%u\n", tunables->io_is_busy);
}

static ssize_t store_io_is_busy(
		struct cpufreq_AI_governor_tunables *tunables, const char *buf,
		size_t count)
{
	int ret;
	unsigned long val;

	ret = kstrtoul(buf, 0, &val);
	if (ret < 0)
		return ret;
	tunables->io_is_busy = val;
	return count;
}

/*
 * Create show/store routines
 * - sys: One governor instance for complete SYSTEM
 * - pol: One governor instance per struct cpufreq_policy
 */
#define show_gov_pol_sys(file_name)					\
static ssize_t show_##file_name##_gov_sys				\
(struct kobject *kobj, struct attribute *attr, char *buf)		\
{									\
	return show_##file_name(common_tunables_AI, buf);			\
}									\
									\
static ssize_t show_##file_name##_gov_pol				\
(struct cpufreq_policy *policy, char *buf)				\
{									\
	return show_##file_name(policy->governor_data, buf);		\
}

#define store_gov_pol_sys(file_name)					\
static ssize_t store_##file_name##_gov_sys				\
(struct kobject *kobj, struct attribute *attr, const char *buf,		\
	size_t count)							\
{									\
	return store_##file_name(common_tunables_AI, buf, count);		\
}									\
									\
static ssize_t store_##file_name##_gov_pol				\
(struct cpufreq_policy *policy, const char *buf, size_t count)		\
{									\
	return store_##file_name(policy->governor_data, buf, count);	\
}

#define show_store_gov_pol_sys(file_name)				\
show_gov_pol_sys(file_name);						\
store_gov_pol_sys(file_name)

show_store_gov_pol_sys(timer_rate)
;
show_store_gov_pol_sys(io_is_busy)
;
show_store_gov_pol_sys(phase_state)
;
#define gov_sys_attr_rw(_name)						\
static struct global_attr _name##_gov_sys =				\
__ATTR(_name, 0644, show_##_name##_gov_sys, store_##_name##_gov_sys)
#define gov_pol_attr_rw(_name)						\
static struct freq_attr _name##_gov_pol =				\
__ATTR(_name, 0644, show_##_name##_gov_pol, store_##_name##_gov_pol)
#define gov_sys_pol_attr_rw(_name)					\
gov_sys_attr_rw(_name); \
gov_pol_attr_rw(_name);
gov_sys_pol_attr_rw(timer_rate)
;
gov_sys_pol_attr_rw(io_is_busy)
;
gov_sys_pol_attr_rw(phase_state)
;

/* One Governor instance for entire system */
static struct attribute *AI_governor_attributes_gov_sys[] = {
		&timer_rate_gov_sys.attr, &io_is_busy_gov_sys.attr,
		&phase_state_gov_sys.attr, NULL, };

static struct attribute_group AI_governor_attr_group_gov_sys = { .attrs =
		AI_governor_attributes_gov_sys, .name = "AI_governor", };

static const char *AI_governor_sysfs[] = {
	"timer_rate",
	"io_is_busy",
	"phase_state",
};

// we want to manage all cores so it is ok to return the global object
static struct kobject *get_governor_parent_kobj(struct cpufreq_policy *policy) {
	return cpufreq_global_kobject;
}

static struct attribute_group *get_sysfs_attr(void) {
	return &AI_governor_attr_group_gov_sys;
}

static int cpufreq_AI_governor_speedchange_task(void* data){
	cpumask_t tmp_mask;
	unsigned long flags, init = 0;

	while(kthread_should_stop()){
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
		cpufreq_AI_governor_timer_resched(common_tunables_AI->timer_rate);
	}
	return 0;
}

extern uint8_t AI_shutdownCpu;

#ifdef CONFIG_ANDROID
static void change_sysfs_owner(struct cpufreq_policy *policy)
{
	char buf[NAME_MAX];
	mm_segment_t oldfs;
	int i;
	char *path = kobject_get_path(get_governor_parent_kobj(policy),
			GFP_KERNEL);

	oldfs = get_fs();
	set_fs(get_ds());

	for (i = 0; i < ARRAY_SIZE(AI_governor_sysfs); i++) {
		snprintf(buf, sizeof(buf), "/sys%s/AI_governor/%s", path,
				AI_governor_sysfs[i]);
		sys_chown(buf, AID_SYSTEM, AID_SYSTEM);
		KERNEL_DEBUG_MSG("[GOVERNOR] AI_Governor: path is: %s\ for cpu %d\n", buf, policy->cpu);
	}

	set_fs(oldfs);
	kfree(path);
}
#else
static inline void change_sysfs_owner(struct cpufreq_policy *policy) {
}
#endif

static int cpufreq_AI_governor_notifier(struct notifier_block *nfb,
		unsigned long action, void *hcpu) {
	unsigned int cpu = (unsigned long) hcpu;
	struct cpufreq_policy *policy = NULL;

	printk(KERN_WARNING "[GOVERNOR] successfully starting %d %d\n", cpu, action);
	switch (action) {
	case CPU_ONLINE:
	case CPU_ONLINE_FROZEN:
		policy = cpufreq_cpu_get(cpu);
		if (policy)
			policy->governor_data = common_tunables_AI;
		else
			printk(KERN_WARNING "[CPUFREQ] could not get policy %d\n", hcpu);
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

static int AI_touch_nb_callback(void)
{
	AI_phases_touch_nb();
	cpufreq_AI_governor_timer_resched(FAST_RESCHEDULE);
	return 0;
}

static struct notifier_block AI_touch_nb = { .notifier_call =
		AI_touch_nb_callback, };


static struct notifier_block cpufreq_notifier_block = { .notifier_call =
		cpufreq_AI_governor_notifier, };

static int cpufreq_governor_AI(struct cpufreq_policy *policy,
					unsigned int event)
{
	int rc = 0;
	int error_ret = 0;
	unsigned int j;
	struct cpufreq_AI_governor_cpuinfo *pcpu;
	struct cpufreq_frequency_table *freq_table;
	struct cpufreq_AI_governor_tunables *tunables;
	char speedchange_task_name[TASK_NAME_LEN];
	struct sched_param param = { .sched_priority = MAX_RT_PRIO - 1 };

	//TODO CHECK FOR CPUS

	switch (event) {
	case CPUFREQ_GOV_START:
		mutex_lock(&gov_lock_AI);

		freq_table = cpufreq_frequency_get_table(policy->cpu);

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

		//create task
		snprintf(speedchange_task_name, TASK_NAME_LEN,
				"AI_governor%d\n", policy->cpu);
		tunables->speedchange_task =  kthread_create(
				cpufreq_AI_governor_speedchange_task, NULL,
				speedchange_task_name);

		//if task errors
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

		//TODO CAN THIS BE MOVED?
		gov_started = 1;

		mutex_unlock(&gov_lock_AI);
		break;
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
				// we have initialized timers for all cores, so we have to delete them all
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
	case CPUFREQ_GOV_POLICY_INIT:

		//HARDWARE INIT
		

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

			// create sysfs entry
			// the location is /sys/devices/system/cpu/cpufreq/AI_governor
			// speculation: it is in the common folder because we use the governor for all CPUs
			rc = sysfs_create_group(get_governor_parent_kobj(policy),
					get_sysfs_attr());
			if (rc) {
				KERNEL_ERROR_MSG("[GOVERNOR]AI_Governor: "
						"Error initializing sysfs! Code: %d\n", rc);
				kfree(tunables);
				return rc;
			}
			change_sysfs_owner(policy);

	//		if (!policy->governor->initialized) {
	//			AI_touch_register_notify(&AI_touch_nb);
	//		}

			if (!policy->governor->initialized) {
				AI_touch_register_notify(&AI_touch_nb);
				//				idle_notifier_register(&cpufreq_chrome_governor_idle_nb);
				//register_hotcpu_notifier(&cpufreq_notifier_block);
			}

			policy->governor_data = tunables;

			//initialize char device and phases
			if ((error_ret = AI_gov_ioctl_init()) < 0)
				KERNEL_ERROR_MSG("[GOVERNOR] AI_Governor: "
						"Error initializing char device! Code: %d\n", error_ret);

			KERNEL_DEBUG_MSG(
					" [GOVERNOR] AI_Governor: finished initialization\n");
		}
		break;
	case CPUFREQ_GOV_POLICY_EXIT:

		if(policy->cpu == L0){
			//CHECK FOR VALID CPU SOMEHOW
			KERNEL_DEBUG_MSG("Entering exit routine, usage count: %d\n",
								tunables->usage_count);
			if (AI_sched_getManagedCores() == 0) {
				if (policy->governor->initialized == 1) {
					//unregister_hotcpu_notifier(&cpufreq_notifier_block);
					//				idle_notifier_unregister(&cpufreq_interactive_idle_nb);
				}

				sysfs_remove_group(get_governor_parent_kobj(policy),
						get_sysfs_attr());

				tuned_parameters_AI = kzalloc(sizeof(*tunables), GFP_KERNEL);
				if (!tuned_parameters_AI) {
					pr_err("%s: POLICY_EXIT: kzalloc failed\n", __func__);
					return -ENOMEM;
				}
				memcpy(tuned_parameters_AI, tunables, sizeof(*tunables));
				kfree(tunables);
				common_tunables_AI = NULL;
				//				KERNEL_DEBUG_MSG("Cleaned up all data\n");
			}
			policy->governor_data = NULL;
		}
		break;
	default:
		break;
	}
	return 0;
}

#ifndef CONFIG_CPU_FREQ_DEFAULT_GOV_AI
static
#endif
struct cpufreq_governor cpufreq_gov_AI = {
	.name		= "AI",
	.governor	= cpufreq_governor_AI,
	.owner		= THIS_MODULE,
};

static int __init cpufreq_gov_AI_init(void)
{
	unsigned int i;
		struct cpufreq_AI_governor_cpuinfo *pcpu;

		KERNEL_DEBUG_MSG(" [GOVERNOR] AI_Governor: entering init routine\n");

		/* Initalize per-cpu timers */
		// do we really need per-core timers?
		// The function iterates through all eight cores
		for_each_possible_cpu(i) {
			pcpu = &per_cpu(cpuinfo, i);
			init_timer_deferrable(&pcpu->cpu_timer);
			pcpu->cpu_timer.function = cpufreq_AI_governor_timer;
			pcpu->cpu_timer.data = i;
			//		init_timer(&pcpu->cpu_slack_timer);
			spin_lock_init(&pcpu->load_lock);
			init_rwsem(&pcpu->enable_sem);
			KERNEL_DEBUG_MSG(" [GOVERNOR] AI_Governor: init, show i: %d \n", i);
		}

		spin_lock_init(&speedchange_cpumask_lock_AI);
		mutex_init(&gov_lock_AI);
	return cpufreq_register_governor(&cpufreq_gov_AI);
}


static void __exit cpufreq_gov_AI_exit(void)
{
	cpufreq_unregister_governor(&cpufreq_gov_AI);
}


MODULE_AUTHOR("Alex");
MODULE_DESCRIPTION("CPUfreq policy governor 'AI'");
MODULE_LICENSE("GPL");

#ifdef CONFIG_CPU_FREQ_DEFAULT_GOV_AI
fs_initcall(cpufreq_gov_AI_init);
#else
module_init(cpufreq_gov_AI_init);
#endif
module_exit(cpufreq_gov_AI_exit);
