/*
 * AI_gov_sysfs.c
 *
 *  Created on: Sep 20, 2017
 *      Author: alxhoff
 */

#include <linux/slab.h>

#include "AI_gov_sysfs.h"
#include "AI_gov_phases.h"
#include "AI_gov_kernel_write.h"

//ACCESSER FUNCTIONS
//TOP LEVEL
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
	if (ret < 0) return ret;
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

static ssize_t show_phase_state(
		struct cpufreq_AI_governor_tunables *tunables, char *buf) {
	phase_state phase = AI_phases_getBrowsingPhase();
	switch (phase) {
		case AI_phase_init:
			return sprintf(buf, "%s\n", "INIT");
			break;
		case AI_phase_framerate:
			return sprintf(buf, "%s\n", "FRAMERATE");
			break;
		case AI_phase_priority:
			return sprintf(buf, "%s\n", "PRIORITY");
			break;
		case AI_phase_time:
			return sprintf(buf, "%s\n", "TIME");
			break;
		case AI_phase_powersave:
			return sprintf(buf, "%s\n", "POWERSAVE");
			break;
		case AI_phase_performance:
				return sprintf(buf, "%s\n", "PERFORMANCE");
				break;
		case AI_phase_response:
				return sprintf(buf, "%s\n", "RESPONSE");
				break;
		default:
			return sprintf(buf, "%s\n", "INVALID");
			break;
		}
}

static ssize_t store_phase_state(
		struct cpufreq_AI_governor_tunables *tunables, char *buf,
		size_t count) {

	int ret;

	int phase = (int)AI_gov->phase;

	ret= kstrtoint(buf, 10, &phase);

	if(ret < 0) return ret;

	return count;
}

static ssize_t show_prev_phase(
		struct cpufreq_AI_governor_tunables *tunables, char *buf) {
	phase_state phase = AI_phases_getPrevBrowsingPhase();
	switch (phase) {
	case AI_phase_init:
		return sprintf(buf, "%s\n", "INIT");
		break;
	case AI_phase_framerate:
		return sprintf(buf, "%s\n", "FRAMERATE");
		break;
	case AI_phase_priority:
		return sprintf(buf, "%s\n", "PRIORITY");
		break;
	case AI_phase_time:
		return sprintf(buf, "%s\n", "TIME");
		break;
	case AI_phase_powersave:
		return sprintf(buf, "%s\n", "POWERSAVE");
		break;
	case AI_phase_performance:
			return sprintf(buf, "%s\n", "PERFORMANCE");
			break;
	case AI_phase_response:
			return sprintf(buf, "%s\n", "RESPONSE");
			break;
	default:
		return sprintf(buf, "%s\n", "INVALID");
		break;
	}
}

static ssize_t store_prev_phase(
		struct cpufreq_AI_governor_tunables *tunables, char *buf,
		size_t count) {

	int ret;

	int phase = (int)AI_gov->prev_phase;

	ret= kstrtoint(buf, 10, &phase);

	if(ret < 0) return ret;

	return count;
}

//PROFILE

static ssize_t show_min_freq(
		struct AI_gov_profile* profile, const char *buf)
{
	return sprintf(buf, "%lu\n", profile->min_freq);
}

static ssize_t store_min_freq(
		struct AI_gov_profile* profile, const char *buf, size_t count)
{
	int ret;
	unsigned long val;

	ret = kstrtoul(buf, 0, &val);
	if (ret < 0)
		return ret;
	profile->min_freq = val;
	return count;
}

static ssize_t show_max_freq(
		struct AI_gov_profile* profile, const char *buf)
{
	return sprintf(buf, "%lu\n", profile->max_freq);
}

static ssize_t store_max_freq(
		struct AI_gov_profile* profile, const char *buf, size_t count)
{
	int ret;
	unsigned long val;

	ret = strict_strtoul(buf, 0, &val);
	if (ret < 0) return ret;
	profile->max_freq = val;
	return count;
}

static ssize_t show_desired_frame_rate(
		struct AI_gov_profile* profile, const char *buf)
{
	return sprintf(buf, "%lu\n", profile->desired_frame_rate);
}

static ssize_t store_desired_frame_rate(
		struct AI_gov_profile* profile, const char *buf, size_t count)
{
	int ret;
	unsigned int val;

	ret = kstrtoint(buf, 0, &val);
	if (ret < 0)
		return ret;
	profile->desired_frame_rate = val;
	return count;
}

static ssize_t show_current_frame_rate(
		struct AI_gov_profile* profile, const char *buf)
{
	return sprintf(buf, "%u\n", profile->current_frame_rate);
}

static ssize_t store_current_frame_rate(
		struct AI_gov_profile* profile, const char *buf, size_t count)
{
	int ret;
	unsigned int val;

	ret = kstrtoint(buf, 0, &val);
	if (ret < 0)
		return ret;
	profile->current_frame_rate = val;
	return count;
}

//HARDWARE

static ssize_t show_is_big_little(
		struct AI_gov_cur_HW* hardware, const char *buf)
{
	return sprintf(buf, "%u\n", hardware->is_big_little);
}

static ssize_t store_is_big_little(
		struct AI_gov_cur_HW* hardware, const char *buf, size_t count)
{
	int val, ret;

	ret = kstrtoint(buf, 10, &val);
	if (ret < 0) return ret;
	hardware->is_big_little = val;
	return count;
}

static ssize_t show_cpu_count(
		struct AI_gov_cur_HW* hardware, const char *buf)
{
	return sprintf(buf, "%u\n", hardware->cpu_count);
}

static ssize_t store_cpu_count(
		struct AI_gov_cur_HW* hardware, const char *buf, size_t count)
{
	int val, ret;

	ret = kstrtoint(buf, 10, &val);
	if (ret < 0) return ret;
	hardware->cpu_count = val;
	return count;
}

static ssize_t show_little_freq(
		struct AI_gov_cur_HW* hardware, const char *buf)
{
	return sprintf(buf, "%u\n", hardware->little_freq);
}

static ssize_t store_little_freq(
		struct AI_gov_cur_HW* hardware, const char *buf, size_t count)
{
	int ret;
	unsigned long val;

	ret = kstrtoul(buf, 0, &val);
	if (ret < 0) return ret;
	hardware->little_freq = val;
	return count;
}

#ifdef CPU_IS_BIG_LITTLE

static ssize_t show_big_state(
		struct AI_gov_cur_HW* hardware, const char *buf)
{
	return sprintf(buf, "%u\n", hardware->big_state);
}

static ssize_t store_big_state(
		struct AI_gov_cur_HW* hardware, const char *buf, size_t count)
{
	int val, ret;

	ret = kstrtoint(buf, 10, &val);
	if (ret < 0) return ret;
	hardware->big_state = val;
	return count;
}

static ssize_t show_big_freq(
		struct AI_gov_cur_HW* hardware, const char *buf)
{
	return sprintf(buf, "%u\n", hardware->big_freq);
}

static ssize_t store_big_freq(
		struct AI_gov_cur_HW* hardware, const char *buf, size_t count)
{
	int ret;
	unsigned long val;

	ret = kstrtoul(buf, 0, &val);
	if (ret < 0) return ret;
	hardware->big_freq = val;
	return count;
}

#endif

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

#define gov_sys_attr_rw(_name)						\
static struct global_attr _name##_gov_sys =				\
__ATTR(_name, 0644, show_##_name##_gov_sys, store_##_name##_gov_sys)
#define gov_pol_attr_rw(_name)						\
static struct freq_attr _name##_gov_pol =				\
__ATTR(_name, 0644, show_##_name##_gov_pol, store_##_name##_gov_pol)
#define gov_sys_pol_attr_rw(_name)					\
gov_sys_attr_rw(_name); \
gov_pol_attr_rw(_name);

//fucntions
//top level
show_store_gov_pol_sys(timer_rate)
;
show_store_gov_pol_sys(io_is_busy)
;
show_store_gov_pol_sys(phase_state)
;
//new
show_store_gov_pol_sys(prev_phase)
;

//profile
//SHOW
#define show_gov_sys_profile(file_name)					\
static ssize_t show_##file_name##_gov_sys				\
(struct kobject *kobj, struct attribute *attr, char *buf)		\
{									\
	return show_##file_name(AI_gov->profile, buf);			\
}									\

//STORE
#define store_gov_sys_profile(file_name)					\
static ssize_t store_##file_name##_gov_sys				\
(struct kobject *kobj, struct attribute *attr, const char *buf,		\
	size_t count)							\
{									\
	return store_##file_name(AI_gov->profile, buf, count);		\
}									\

#define show_store_gov_profile(file_name)				\
show_gov_sys_profile(file_name);						\
store_gov_sys_profile(file_name)

show_store_gov_profile(min_freq)
;
show_store_gov_profile(max_freq)
;
show_store_gov_profile(desired_frame_rate)
;
show_store_gov_profile(current_frame_rate)
;

//hardware
#define show_gov_sys_hardware(file_name)					\
static ssize_t show_##file_name##_gov_sys				\
(struct kobject *kobj, struct attribute *attr, char *buf)		\
{									\
	return show_##file_name(AI_gov->hardware, buf);			\
}									\

//STORE
#define store_gov_sys_hardware(file_name)					\
static ssize_t store_##file_name##_gov_sys				\
(struct kobject *kobj, struct attribute *attr, const char *buf,		\
	size_t count)							\
{									\
	return store_##file_name(AI_gov->hardware, buf, count);		\
}									\

#define show_store_gov_hardware(file_name)				\
show_gov_sys_hardware(file_name);						\
store_gov_sys_hardware(file_name)

show_store_gov_hardware(is_big_little)
;
show_store_gov_hardware(cpu_count)
;
show_store_gov_hardware(little_freq)
;
#ifdef CPU_IS_BIG_LITTLE
show_store_gov_hardware(big_state)
;
show_store_gov_hardware(big_freq)
;
#endif


//attributes
gov_sys_pol_attr_rw(timer_rate)
;
gov_sys_pol_attr_rw(io_is_busy)
;
gov_sys_pol_attr_rw(phase_state)
;
//new
gov_sys_pol_attr_rw(prev_phase)
;

//profile
#define show_store_gov_pol_sys(file_name)				\
show_gov_pol_sys(file_name);						\
store_gov_pol_sys(file_name)

#define gov_sys_attr_rw_profile(_name)						\
static struct global_attr _name##_gov_sys =				\
__ATTR(_name, 0644, show_##_name##_gov_sys, store_##_name##_gov_sys)

#define gov_sys_pol_attr_rw_profile(_name)					\
gov_sys_attr_rw_profile(_name);

gov_sys_pol_attr_rw_profile(min_freq)
;
gov_sys_pol_attr_rw_profile(max_freq)
;
gov_sys_pol_attr_rw_profile(desired_frame_rate)
;
gov_sys_pol_attr_rw_profile(current_frame_rate)
;

//hardware
#define gov_sys_attr_rw_hardware(_name)						\
static struct global_attr _name##_gov_sys =				\
__ATTR(_name, 0644, show_##_name##_gov_sys, store_##_name##_gov_sys)

#define gov_sys_pol_attr_rw_hardware(_name)					\
gov_sys_attr_rw(_name);

gov_sys_pol_attr_rw_hardware(is_big_little)
;
gov_sys_pol_attr_rw_hardware(cpu_count)
;
gov_sys_pol_attr_rw_hardware(little_freq)
;
#ifdef CPU_IS_BIG_LITTLE
gov_sys_pol_attr_rw_hardware(big_state)
;
gov_sys_pol_attr_rw_hardware(big_freq)
;
#endif

/* One Governor instance for entire system */
static struct attribute *AI_governor_attributes_gov_sys[] = {
		&timer_rate_gov_sys.attr,
		&io_is_busy_gov_sys.attr,
		&phase_state_gov_sys.attr,
		&prev_phase_gov_sys.attr,
		NULL,
};

static struct attribute_group AI_governor_attr_group_gov_sys = {
		.attrs = AI_governor_attributes_gov_sys,
		.name = NULL,
};

const char *AI_governor_sysfs[] = {
	"timer_rate",
	"io_is_busy",
	"phase_state",
	"prev_phase"
};

static struct attribute *AI_gov_attrs_profile[] = {
		&min_freq_gov_sys.attr,
		&max_freq_gov_sys.attr,
		&desired_frame_rate_gov_sys.attr,
		&current_frame_rate_gov_sys.attr,
		NULL,
};

struct attribute_group AI_gov_attrs_grp_profile = {
		.attrs = AI_gov_attrs_profile,
		.name = NULL,
};

const char *AI_governor_sysfs_profile[] = {
	"min_freq",
	"max_freq",
	"desired_frame_rate",
	"current_frame_rate"
};

static struct attribute *AI_gov_attrs_hardware[] = {
		&is_big_little_gov_sys.attr,
		&cpu_count_gov_sys.attr,
		&little_freq_gov_sys.attr,
#ifdef CPU_IS_BIG_LITTLE
		&big_state_gov_sys.attr,
		&big_freq_gov_sys.attr,
#endif
		NULL,
};

struct attribute_group AI_gov_attrs_grp_hardware = {
		.attrs = AI_gov_attrs_hardware,
		.name = NULL,
};

const char *AI_governor_sysfs_hardware[] = {
	"is_big_little",
	"cpu_count",
	"little_freq",
#ifdef CPU_IS_BIG_LITTLE
	"big_state",
	"big_freq",
#endif
};


static struct attribute_group *AI_get_sysfs_attr(void)
{
	return &AI_governor_attr_group_gov_sys;
}


signed int AI_gov_sysfs_init(struct AI_gov_info* AI_gov, struct phase_profiles* AI_gov_profiles)
{
	int ret = 0;

	//AI_governor parent folder
	AI_gov->kobj = kobject_create_and_add("AI_governor",
			cpufreq_global_kobject);

	if(!AI_gov->kobj) return -ENOMEM;

	ret = sysfs_create_group(AI_gov->kobj, AI_get_sysfs_attr());
	if (ret) {
		KERNEL_ERROR_MSG("[GOVERNOR]AI_Governor: "
				"Error initializing sysfs! Code: %d\n", ret);
		kobject_put(AI_gov->kobj);
		return ret;
	}

	//profile subdirectory
	AI_gov->profile->kobj = kobject_create_and_add("profile",
			AI_gov->kobj);

	if(!AI_gov->profile->kobj) return -ENOMEM;

	ret = sysfs_create_group(AI_gov->profile->kobj,
			&AI_gov_attrs_grp_profile);

	if (ret) {
		KERNEL_ERROR_MSG("[GOVERNOR]AI_Governor: "
				"Error initializing profile sysfs! Code: %d\n", ret);
		kobject_put(AI_gov->profile->kobj);
		return ret;
	}

	//hardware subdirectory
	AI_gov->hardware->kobj = kobject_create_and_add("hardware",
			AI_gov->kobj);

	if(!AI_gov->hardware->kobj) return -ENOMEM;

	ret = sysfs_create_group(AI_gov->hardware->kobj,
			&AI_gov_attrs_grp_hardware);

	if (ret) {
		KERNEL_ERROR_MSG("[GOVERNOR]AI_Governor: "
				"Error initializing hardware sysfs! Code: %d\n", ret);
		kobject_put(AI_gov->hardware->kobj);
		return ret;
	}

	return 0;
}

static ssize_t phase_show(struct kobject *kobj, struct kobj_attribute *attr,
                                         char *buf)
{
	int val = 0;

	if(strcmp(attr->attr.name, "phase") == 0){
		val = (int)AI_gov->phase;
	}else if(strcmp(attr->attr.name, "prev_phase") == 0){
		val = AI_gov->prev_phase;
	}

	return sprintf(buf, "%d\n", val);
}

static ssize_t phase_store(struct kobject *kobj, struct kobj_attribute *attr,
                         const char *buf, size_t count)
{

        int val, ret;
        ret = kstrtoint(buf, 10, &val);
        if(ret<0)
        	return ret;

        if(strcmp(attr->attr.name, "phase") == 0){
			  AI_gov->phase = val;
		}else if(strcmp(attr->attr.name, "prev_phase") == 0){
			  AI_gov->prev_phase = val;
		}

        return count;
}

struct kobject *AI_get_governor_parent_kobj(struct cpufreq_policy *policy)
{
	return cpufreq_global_kobject;
}

