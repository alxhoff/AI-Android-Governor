
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cpufreq.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/sched/rt.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/syscalls.h>

#include "chrome_governor_kernel_write.h"
#include "AI_gov_phases.h"
#include "AI_gov_ioctl.h"
#include "AI_gov.h"

#define AID_SYSTEM	(1000)

#define DEFAULT_TIMER_RATE (20 * USEC_PER_MSEC)

#define DEFAULT_TARGET_LOAD 90
static unsigned int default_target_loads_AI[] = { DEFAULT_TARGET_LOAD };

static cpumask_t speedchange_cpumask_AI;
static spinlock_t speedchange_cpumask_lock_AI;
static struct mutex gov_lock_AI;

struct cpufreq_AI_governor_tunables *common_tunables_AI;
static struct cpufreq_AI_governor_tunables *tuned_parameters_AI = NULL;
static DEFINE_PER_CPU(struct cpufreq_AI_governor_cpuinfo, cpuinfo);

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

void cpufreq_AI_governor_timer(unsigned long data)
{
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

static void cpufreq_AI_governor_timer_start(
		struct cpufreq_AI_governor_tunables *tunables, int cpu)
{
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

AI_governor AI_gov = {{0}};

void AI_phase_change()
{
	AI_gov.prev_phase = AI_gov.phase;

	//do stuff
}

void AI_coordinator(void)
{

	//phase change
	if (AI_gov.prev_phase!=AI_gov.phase)
		AI_phase_change();

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


static int cpufreq_AI_governor_speedchange_task(void* data){
	cpumask_t tmp_mask;
	unsigned long flags;

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

		//CALCULATE GOVERNOR's ACTIONS
		AI_coordinator();
	}
	return 0;
}

//SYSFS
static ssize_t show_phase_state(
		struct cpufreq_AI_governor_tunables *tunables, char *buf) {
//	int phase = cg_phases_getBrowsingPhase();
//	switch (phase) {
//	case CG_BROWSING_PHASE_IDLE:
//		return sprintf(buf, "%s\n", "IDLE");
//		break;
//	case CG_BROWSING_PHASE_SCROLL:
//		return sprintf(buf, "%s\n", "SCROLL");
//		break;
//	case CG_BROWSING_PHASE_LOAD:
//		return sprintf(buf, "%s\n", "LOAD");
//		break;
//	case CG_BROWSING_PHASE_LOAD_IDLE:
//		return sprintf(buf, "%s\n", "LOAD/IDLE");
//		break;
//	case CG_BROWSING_PHASE_LOAD_SCROLL:
//		return sprintf(buf, "%s\n", "LOAD/SCROLL");
//		break;
//	case CG_BROWSING_PHASE_NO_CHROME:
//		return sprintf(buf, "%s\n", "Chrome n.a.");
//		break;
//	case CG_BROWSING_PHASE_STARTUP:
//		return sprintf(buf, "%s\n", "STARTUP");
//		break;
//	case CG_BROWSING_PHASE_LOAD_INTERMEDIATE:
//		return sprintf(buf, "%s\n", "LOAD/INTERMEDIATE");
//		break;
//	case CG_BROWSING_PHASE_TOUCH_START:
//		return sprintf(buf, "%s\n", "TOUCH/START");
//		break;
//	default:
//		return sprintf(buf, "%s\n", "INVALID");
//		break;
//	}
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
		KERNEL_DEBUG_MSG("[GOVERNOR] Chrome_Governor: path is: %s\ for cpu %d\n", buf, policy->cpu);
	}

	set_fs(oldfs);
	kfree(path);
}

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


	//get tunables
    if (have_governor_per_policy())
			tunables = policy->governor_data;
	else
			tunables = common_tunables_AI;

	switch (event) {
	case CPUFREQ_GOV_POLICY_INIT:
		if(common_tunables_AI){
			tunables->usage_count++;
			policy->governor_data = common_tunables_AI;
			return 0;
		}

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

		policy->governor_data = tunables;

		//initialize char device and phases
		if ((error_ret = AI_governor_ioctl_init()) < 0)
			KERNEL_ERROR_MSG("[GOVERNOR] AI_Governor: "
					"Error initializing char device! Code: %d\n", error_ret);

		KERNEL_DEBUG_MSG(
				" [GOVERNOR] AI_Governor: finished initialization\n");
		break;
	case CPUFREQ_GOV_POLICY_EXIT:
		//CHECK FOR VALID CPU SOMEHOW
		KERNEL_DEBUG_MSG("Entering exit routine, usage count: %d\n",
							tunables->usage_count);
		//remove sysfs
		sysfs_remove_group(get_governor_parent_kobj(policy),get_sysfs_attr());



		break;
	case CPUFREQ_GOV_START:
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

		//Conditional regarding hardware

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

		kthread_bind(tunables->speedchange_task, policy->cpu);

		/* NB: wake up so the thread does not look hung to the freezer */
		wake_up_process(tunables->speedchange_task);

		mutex_unlock(&gov_lock_AI);
		break;
	case CPUFREQ_GOV_STOP:
		mutex_lock(&gov_lock_AI);
		for_each_cpu(j, policy->cpus) {
			pcpu = &per_cpu(cpuinfo, j);
			down_write(&pcpu->enable_sem);
			pcpu->governor_enabled = 0;
			up_write(&pcpu->enable_sem);
		}

		mutex_unlock(&gov_lock_AI);

		//HARDWARE CLEANUP (TODO)
		break;
	case CPUFREQ_GOV_LIMITS:
		pr_debug("setting to %u kHz because of event %u\n",
							policy->min, event);
		__cpufreq_driver_target(policy, policy->min, CPUFREQ_RELATION_L);
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
