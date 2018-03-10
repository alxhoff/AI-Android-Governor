/**
 * @file AI_gov_phases.c
 * @author Alex Hoffman
 * @date 10 March 2018
 * @brief Phase specific data structures
*/

/* -- Includes -- */
/* Kernel includes. */
#include <linux/slab.h>

//#include "cpufreq_governor.h"

/* Governor includes. */
#include "AI_gov_ioctl.h"
#include "AI_gov.h"
#include "AI_gov_phases.h"
#include "AI_gov_sysfs.h"
#include "AI_gov_kernel_write.h"

//governor ports
#include "AI_ondemand_port.h"

/**
* @brief Creates string literal for a given input
*/
#define PHASE_NAME(NAME)	#NAME

/**
* @brief Creates profile functions as well as generating init function 
* to initialize the profile
*/
#define init_phase_struct(SET_PHASE)	\
	unsigned int enter_##SET_PHASE##_phase(void); \
	unsigned int exit_##SET_PHASE##_phase(void); \
	unsigned int run_##SET_PHASE##_phase(void); \
	static unsigned int phase_##SET_PHASE##_enter(void){ \
		return enter_##SET_PHASE##_phase(); \
	} \
	static unsigned int phase_##SET_PHASE##_exit(void){ \
		return exit_##SET_PHASE##_phase(); \
	} \
	static unsigned int phase_##SET_PHASE##_run(void){ \
		return run_##SET_PHASE##_phase(); \
	} \
	struct phase_profile* init_##SET_PHASE##_profile(void){ \
		struct phase_profile* init_phase_profile =  \
				kcalloc(1, sizeof(struct phase_profile), GFP_KERNEL); \
		init_phase_profile->profile_attributes = \
				kcalloc(1, sizeof(struct phase_##SET_PHASE##_attributes), \
						GFP_KERNEL); \
		init_phase_profile->phase_name = \
			kmalloc(strlen(PHASE_STRINGS[SET_PHASE]) + 1, GFP_KERNEL); \
		strcpy(init_phase_profile->phase_name, PHASE_STRINGS[SET_PHASE] ); \
		KERNEL_DEBUG_MSG(  \
				"[GOVERNOR] initializing %s with the saved name %s \n", \
				PHASE_STRINGS[SET_PHASE], init_phase_profile->phase_name); \
		init_phase_profile->enter = &phase_##SET_PHASE##_enter; \
		init_phase_profile->exit = &phase_##SET_PHASE##_exit; \
		init_phase_profile->run = &phase_##SET_PHASE##_run; \
		return init_phase_profile; \
	}\

/**
* @brief Calls init function for a given phase as well as adding the phase profile
* to the governor.
*/
#define INIT_PROFILE(PHASE) \
		init_profile = init_##PHASE##_profile(); \
		KERNEL_DEBUG_MSG("[GOVERNOR] profile initialized named: %s \n", \
				init_profile->phase_name); \
		AI_phases_add_profile(init_profile);

/**
* @brief Invokes profile generation for each phase
*/
#define GENERATE_PROFILES \
			struct phase_profile* init_profile; \
			FOR_EACH_PHASE(INIT_PROFILE)

/**
* @brief Initialised the phase struct for each phase profile speficied in the governor 
*/
FOR_EACH_PHASE(init_phase_struct);

/**
 * @brief String array with string literal representation of each phase profile, strings
 * are accessable using the PHASE_ENUM values 
 */
char* PHASE_STRINGS[] = {
	FOR_EACH_PHASE(GENERATE_STRING)
};

signed int AI_gov_sysfs_load_profile(enum PHASE_ENUM new_phase)
{
	int ret = 0;

	if(AI_gov->current_profile->exit!= NULL) AI_gov->current_profile->exit();

	AI_gov->prev_phase = AI_gov->phase;
	AI_gov->phase = new_phase;

	if(AI_gov->current_profile != NULL){
		KERNEL_DEBUG_MSG("[GOVERNOR] unloading profile: %s \n",
				AI_gov->current_profile->phase_name);

		AI_gov->previous_profile = AI_gov->current_profile;

		kobject_del(AI_gov->current_profile->kobj);
	}else {
		KERNEL_DEBUG_MSG(
				"[GOVERNOR] no profile to unload \n");
		return -ENOENT;
	}

	AI_gov->current_profile = AI_phases_get_name(PHASE_STRINGS[new_phase]);

	if(AI_gov->current_profile == NULL){
		KERNEL_ERROR_MSG( \
			"[GOVERNOR] AI_Governor: failed to retrieve new profile:" \
				" %d \n", new_phase);
		return -ENOENT;
	}

	if(AI_gov->current_profile->enter != NULL) AI_gov->current_profile->enter();

	if(kobject_add(AI_gov->current_profile->kobj, AI_gov->kobj, "profile")){
		KERNEL_ERROR_MSG("[GOVERNOR]AI_Governor: "
						"Can't add kobj\n");
		return -ENOENT;
	}

	ret = sysfs_create_group(AI_gov->current_profile->kobj,
			AI_gov->current_profile->sysfs_attr_grp);

	if(ret){
		KERNEL_ERROR_MSG("[GOVERNOR]AI_Governor: "
				"Error attaching current sysfs profile's attributes! Code: %d\n", ret);
		kobject_put(AI_gov->hardware->kobj);
		return ret;
	}

	KERNEL_DEBUG_MSG(
		"[GOVERNOR] profile loaded: %s \n",
		AI_gov->current_profile->phase_name);

	return 0;
}

/** @defgroup phase_functions Phase functions
*
* Each phase must have three functions, an enter, exit and run
* function. The functions must have the format: XXX_AI_#phase#_phase(void)
* where XXX denotes enter, exit or run. The enter function is called upon
* entering the phase, exit is called upon leaving the phase while run is 
* executed with each tick of the governor's task.
* @{
*/

/**
* @brief
*
* 
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int enter_AI_init_phase(void)
{
	KERNEL_DEBUG_MSG(
			"[GOVERNOR] Entered INIT phase\n");
	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int exit_AI_init_phase(void)
{
	KERNEL_DEBUG_MSG(
			"[GOVERNOR] Exited INIT phase\n");
	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int run_AI_init_phase(void)
{
	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int enter_AI_framerate_phase(void)
{
	KERNEL_DEBUG_MSG(
			"[GOVERNOR] Entered FRAMERATE phase\n");
	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int exit_AI_framerate_phase(void)
{
	KERNEL_DEBUG_MSG(
			"[GOVERNOR] Exited FRAMERATE phase\n");
	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int run_AI_framerate_phase(void)
{
	KERNEL_DEBUG_MSG(
					"[GOVERNOR] Running FRAMERATE phase\n");
	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int enter_AI_ondemand_phase(void)
{
	//register governor
	cpufreq_register_governor(&cpufreq_gov_ondemand);
	KERNEL_DEBUG_MSG(
		"[GOVERNOR] Entered ONDEMAND phase\n");

	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int exit_AI_ondemand_phase(void)
{
	cpufreq_unregister_governor(&cpufreq_gov_ondemand);
	KERNEL_DEBUG_MSG(
			"[GOVERNOR] Exited ONDEMAND phase\n");
	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int run_AI_ondemand_phase(void)
{
	//OD_od_check_cpu();
	KERNEL_DEBUG_MSG(
				"[GOVERNOR] Running ONDEMAND phase\n");
	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int enter_AI_priority_phase(void)
{
	KERNEL_DEBUG_MSG(
			"[GOVERNOR] Entered PRIORITY phase\n");
	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int exit_AI_priority_phase(void)
{
	KERNEL_DEBUG_MSG(
			"[GOVERNOR] Exited PRIORITY phase\n");
	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int run_AI_priority_phase(void)
{
	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int enter_AI_time_phase(void)
{
	KERNEL_DEBUG_MSG(
			"[GOVERNOR] Entered TIME phase\n");
	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int exit_AI_time_phase(void)
{
	KERNEL_DEBUG_MSG(
			"[GOVERNOR] Exited TIME phase\n");
	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int run_AI_time_phase(void)
{
	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int enter_AI_powersave_phase(void)
{
	KERNEL_DEBUG_MSG(
			"[GOVERNOR] Entered POWERSAVE phase\n");
	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int exit_AI_powersave_phase(void)
{
	KERNEL_DEBUG_MSG(
			"[GOVERNOR] Exited POWERSAVE phase\n");
	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int run_AI_powersave_phase(void)
{
	//TODO CHECK CURRENT FREQ AND DETERMINE IF NEEDS TO BE SET
	KERNEL_DEBUG_MSG(
				"[GOVERNOR] Running POWERSAVE phase\n");
	pr_debug("setting to %u kHz because of powersave \n",
								AI_gov->cpu_freq_policy->min);
	__cpufreq_driver_target(AI_gov->cpu_freq_policy, AI_gov->cpu_freq_policy->min,
					CPUFREQ_RELATION_L);
	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int enter_AI_performance_phase(void)
{
	KERNEL_DEBUG_MSG(
			"[GOVERNOR] Entered PERFORMANCE phase\n");
	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int exit_AI_performance_phase(void)
{
	KERNEL_DEBUG_MSG(
			"[GOVERNOR] Exited PERFORMANCE phase\n");
	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int run_AI_performance_phase(void)
{
	//TODO CHECK CURRENT FREQ AND DETERMINE IF NEEDS TO BE SET
	KERNEL_DEBUG_MSG(
				"[GOVERNOR] Run PERFORMANCE phase\n");
	pr_debug("setting to %u kHz because of performance \n",
										AI_gov->cpu_freq_policy->max);
	__cpufreq_driver_target(AI_gov->cpu_freq_policy, AI_gov->cpu_freq_policy->max,
					CPUFREQ_RELATION_H);
	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int enter_AI_response_phase(void)
{
	KERNEL_DEBUG_MSG(
			"[GOVERNOR] Entered RESPONSE phase\n");
	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int exit_AI_response_phase(void)
{
	KERNEL_DEBUG_MSG(
			"[GOVERNOR] Exited RESPONSE phase\n");
	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int run_AI_response_phase(void)
{
	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int enter_AI_exit_phase(void)
{
	KERNEL_DEBUG_MSG(
			"[GOVERNOR] Entered EXIT phase\n");
	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int exit_AI_exit_phase(void)
{
	KERNEL_DEBUG_MSG(
			"[GOVERNOR] Exited EXIT phase\n");
	return 0;
}

/**
* @brief
*
*
*
* @return 0 on success
* @ingroup phase_functions
*/
unsigned int run_AI_exit_phase(void)
{
	return 0;
}
/** @} */ // end of phase_functions

struct phase_profile* AI_phases_get_name(char* name)
{
	struct phase_profile* head = AI_gov->profile_head;

	KERNEL_DEBUG_MSG(
			"[GOVERNOR] getting phase via name: %s \n",
			name);

	if(AI_gov->profile_count == 0){
		KERNEL_ERROR_MSG(
				"[GOVERNOR] AI_Governor: head returned as NULL,"
				"no profile count\n");
		return NULL;
	}

	while(strcmp(head->phase_name, name)){
		if(head->next == NULL){
			KERNEL_ERROR_MSG(
					"[GOVERNOR] AI_Governor: head returned as NULL, "
					"no head set\n");
			return NULL;
		}

		head = head->next;
	}

	KERNEL_ERROR_MSG(
		"[GOVERNOR] head returned as %s for input %s\n",
		head->phase_name, name);

	return head;
}

/**
* @brief Gets the last phase profile from the profile linked list
*
* Retrieves a pointer to the last phase profile in the linked list
* of phase profiles stored within AI_gov_info
*
* @return pointer to last phase profile, NULL on error
*/
struct phase_profile* AI_phases_get_last(void)
{
	struct phase_profile* head = AI_gov->profile_head;

	if(AI_gov->profile_count == 0)
		return NULL;

	while(head->next != NULL)
		head = head->next;

	return head;
}

unsigned char AI_phases_add_profile(struct phase_profile* to_add)
{

	KERNEL_DEBUG_MSG( "[GOVERNOR] adding profile %s \n",
			to_add->phase_name);

	if(AI_gov->profile_count == 0){
		KERNEL_DEBUG_MSG( "[GOVERNOR] profile %s added to head\n",
				to_add->phase_name);
		AI_gov->profile_head = to_add;
		AI_gov->profile_count++;
	}else{
		struct phase_profile* last = AI_phases_get_last();
		last->next = to_add;
		KERNEL_DEBUG_MSG( "[GOVERNOR] adding profile %s after %s \n",
				to_add->phase_name, last->phase_name);
		AI_gov->profile_count++;
	}

	return 0;
}

unsigned char AI_phases_set_defaults(void)
{

	struct phase_profile* set_defaults;
	//framerate
	set_defaults = AI_phases_get_name(PHASE_STRINGS[AI_framerate]);
	GET_ATTRIBUTES_W_PROFILE(AI_framerate, set_defaults)->desired_framerate
			= FRAMERATE_DESIRED_FRAMERATE;
	//THIS LINE IS PROBABLY WRONG VVVVV
	GET_ATTRIBUTES_W_PROFILE(AI_framerate, set_defaults)->timestamp_history =
			kmalloc(sizeof(int)*FRAMERATE_HISTORY_LENGTH, GFP_KERNEL);
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	if(GET_ATTRIBUTES_W_PROFILE(AI_framerate, set_defaults)->timestamp_history
			== NULL) return -ENOMEM;

	//ondemand

	//priority
	set_defaults = AI_phases_get_name(PHASE_STRINGS[AI_priority]);
	GET_ATTRIBUTES_W_PROFILE(AI_priority, set_defaults)->maximum_priority
			= MAXIMUM_PRIORITY;
	GET_ATTRIBUTES_W_PROFILE(AI_priority, set_defaults)->minimum_priority
			= MINIMUM_PRIORITY;
	GET_ATTRIBUTES_W_PROFILE(AI_priority, set_defaults)->priority_scalar
			= DEFAULT_PRIORITY_SCALAR;

	//time
	set_defaults = AI_phases_get_name(PHASE_STRINGS[AI_time]);
	GET_ATTRIBUTES_W_PROFILE(AI_time, set_defaults)->alarm_mode = DEFAULT_TIME_MODE;

	//response
	set_defaults = AI_phases_get_name(PHASE_STRINGS[AI_response]);
	GET_ATTRIBUTES_W_PROFILE(AI_response, set_defaults)->user_input_importance
			= DEFAULT_USER_IMPORTANCE;

	//exit
	set_defaults = AI_phases_get_name(PHASE_STRINGS[AI_exit]);
	GET_ATTRIBUTES_W_PROFILE(AI_exit, set_defaults)->deinitialized = 0;

	return 0;
}

unsigned char AI_phases_init_profiles(void)
{
	//calls phase init and appends profile struct to LL
	GENERATE_PROFILES

	//creates, attachees then removes each kobject
	AI_gov_sysfs_init_profiles();

	AI_phases_set_defaults();

	return 0;
}

unsigned char AI_phases_getBrowsingPhase(void)
{
	return AI_gov->phase;
}

unsigned char AI_phases_getPrevBrowsingPhase(void)
{
	return AI_gov->prev_phase;
}

int AI_phases_touch_nb(void)
{
//	//KERNEL_LOGGG_MSG("[PHASES] Touch notifier callback.\n");
//	int ret = 0;
//#ifdef OVERHEAD_LOGGER
//	struct timespec ts;
//	getrawmonotonic(&ts);
//#endif
//
//	if (current_phase == AI_BROWSING_PHASE_NO_AI)
//		return ret;
//	if (!timer_pending(&touch_event_timer)) {
//		ret = mod_timer(
//				&touch_event_timer,
//				(get_jiffies_64() + msecs_to_jiffies(
//						AI_PHASES_TOUCH_START_TIME_OUT)));
//	} else {
//		ret = mod_timer_pending(
//				&touch_event_timer,
//				(get_jiffies_64() + msecs_to_jiffies(
//						AI_PHASES_TOUCH_START_TIME_OUT)));
//	}
//	if (ret != 0) {
//		//KERNEL_ERROR_MSG("[PHASES] AI_Governor: Error arming timer!\n");
//		return ret;
//	}
//	write_lock(&AI_rwlock);
//	touch_event_active = true;
//	write_unlock(&AI_rwlock);
//
//#ifdef OVERHEAD_LOGGER
//	AI_overhead_add_log_entry(AI_OVERHEAD_TOUCH_START, ts, 0);
//#endif
//	return ret;

	return 0;
}

