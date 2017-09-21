/*
 * AI_gov_sysfs.c
 *
 *  Created on: Sep 20, 2017
 *      Author: alxhoff
 */

#include <linux/slab.h>

#include "AI_gov_sysfs.h"
#include "AI_gov_kernel_write.h"

static struct kobj_attribute phase_attribute =
		__ATTR(phase, 0664, phase_show, phase_store);
static struct kobj_attribute phase_attribute2 =
		__ATTR(prev_phase, 0664, phase_show, phase_store);

static struct attribute *AI_gov_profile_attrs[] = {
		&phase_attribute.attr,
		&phase_attribute2.attr,
		NULL,
};

struct attribute_group AI_gov_profile_attr_grp = {
		//.name = "profile_attr_grp",
		.attrs = AI_gov_profile_attrs,
};

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

//static struct attribute *AI_governor_attributes_gov_sys[] = {
//		&timer_rate_gov_sys.attr, &io_is_busy_gov_sys.attr,
//		&phase_state_gov_sys.attr, NULL, };

static struct attribute_group AI_governor_attr_group_gov_sys = {
		.attrs = AI_governor_attributes_gov_sys,
		.name = NULL,
		//.name = "AI_governor_att_grp",
};

//statuc stuct attribute_group AI_governor_profile_attr_group = { .attrs =
//
//};

const char *AI_governor_sysfs[] = {
	"timer_rate",
	"io_is_busy",
	"phase_state",

};

// we want to manage all cores so it is ok to return the global object


static struct attribute_group *AI_get_sysfs_attr(void)
{
	return &AI_governor_attr_group_gov_sys;
}

static ssize_t show_phase_state(
		struct cpufreq_AI_governor_tunables *tunables, char *buf) {
	int phase = AI_phases_getBrowsingPhase();
	switch (phase) {
	case AI_phase_response:
		return sprintf(buf, "%s\n", "CHILLIN");
		break;
	case AI_phase_animation:
		return sprintf(buf, "%s\n", "ANIMATION");
		break;
	case AI_phase_idle:
		return sprintf(buf, "%s\n", "IDLE");
		break;
	case AI_phase_load:
		return sprintf(buf, "%s\n", "LOAD");
		break;
	default:
		return sprintf(buf, "%s\n", "INVALID");
		break;
	}
}

signed int AI_gov_sysfs_init(struct AI_gov_info* AI_gov)
{
	int ret = 0;

//	static struct kobject* AI_gov_kobj;
//	static struct kobject* AI_gov_profile_kobj;

	//AI_governor parent folder
	AI_gov->kobj = kobject_create_and_add("AI_governor", cpufreq_global_kobject);
	if(!AI_gov->kobj) return -ENOMEM;

	ret = sysfs_create_group(AI_gov->kobj, AI_get_sysfs_attr());
	if (ret) {
		KERNEL_ERROR_MSG("[GOVERNOR]AI_Governor: "
				"Error initializing sysfs! Code: %d\n", ret);
		kobject_put(AI_gov->kobj);
		return ret;
	}

	AI_gov->profile->kobj= kobject_create_and_add("profile",
			AI_gov->kobj);

	if(!AI_gov->profile->kobj) return -ENOMEM;

	ret = sysfs_create_group(AI_gov->profile->kobj, &AI_gov_profile_attr_grp);
	if (ret) {
		KERNEL_ERROR_MSG("[GOVERNOR]AI_Governor: "
				"Error initializing sysfs! Code: %d\n", ret);
		kobject_put(AI_gov->profile->kobj);
		return ret;
	}

	return 0;
}

static ssize_t phase_show(struct kobject *kobj, struct kobj_attribute *attr,
                                         char *buf)
{
	int var = 0;

	if(strcmp(attr->attr.name, "phase") == 0){
		var = (int)AI_gov->phase;
	}else if(strcmp(attr->attr.name, "prev_phase") == 0){
		var = AI_gov->prev_phase;
	}

	return sprintf(buf, "%d\n", var);
}

static ssize_t phase_store(struct kobject *kobj, struct kobj_attribute *attr,
                         const char *buf, size_t count)
{

        int var, ret;
        ret = kstrtoint(buf, 10, &var);
        if(ret<0)
        	return ret;

        if(strcmp(attr->attr.name, "phase") == 0){
			  AI_gov->phase = var;
		}else if(strcmp(attr->attr.name, "prev_phase") == 0){
			  AI_gov->prev_phase = var;
		}

        return count;
}

struct kobject *AI_get_governor_parent_kobj(struct cpufreq_policy *policy)
{
	return cpufreq_global_kobject;
}

