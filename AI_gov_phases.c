/*
* AI_gov_phases.c
*
*  Created on: Sep 7, 2017
*      Author: alxhoff
*/

#include <linux/slab.h>

#include "AI_gov_ioctl.h"
#include "AI_gov.h"
#include "AI_gov_phases.h"
#include "AI_gov_sysfs.h"

#define PHASE_NAME(NAME)	#NAME

#define init_phase_struct(SET_PHASE)	\
	unsigned int enter_##SET_PHASE##_phase(void* attributes); \
	unsigned int exit_##SET_PHASE##_phase(void* attributes); \
	unsigned int run_##SET_PHASE##_phase(void* attributes); \
	static unsigned int phase_##SET_PHASE##_enter(void* attributes){ \
		return enter_##SET_PHASE##_phase(attributes); \
	} \
	static unsigned int phase_##SET_PHASE##_exit(void* attributes){ \
		return exit_##SET_PHASE##_phase(attributes); \
	} \
	static unsigned int phase_##SET_PHASE##_run(void* attributes){ \
		return run_##SET_PHASE##_phase(attributes); \
	} \
	struct phase_profile* init_##SET_PHASE##_profile(){ \
		struct phase_profile* init_phase_profile =  \
				kcalloc(1, sizeof(struct phase_profile), GFP_KERNEL); \
		init_phase_profile->profile_attributes = \
				kcalloc(1, sizeof(struct phase_##SET_PHASE##_attributes), GFP_KERNEL); \
		init_phase_profile->phase_name = \
			kmalloc(strlen(PHASE_STRINGS[SET_PHASE]) + 1, GFP_KERNEL); \
		strcpy(init_phase_profile->phase_name, PHASE_STRINGS[SET_PHASE] ); \
		init_phase_profile->enter = &phase_##SET_PHASE##_enter; \
		init_phase_profile->exit = &phase_##SET_PHASE##_exit; \
		init_phase_profile->run = &phase_##SET_PHASE##_run; \
		return init_phase_profile; \
	}\

#define GET_ATTRIBUTES(phase, defaults) \
		((struct phase_##phase##_attributes*)defaults->profile_attributes)

//enum and attribute names

#define INIT_PROFILE(PHASE) \
		init_profile = init_##PHASE##_profile(); \
		AI_phases_add_profile(init_profile);

#define GENERATE_PROFILES \
			struct phase_profile* init_profile; \
			FOR_EACH_PHASE(INIT_PROFILE)


FOR_EACH_PHASE(init_phase_struct);

//INIT
unsigned int enter_AI_init_phase(void* attributes)
{

}

unsigned int exit_AI_init_phase(void* attributes)
{

}

unsigned int run_AI_init_phase(void* attributes)
{

}

//FRAMERATE
unsigned int enter_AI_framerate_phase(void* attributes)
{

}

unsigned int exit_AI_framerate_phase(void* attributes)
{

}

unsigned int run_AI_framerate_phase(void* attributes)
{

}

//init_phase_struct(framerate);

//PRIORITY
unsigned int enter_AI_priority_phase(void* attributes)
{

}

unsigned int exit_AI_priority_phase(void* attributes)
{

}

unsigned int run_AI_priority_phase(void* attributes)
{

}

//init_phase_struct(priority);

////TIME
unsigned int enter_AI_time_phase(void* attributes)
{

}

unsigned int exit_AI_time_phase(void* attributes)
{

}

unsigned int run_AI_time_phase(void* attributes)
{

}

//init_phase_struct(time);

////POWERSAVE
unsigned int enter_AI_powersave_phase(void* attributes)
{

}

unsigned int exit_AI_powersave_phase(void* attributes)
{

}

unsigned int run_AI_powersave_phase(void* attributes)
{

}

//init_phase_struct(powersave);

////PRIORITY
unsigned int enter_AI_performance_phase(void* attributes)
{

}

unsigned int exit_AI_performance_phase(void* attributes)
{

}

unsigned int run_AI_performance_phase(void* attributes)
{

}

//init_phase_struct(performance);

////RESPONSE
unsigned int enter_AI_response_phase(void* attributes)
{

}

unsigned int exit_AI_response_phase(void* attributes)
{

}

unsigned int run_AI_response_phase(void* attributes)
{

}

//init_phase_struct(response);

////EXIT
unsigned int enter_AI_exit_phase(void* attributes)
{

}

unsigned int exit_AI_exit_phase(void* attributes)
{

}

unsigned int run_AI_exit_phase(void* attributes)
{

}

//init_phase_struct(exit);


struct phase_profile* AI_phases_get_name(char* name)
{
	if(AI_gov->profile_count == 0) return NULL;

	struct phase_profile* head = AI_gov->profile_head;

	while(strcmp(head->phase_name, name)){
		if(head->next == NULL) return NULL;

		head = head->next;
	}

	return head;
}

struct phase_profile* AI_phases_get_last()
{
	if(AI_gov->profile_count == 0)
		return NULL;

	struct phase_profile* head = AI_gov->profile_head;

	while(head->next != NULL)
		head = head->next;

	return head;
}

unsigned char AI_phases_add_profile(struct phase_profile* to_add)
{
	//TODO SHOULD THIS BE DYNAMIC?
	if(AI_gov->profile_count == 0){
		AI_gov->profile_head = to_add;
		AI_gov->profile_count++;
	}else{
		struct phase_profile* last = AI_phases_get_last();
		last->next = to_add;
		AI_gov->profile_count++;
	}

	return 0;
}

unsigned char AI_phases_set_defaults()
{

	struct phase_profile* set_defaults;
	//framerate
	set_defaults = AI_phases_get_name(PHASE_STRINGS[AI_framerate]);
	GET_ATTRIBUTES(AI_framerate, set_defaults)->desired_framerate
			= FRAMERATE_DESIRED_FRAMERATE;
	//THIS LINE IS PROBABLY WRONG VVVVV
	GET_ATTRIBUTES(AI_framerate, set_defaults)->timestamp_history =
			kmalloc(sizeof(int)*FRAMERATE_HISTORY_LENGTH, GFP_KERNEL);
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	if(GET_ATTRIBUTES(AI_framerate, set_defaults)->timestamp_history
			== NULL) return -ENOMEM;

	//priority
	set_defaults = AI_phases_get_name(PHASE_STRINGS[AI_priority]);
	GET_ATTRIBUTES(AI_priority, set_defaults)->maximum_priority
			= MAXIMUM_PRIORITY;
	GET_ATTRIBUTES(AI_priority, set_defaults)->minimum_priority
			= MINIMUM_PRIORITY;
	GET_ATTRIBUTES(AI_priority, set_defaults)->priority_scalar
			= DEFAULT_PRIORITY_SCALAR;

	//time
	set_defaults = AI_phases_get_name(PHASE_STRINGS[AI_time]);
	GET_ATTRIBUTES(AI_time, set_defaults)->alarm_mode = DEFAULT_TIME_MODE;

	//response
	set_defaults = AI_phases_get_name(PHASE_STRINGS[AI_response]);
	GET_ATTRIBUTES(AI_response, set_defaults)->user_input_importance
			= DEFAULT_USER_IMPORTANCE;

	return 0;
}

unsigned char AI_phases_init_profiles(void)
{
	GENERATE_PROFILES

	AI_phases_set_defaults();

	AI_gov_sysfs_init_profiles();

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
}

