/**
 * @file AI_gov_sysfs.c
 * @author Alex Hoffman
 * @date 10 March 2018 
 * @brief sysfs interface for the AI_governor.
 */

/* -- Includes -- */
/* Kernel includes. */
#include <linux/slab.h>
#include <linux/string.h>

/* Governor includes. */
#include "AI_gov_sysfs.h"
#include "AI_gov_phases.h"
#include "AI_gov_kernel_write.h"

/**
* @defgroup sysfs_module Sysfs subsystem
* @brief All attributes and accessor funcitons to use the
* sysfs subsystem
*
* The governor utilizes sysfs to export usefull attributes
* and states of the governor from kernel space. While properties
* of the governor can be changed through sysfs it is designed
* to be used as a kernel level interface to the governor as well
* as a debuggin interface. All application interaction should
* be done through IOcontrol as there are a number of safegards
* in place through IOcontrol.
*/

/**
* @defgroup sysfs_top_level Top level Sysfs attributes
* @ingroup sysfs_module
*
* These attributes are the highest level attributes for the
* governor, they are not profile specific and are always
* applicable to the current state and functionality of the
* governor.
*/

/**
* @brief Shows the current timer rate
* @ingroup sysfs_top_level
*
* Shows the current timer rate when called by "cat timer_rate"
* within the governor's top level sysfs directory.
*
* @param tunables Governor tunables struct where the timer 
* rate is stored
* @param buf The output buffer used to display test to the
* terminal
* @return On success, the total number of characters written is returned
*/
static ssize_t show_timer_rate(
		struct cpufreq_AI_gov_tunables *tunables, char *buf)
{
	return sprintf(buf, "%lu\n", tunables->timer_rate);
}

/**
* @brief Stores the current timer rate
* @ingroup sysfs_top_level
*
* Stores the input value into the current timer rate  
* when set via "echo XX > timer_rate" from within the 
* governor's top level sysfs directory.
*
* @param tunables Governor tunables struct where the timer 
* rate is stored
* @param buf Command line input buffer
* @param count TODO
* @return count
*/
static ssize_t store_timer_rate(
		struct cpufreq_AI_gov_tunables *tunables, const char *buf,
		size_t count)
{
	int ret;
	unsigned long val;

	ret = strict_strtoul(buf, 0, &val);
	if (ret < 0) return ret;
	tunables->timer_rate = val;
	return count;
}

/**
* @brief Shows the current io state
* @ingroup sysfs_top_level
*
* Shows the current io state when called by "cat io_is_busy"
* within the governor's top level sysfs directory.
*
* @param tunables Governor tunables struct where the timer 
* rate is stored
* @param buf The output buffer used to display test to the
* terminal
* @return On success, the total number of characters written is returned
*/
static ssize_t show_io_is_busy(
		struct cpufreq_AI_gov_tunables *tunables, char *buf)
{
	return sprintf(buf, "%u\n", tunables->io_is_busy);
}

/**
* @brief Stores the current io state
* @ingroup sysfs_top_level
*
* Stores the input value into the io state 
* when set via "echo XX > io_is_busy" from within the 
* governor's top level sysfs directory.
*
* @param tunables Governor tunables struct where the timer 
* rate is stored
* @param buf Command line input buffer
* @param count TODO
* @return count
*/
static ssize_t store_io_is_busy(
		struct cpufreq_AI_gov_tunables *tunables, const char *buf,
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

/**
* @brief Shows the current phase state
* @ingroup sysfs_top_level
*
* Shows the current timer rate when called by "cat phase_state"
* within the governor's top level sysfs directory.
*
* @param tunables Governor tunables struct where the timer 
* rate is stored
* @param buf The output buffer used to display test to the
* terminal
* @return On success, the total number of characters written is returned
*/
ssize_t show_phase_state(
		struct cpufreq_AI_gov_tunables *tunables, char *buf)
{
	enum PHASE_ENUM phase = AI_phases_getBrowsingPhase();

	KERNEL_DEBUG_MSG(
				"[GOVERNOR] Showing profile %d\n", (int)phase);

	switch (AI_gov->phase) {
		case AI_init:
			return sprintf(buf, "%s\n", "AI_init");
			break;
		case AI_framerate:
			return sprintf(buf, "%s\n", "AI_framerate");
			break;
		case AI_ondemand:
			return sprintf(buf, "%s\n", "AI_ondemand");
			break;
		case AI_priority:
			return sprintf(buf, "%s\n", "AI_priority");
			break;
		case AI_time:
			return sprintf(buf, "%s\n", "AI_time");
			break;
		case AI_powersave:
			return sprintf(buf, "%s\n", "AI_powersave");
			break;
		case AI_performance:
			return sprintf(buf, "%s\n", "AI_performance");
			break;
		case AI_response:
			return sprintf(buf, "%s\n", "AI_response");
			break;
		case AI_exit:
			return sprintf(buf, "%s\n", "AI_exit");
			break;
		default:
			return sprintf(buf, "%s\n", "INVALID");
			break;
		}
}

/**
* @brief Stores the current phase state
* @ingroup sysfs_top_level
*
* Stores the input value into the phase state  
* when set via "echo XX > phase_state" from within the 
* governor's top level sysfs directory.
*
* @param tunables Governor tunables struct where the timer 
* rate is stored
* @param buf Command line input buffer
* @param count TODO
* @return count
*/
ssize_t store_phase_state(
		struct cpufreq_AI_gov_tunables *tunables,const char *buf,
		size_t count) 
{

	if(strcmp(buf, "AI_init\n") == 0){
		KERNEL_ERROR_MSG(
			"[GOVERNOR] STORING STATE: %s \n", buf);
		AI_gov_sysfs_load_profile(AI_init);
	}else if(strcmp(buf, "AI_framerate\n") == 0){
		KERNEL_ERROR_MSG(
			"[GOVERNOR] STORING STATE: %s \n", buf);
		AI_gov_sysfs_load_profile(AI_framerate);
	}else if(strcmp(buf, "AI_ondemand\n") == 0){
		KERNEL_ERROR_MSG(
			"[GOVERNOR] STORING STATE: %s \n", buf);
		AI_gov_sysfs_load_profile(AI_ondemand);
	}else if(strcmp(buf, "AI_priority\n") == 0){
		KERNEL_ERROR_MSG(
			"[GOVERNOR] STORING STATE: %s \n", buf);
		AI_gov_sysfs_load_profile(AI_priority);
	}else if(strcmp(buf, "AI_time\n") == 0){
		KERNEL_ERROR_MSG(
			"[GOVERNOR] STORING STATE: %s \n", buf);
		AI_gov_sysfs_load_profile(AI_time);
	}else if(strcmp(buf, "AI_powersave\n") == 0){
		KERNEL_ERROR_MSG(
			"[GOVERNOR] STORING STATE: %s \n", buf);
		AI_gov_sysfs_load_profile(AI_powersave);
	}else if(strcmp(buf, "AI_performance\n") == 0){
		KERNEL_ERROR_MSG(
			"[GOVERNOR] STORING STATE: %s \n", buf);
		AI_gov_sysfs_load_profile(AI_performance);
	}else if(strcmp(buf, "AI_response\n") == 0){
		KERNEL_ERROR_MSG(
			"[GOVERNOR] STORING STATE: %s \n", buf);
		AI_gov_sysfs_load_profile(AI_response);
	}else if(strcmp(buf, "AI_exit\n") == 0){
		KERNEL_ERROR_MSG(
			"[GOVERNOR] STORING STATE: %s \n", buf);
		AI_gov_sysfs_load_profile(AI_exit);
	}

	KERNEL_ERROR_MSG(
			"[GOVERNOR] CURRENT STATE: %s \n",
					AI_gov->current_profile->phase_name);

	return count;
}

/**
* @brief Shows the previous phase
* @ingroup sysfs_top_level
*
* Shows the current timer rate when called by "cat prev_phase"
* within the governor's top level sysfs directory
*
* @param tunables Governor tunables struct where the timer 
* rate is stored
* @param buf The output buffer used to display test to the
* terminal
* @return On success, the total number of characters written is returned
*/
ssize_t show_prev_phase(
		struct cpufreq_AI_gov_tunables *tunables, char *buf) 
{
	switch (AI_gov->prev_phase) {
		case AI_init:
			return sprintf(buf, "%s\n", "AI_init");
			break;
		case AI_framerate:
			return sprintf(buf, "%s\n", "AI_framerate");
			break;
		case AI_ondemand:
			return sprintf(buf, "%s\n", "AI_ondemand");
			break;
		case AI_priority:
			return sprintf(buf, "%s\n", "AI_priority");
			break;
		case AI_time:
			return sprintf(buf, "%s\n", "AI_time");
			break;
		case AI_powersave:
			return sprintf(buf, "%s\n", "AI_powersave");
			break;
		case AI_performance:
			return sprintf(buf, "%s\n", "AI_performance");
			break;
		case AI_response:
			return sprintf(buf, "%s\n", "AI_response");
			break;
		case AI_exit:
			return sprintf(buf, "%s\n", "AI_exit");
			break;
		default:
			return sprintf(buf, "%s\n", "INVALID");
			break;
		}
}

/** 
* @brief Stores the previous phase
* @ingroup sysfs_top_level
*
* Stores the input value into the previous phase 
* when set via "echo XX > pre_phase" from within the 
* governor's top level sysfs directory.
*
* @param tunables Governor tunables struct where the timer 
* rate is stored
* @param buf Command line input buffer
* @param count TODO
* @return count
*/
ssize_t store_prev_phase(
		struct cpufreq_AI_gov_tunables *tunables, const char *buf,
		size_t count) 
{

	return count;
}

/**
* @defgroup sysfs_hardware Hardare subdirectory of the sysfs heirarchy
* @ingroup sysfs_module
*
* These attributes represent the systems current hardware values
* that are of interest to the governor. Stored within the hardware
* subdirectory in the governors sysfs directory.
*/
/**
* @brief Shows if the system has a BIGlittle CPU architecture
* @ingroup sysfs_hardware
*
* Shows the boolean value that represents if the systems has a
* BIGlittle CPU architecture.
*
* @param hardware The current AI governor's hardware struct
* @param buf Command line input buffer
* @return On success, the total number of characters written is returned
*/
static ssize_t show_is_big_little(
		struct AI_gov_cur_HW* hardware, char *buf)
{
	return sprintf(buf, "%u\n", hardware->is_big_little);
}

static ssize_t store_is_big_little(
		struct AI_gov_cur_HW* hardware, const char *buf, size_t count)
{
//	int val, ret;
//
//	ret = kstrtoint(buf, 10, &val);
//	if (ret < 0) return ret;
//	hardware->is_big_little = val;
	return count;
}

/**
* @brief Shows the CPU count
* @ingroup sysfs_hardware
*
* Shows the number of CPUs currently registered with the governor
*
* @param hardware The current AI governor's hardware struct
* @param buf Command line input buffer
* @return On success, the total number of characters written is returned
*/
static ssize_t show_cpu_count(
		struct AI_gov_cur_HW* hardware, char *buf)
{
	return sprintf(buf, "%u\n", hardware->cpu_count);
}

static ssize_t store_cpu_count(
		struct AI_gov_cur_HW* hardware, const char *buf, size_t count)
{
//	int val, ret;
//
//	ret = kstrtoint(buf, 10, &val);
//	if (ret < 0) return ret;
//	hardware->cpu_count = val;
	return count;
}

/**
* @brief Shows the frequency of the little CPU
* @ingroup sysfs_hardware
*
* The little CPU is assumed to be present in all systems.
* A BIGlittle system will have both a BIG CPU and a little CPU
* while a system such as a desktop computer with a single CPU
* can be though of as having only the little CPU. 
*
* @param hardware The current AI governor's hardware struct
* @param buf Command line input buffer
* @return On success, the total number of characters written is returned
*/
static ssize_t show_little_freq(
		struct AI_gov_cur_HW* hardware, char *buf)
{
	return sprintf(buf, "%u\n", hardware->little_freq);
}

static ssize_t store_little_freq(
		struct AI_gov_cur_HW* hardware, const char *buf, size_t count)
{
//	int ret;
//	unsigned long val;
//
//	ret = kstrtoul(buf, 0, &val);
//	if (ret < 0) return ret;
//	hardware->little_freq = val;
	return count;
}

#ifdef CPU_IS_BIG_LITTLE

/**
* @brief Shows the current state of the BIG CPU
* @ingroup sysfs_hardware
*
* Show the state of the BIG CPU, if it is active or not.
*
* @param hardware The current AI governor's hardware struct
* @param buf Command line input buffer
* @return On success, the total number of characters written is returned
*/
static ssize_t show_big_state(
		struct AI_gov_cur_HW* hardware, char *buf)
{
	return sprintf(buf, "%u\n", hardware->big_state);
}

static ssize_t store_big_state(
		struct AI_gov_cur_HW* hardware, const char *buf, size_t count)
{
//	int val, ret;
//
//	ret = kstrtoint(buf, 10, &val);
//	if (ret < 0) return ret;
//	hardware->big_state = val;
	return count;
}

/**
* @brief Shows the freuquency of the BIG CPU.
* @ingroup sysfs_hardware
*
* Only applicable for BIGlittle systems.
*
* @param hardware The current AI governor's hardware struct
* @param buf Command line input buffer
* @return On success, the total number of characters written is returned
*/
static ssize_t show_big_freq(
		struct AI_gov_cur_HW* hardware, char *buf)
{
	return sprintf(buf, "%u\n", hardware->big_freq);
}

static ssize_t store_big_freq(
		struct AI_gov_cur_HW* hardware, const char *buf, size_t count)
{
//	int ret;
//	unsigned long val;
//
//	ret = kstrtoul(buf, 0, &val);
//	if (ret < 0) return ret;
//	hardware->big_freq = val;
	return count;
}

#endif

/*
 * Create show/store routines
 * - sys: One gov instance for complete SYSTEM
 * - pol: One gov instance per struct cpufreq_policy
 */
#define show_gov_pol_sys(file_name)					\
static ssize_t show_##file_name##_gov_sys				\
(struct kobject *kobj, struct attribute *attr, char *buf)		\
{									\
	return show_##file_name(common_tunables_AI, buf);			\
}									\


#define store_gov_pol_sys(file_name)					\
static ssize_t store_##file_name##_gov_sys				\
(struct kobject *kobj, struct attribute *attr, const char *buf,		\
	size_t count)							\
{									\
	return store_##file_name(common_tunables_AI, buf, count);		\
}									\

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
static struct attribute *AI_gov_attributes_gov_sys[] = {
		&timer_rate_gov_sys.attr,
		&io_is_busy_gov_sys.attr,
		&phase_state_gov_sys.attr,
		&prev_phase_gov_sys.attr,
		NULL,
};

static struct attribute_group AI_gov_attr_group_gov_sys = {
		.attrs = AI_gov_attributes_gov_sys,
		.name = NULL,
};

const char *AI_gov_sysfs[] = {
	"timer_rate",
	"io_is_busy",
	"phase_state",
	"prev_phase"
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

const char *AI_gov_sysfs_hardware[] = {
	"is_big_little",
	"cpu_count",
	"little_freq",
#ifdef CPU_IS_BIG_LITTLE
	"big_state",
	"big_freq",
#endif
};

//INIT
static ssize_t show_AI_init_initialized_attribute(char* buf)
{
	return sprintf(buf, "%d\n",
			GET_ATTRIBUTES_W_PROFILE(AI_init,AI_gov->current_profile)->initialized);
}

static ssize_t store_AI_init_initialized_attribute(const char* buf, size_t count)
{
	return 0;
}

//FRAMERATE
static ssize_t show_AI_framerate_desired_framerate_attribute(char* buf)
{
	return sprintf(buf, "%d\n",
		GET_ATTRIBUTES_W_PROFILE(AI_framerate,AI_gov->current_profile)->desired_framerate);
}

static ssize_t store_AI_framerate_desired_framerate_attribute(const char* buf, size_t count)
{
	int var, ret;

	ret = kstrtoint(buf, 10, &var);
	if(ret < 0) return ret;

	GET_ATTRIBUTES_W_PROFILE(AI_framerate,AI_gov->current_profile)->desired_framerate = var;

	return count;
}

static ssize_t show_AI_framerate_current_framerate_attribute(char* buf)
{
	return sprintf(buf, "%d\n",
		GET_ATTRIBUTES_W_PROFILE(AI_framerate,AI_gov->current_profile)->current_frametate);
}

static ssize_t store_AI_framerate_current_framerate_attribute(const char* buf, size_t count)
{
	int var, ret;

	ret = kstrtoint(buf, 10, &var);
	if(ret < 0) return ret;

	GET_ATTRIBUTES_W_PROFILE(AI_framerate,AI_gov->current_profile)->current_frametate = var;

	return count;
}

//ONDEMAND
static ssize_t show_AI_ondemand_sampling_rate_attribute(char* buf)
{
	return sprintf(buf, "%d\n",
		GET_ATTRIBUTES_W_PROFILE(AI_ondemand, AI_gov->current_profile)->sampling_rate);
}

static ssize_t store_AI_ondemand_sampling_rate_attribute(const char* buf, size_t count)
{
	int var, ret;

	ret = kstrtoint(buf, 10, &var);
	if(ret < 0) return ret;

	GET_ATTRIBUTES_W_PROFILE(AI_ondemand,AI_gov->current_profile)->sampling_rate = var;

	return count;
}

static ssize_t show_AI_ondemand_io_is_busy_attribute(char* buf)
{
	return sprintf(buf, "%d\n",
		GET_ATTRIBUTES_W_PROFILE(AI_ondemand, AI_gov->current_profile)->io_is_busy);
}

static ssize_t store_AI_ondemand_io_is_busy_attribute(const char* buf, size_t count)
{
	int var, ret;

	ret = kstrtoint(buf, 10, &var);
	if(ret < 0) return ret;

	GET_ATTRIBUTES_W_PROFILE(AI_ondemand,AI_gov->current_profile)->io_is_busy = var;

	return count;
}

static ssize_t show_AI_ondemand_up_threshold_attribute(char* buf)
{
	return sprintf(buf, "%d\n",
		GET_ATTRIBUTES_W_PROFILE(AI_ondemand, AI_gov->current_profile)->up_threshold);
}

static ssize_t store_AI_ondemand_up_threshold_attribute(const char* buf, size_t count)
{
	int var, ret;

	ret = kstrtoint(buf, 10, &var);
	if(ret < 0) return ret;

	GET_ATTRIBUTES_W_PROFILE(AI_ondemand,AI_gov->current_profile)->up_threshold = var;

	return count;
}

static ssize_t show_AI_ondemand_sampling_down_factor_attribute(char* buf)
{
	return sprintf(buf, "%d\n",
		GET_ATTRIBUTES_W_PROFILE(AI_ondemand, AI_gov->current_profile)->sampling_down_factor);
}

static ssize_t store_AI_ondemand_sampling_down_factor_attribute(const char* buf, size_t count)
{
	int var, ret;

	ret = kstrtoint(buf, 10, &var);
	if(ret < 0) return ret;

	GET_ATTRIBUTES_W_PROFILE(AI_ondemand,AI_gov->current_profile)->sampling_down_factor = var;

	return count;
}

static ssize_t show_AI_ondemand_ignore_nice_load_attribute(char* buf)
{
	return sprintf(buf, "%d\n",
		GET_ATTRIBUTES_W_PROFILE(AI_ondemand, AI_gov->current_profile)->ignore_nice_load);
}

static ssize_t store_AI_ondemand_ignore_nice_load_attribute(const char* buf, size_t count)
{
	int var, ret;

	ret = kstrtoint(buf, 10, &var);
	if(ret < 0) return ret;

	GET_ATTRIBUTES_W_PROFILE(AI_ondemand,AI_gov->current_profile)->ignore_nice_load = var;

	return count;
}

static ssize_t show_AI_ondemand_powersave_bias_attribute(char* buf)
{
	return sprintf(buf, "%d\n",
		GET_ATTRIBUTES_W_PROFILE(AI_ondemand, AI_gov->current_profile)->powersave_bias);
}

static ssize_t store_AI_ondemand_powersave_bias_attribute(const char* buf, size_t count)
{
	int var, ret;

	ret = kstrtoint(buf, 10, &var);
	if(ret < 0) return ret;

	GET_ATTRIBUTES_W_PROFILE(AI_ondemand,AI_gov->current_profile)->powersave_bias = var;

	return count;
}

static ssize_t show_AI_ondemand_sampling_rate_min_attribute(char* buf)
{
	return sprintf(buf, "%d\n",
		GET_ATTRIBUTES_W_PROFILE(AI_ondemand, AI_gov->current_profile)->sampling_rate_min);
}

static ssize_t store_AI_ondemand_sampling_rate_min_attribute(const char* buf, size_t count)
{
	int var, ret;

	ret = kstrtoint(buf, 10, &var);
	if(ret < 0) return ret;

	GET_ATTRIBUTES_W_PROFILE(AI_ondemand,AI_gov->current_profile)->sampling_rate_min = var;

	return count;
}
//PRIORITY
static ssize_t show_AI_priority_priority_scalar_attribute(char* buf)
{
	return sprintf(buf, "%d\n",
		GET_ATTRIBUTES_W_PROFILE(AI_priority,AI_gov->current_profile)->priority_scalar);
}

static ssize_t store_AI_priority_priority_scalar_attribute(const char* buf, size_t count)
{
	int var, ret;

	ret = kstrtoint(buf, 10, &var);
	if(ret < 0) return ret;

	GET_ATTRIBUTES_W_PROFILE(AI_priority,AI_gov->current_profile)->priority_scalar = var;

	return count;
}

static ssize_t show_AI_priority_minimum_priority_attribute(char* buf)
{
	return sprintf(buf, "%d\n",
		GET_ATTRIBUTES_W_PROFILE(AI_priority,AI_gov->current_profile)->minimum_priority);
}

static ssize_t store_AI_priority_minimum_priority_attribute(const char* buf, size_t count)
{
	int var, ret;

	ret = kstrtoint(buf, 10, &var);
	if(ret < 0) return ret;

	GET_ATTRIBUTES_W_PROFILE(AI_priority,AI_gov->current_profile)->minimum_priority = var;

	return count;
}

static ssize_t show_AI_priority_maximum_priority_attribute(char* buf)
{
	return sprintf(buf, "%d\n",
		GET_ATTRIBUTES_W_PROFILE(AI_priority,AI_gov->current_profile)->maximum_priority);
}

static ssize_t store_AI_priority_maximum_priority_attribute(const char* buf, size_t count)
{
	int var, ret;

	ret = kstrtoint(buf, 10, &var);
	if(ret < 0) return ret;

	GET_ATTRIBUTES_W_PROFILE(AI_priority,AI_gov->current_profile)->maximum_priority = var;

	return count;
}

//TIME

static ssize_t show_AI_time_time_till_completion_attribute(char* buf)
{
	return sprintf(buf, "%lu\n",
		GET_ATTRIBUTES_W_PROFILE(AI_time, AI_gov->current_profile)->time_till_completion);
}

static ssize_t store_AI_time_time_till_completion_attribute(const char* buf, size_t count)
{
	long var;
	int ret;

	ret = kstrtoul(buf, 10, &var);
	if(ret < 0) return ret;

	GET_ATTRIBUTES_W_PROFILE(AI_time,AI_gov->current_profile)->time_till_completion = var;

	return count;
}

static ssize_t show_AI_time_time_at_completion_attribute(char* buf)
{
	return sprintf(buf, "%lu\n",
			GET_ATTRIBUTES_W_PROFILE(AI_time,AI_gov->current_profile)->time_at_completion);
}

static ssize_t store_AI_time_time_at_completion_attribute(const char* buf, size_t count)
{
	long var;
	int ret;

	ret = kstrtoul(buf, 10, &var);
	if(ret < 0) return ret;

	GET_ATTRIBUTES_W_PROFILE(AI_time,AI_gov->current_profile)->time_at_completion = var;

	return count;
}

static ssize_t show_AI_time_alarm_mode_attribute(char* buf)
{
	return sprintf(buf, "%d\n",
			GET_ATTRIBUTES_W_PROFILE(AI_time,AI_gov->current_profile)->alarm_mode);
}

static ssize_t store_AI_time_alarm_mode_attribute(const char* buf, size_t count)
{
	int var, ret;

	ret = kstrtoint(buf, 10, &var);
	if(ret < 0) return ret;

	GET_ATTRIBUTES_W_PROFILE(AI_time,AI_gov->current_profile)->alarm_mode = var;

	return count;
}

//POWERSAVE
static ssize_t show_AI_powersave_initialized_attribute(char* buf)
{
	return sprintf(buf, "%d\n",
			GET_ATTRIBUTES_W_PROFILE(AI_powersave,AI_gov->current_profile)->initialized);
}

static ssize_t store_AI_powersave_initialized_attribute(const char* buf, size_t count)
{
	return 0;
}

//PERFORMANCE
static ssize_t show_AI_performance_initialized_attribute(char* buf)
{
	return sprintf(buf, "%d\n",
			GET_ATTRIBUTES_W_PROFILE(AI_performance,AI_gov->current_profile)->initialized);
}

static ssize_t store_AI_performance_initialized_attribute(const char* buf, size_t count)
{
	return 0;
}

//RESPONSE
static ssize_t show_AI_response_user_input_importance_attribute(char* buf)
{
	return sprintf(buf, "%d\n",
			GET_ATTRIBUTES_W_PROFILE(AI_response,AI_gov->current_profile)->user_input_importance);
}

static ssize_t store_AI_response_user_input_importance_attribute(const char* buf, size_t count)
{
	int var, ret;

	ret = kstrtoint(buf, 10, &var);
	if(ret < 0) return ret;

	GET_ATTRIBUTES_W_PROFILE(AI_response,AI_gov->current_profile)->user_input_importance = var;

	return count;
}

//EXIT
static ssize_t show_AI_exit_deinitialized_attribute(char* buf)
{
	return sprintf(buf, "%d\n",
			GET_ATTRIBUTES_W_PROFILE(AI_exit,AI_gov->current_profile)->deinitialized);
}

static ssize_t store_AI_exit_deinitialized_attribute(const char* buf, size_t count)
{
	return 0;
}

#define ATTRB_INIT_ATTRS(PROFILE, ATTRS) \
	static ssize_t show_##PROFILE##_##ATTRS##_gov_sys									\
		(struct kobject *kobj, struct attribute *attr, char *buf)			\
			{																\
				return show_##PROFILE##_##ATTRS##_attribute(buf);						\
			}																\
	static ssize_t store_##PROFILE##_##ATTRS##_gov_sys								\
		(struct kobject *kobj, struct attribute *attr, const char *buf,		\
				size_t count)												\
			{																\
				return store_##PROFILE##_##ATTRS##_attribute(buf, count);				\
			}																\
	static struct global_attr PROFILE##_##ATTRS##_gov_sys =								\
		__ATTR(ATTRS, 0644, show_##PROFILE##_##ATTRS##_gov_sys,				\
				store_##PROFILE##_##ATTRS##_gov_sys);

#define ATTRB_INIT_ATTR_ARRAY_ENTRY(PROFILE, ATTRS)									\
		&PROFILE##_##ATTRS##_gov_sys.attr,

#define ATTRB_INIT_ATTR_ARRAY(PROFILE)									\
		static struct attribute *AI_gov_attrs_##PROFILE##_gov_sys[] = {	\
			SYSFS_##PROFILE##_ATTRIBS(ATTRB_INIT_ATTR_ARRAY_ENTRY)									\
			NULL,																\
		};

#define ATTRB_INIT_GRP(PROFILE)			\
		static struct attribute_group AI_gov_attrs_group_##PROFILE##_gov_sys = { \
			.attrs = AI_gov_attrs_##PROFILE##_gov_sys,					\
			.name = NULL, 								\
			};

#define INIT_SYSFS_GROUP(PROFILE)		\
	SYSFS_##PROFILE##_ATTRIBS(ATTRB_INIT_ATTRS)			\
	ATTRB_INIT_ATTR_ARRAY(PROFILE)	\
	ATTRB_INIT_GRP(PROFILE)

#define INIT_ALL_SYSFS_GROUPS \
	FOR_EACH_PHASE(INIT_SYSFS_GROUP)

INIT_ALL_SYSFS_GROUPS

#define ATTACH_SINGLE_SYSFS_GROUP(PHASE) \
	sysfs_init = AI_phases_get_name(PHASE_STRINGS[PHASE]); \
	sysfs_init->sysfs_attr_grp = &AI_gov_attrs_group_##PHASE##_gov_sys; \
	sysfs_init->kobj = kobject_create_and_add("profile", AI_gov->kobj); \
	if(sysfs_init->kobj == NULL){ \
		KERNEL_ERROR_MSG( \
				"[SYSFS] AI_Governor: Init sysfs group couldn't init kobject for" \
					"profile %s \n", sysfs_init->phase_name); \
		return -ENOMEM;	\
	}\
	KERNEL_DEBUG_MSG( "[GOVERNOR] kobject created for %s \n" \
		, sysfs_init->phase_name); \
	ret = sysfs_create_group(sysfs_init->kobj, \
			sysfs_init->sysfs_attr_grp);	\
	kobject_del(sysfs_init->kobj);

#define ATTACH_SYSFS_GROUPS \
	FOR_EACH_PHASE(ATTACH_SINGLE_SYSFS_GROUP)


void debug_profile(struct phase_profile* profile)
{
	if(profile->phase >= AI_init && profile->phase < AI_END)
		KERNEL_DEBUG_MSG(
				"[PROFILE] phase: %s \n", PHASE_STRINGS[profile->phase]);

	if(profile->phase_name != NULL)
		KERNEL_DEBUG_MSG(
				"[PROFILE] name: %s \n",profile->phase_name);

	if(profile->next != NULL)
		KERNEL_DEBUG_MSG(
				"[PROFILE] next profile: %s \n", profile->next->phase_name);
}

//must get called after the phase has been updated
signed int AI_gov_sysfs_actualize_phase(void){
	//unregisted old kobj

	int ret = 0;
	//TODO checks
	kobject_del(AI_gov->previous_profile->kobj);
	ret = kobject_add(AI_gov->current_profile->kobj, AI_gov->kobj,
			"profile");

	if(ret) return ret;

	return 0;
}

signed int AI_gov_sysfs_init_profiles(void)
{

	int ret = 0;
	struct phase_profile* sysfs_init;

	ATTACH_SYSFS_GROUPS

	return 0;
}

signed int AI_gov_sysfs_init(void)
{

	int ret = 0;
	struct phase_profile* current_profile_struct;

	//AI_gov parent folder
	AI_gov->kobj = kobject_create_and_add("AI_governor",
			cpufreq_global_kobject);

	if(!AI_gov->kobj) return -ENOMEM;

	KERNEL_DEBUG_MSG( "[GOVERNOR] AI_gov_sysfs_init"
			"AI_governor kobj added \n");

	//TODO put this attr group into AI_gov_info
	ret = sysfs_create_group(AI_gov->kobj, AI_get_sysfs_attr());

	if (ret) {
		KERNEL_ERROR_MSG("[GOVERNOR]AI_gov_sysfs_init "
				"Error initializing sysfs! Code: %d\n", ret);
		kobject_put(AI_gov->kobj);
		return ret;
	}

	KERNEL_DEBUG_MSG( "[GOVERNOR] AI_gov_sysfs_init"
				"AI_governor sysfs group added \n");

	//hardware subdirectory
	AI_gov->hardware->kobj = kobject_create_and_add("hardware",
			AI_gov->kobj);

	if(!AI_gov->hardware->kobj) return -ENOMEM;

	KERNEL_DEBUG_MSG( "[GOVERNOR] AI_gov_sysfs_init"
				"hardware kobj added \n");

	ret = sysfs_create_group(AI_gov->hardware->kobj,
			&AI_gov_attrs_grp_hardware);

	if (ret) {
		KERNEL_ERROR_MSG("[GOVERNOR]AI_gov_sysfs_init "
				"Error initializing hardware sysfs! Code: %d\n", ret);
		kobject_put(AI_gov->hardware->kobj);
		return ret;
	}

	KERNEL_DEBUG_MSG( "[GOVERNOR] AI_gov_sysfs_init"
					"hardware sysfs group added \n");

	//current profile
	AI_gov->phase = AI_init;

	KERNEL_DEBUG_MSG("[GOVERNOR] retrieving first profile with "
			"name: %s \n", PHASE_STRINGS[AI_gov->phase]);

	current_profile_struct = GET_CURRENT_PROFILE;

	AI_gov->current_profile = current_profile_struct;

	KERNEL_DEBUG_MSG("[GOVERNOR] current profile struct pointer pointing to: %p, "
				"AI_gov->current profile pointing to: %p \n",
				(void*)current_profile_struct, (void*)AI_gov->current_profile);

	if(AI_gov->current_profile == NULL){
		KERNEL_ERROR_MSG("[GOVERNOR] AI_gov_sysfs_init Can't add "
				"current profile kobject \n");
		return -ENOENT;
	}

	KERNEL_DEBUG_MSG( "[GOVERNOR] first retrieved profile"
		" with name: %s \n", AI_gov->current_profile->phase_name);

	if(kobject_add(AI_gov->current_profile->kobj, AI_gov->kobj,
		"profile")){
		KERNEL_ERROR_MSG("[GOVERNOR] AI_gov_sysfs_init can't add kobject \n");
		return -ENOENT;
	}

	KERNEL_DEBUG_MSG( "[GOVERNOR] first retrieved profile"
			" kobject added \n");

	ret = sysfs_create_group(AI_gov->current_profile->kobj,
		AI_gov->current_profile->sysfs_attr_grp);

	if(ret){
		KERNEL_ERROR_MSG("[GOVERNOR]AI_gov_sysfs_init "
				"Error attaching current sysfs profile's attributes! "
				"Code: %d\n", ret);
		kobject_put(AI_gov->hardware->kobj);
		return ret;
	}

	KERNEL_DEBUG_MSG( "[GOVERNOR] first retrieved profile"
				" sysfs group added \n");

	KERNEL_DEBUG_MSG("[GOVERNOR] HERE AI_gov->current profile pointing to: %p \n",
						(void*)AI_gov->current_profile);

	return 0;
}

struct attribute_group *AI_get_sysfs_attr(void)
{
	return &AI_gov_attr_group_gov_sys;
}

struct kobject *AI_get_gov_parent_kobj(struct cpufreq_policy *policy)
{
	return cpufreq_global_kobject;
}
