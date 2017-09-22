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

#define init_phase_struct(phase_name)	\
	static unsigned int phase_##phase_name##_enter(void* attributes){ \
		return enter_##phase_name##_phase(attributes); \
	} \
	static unsigned int phase_##phase_name##_exit(void* attributes){ \
		return exit_##phase_name##_phase(attributes); \
	} \
	static unsigned int phase_##phase_name##_run(void* attributes){ \
		return run_##phase_name##_phase(attributes); \
	} \
	struct phase_profile* init_##phase_name##_profile(){ \
		struct phase_profile* init =  \
				kmalloc(sizeof(struct phase_profile), GFP_KERNEL); \
		init->profile_attributes = \
				kmalloc(sizeof(struct phase_##phase_name##_attributes), GFP_KERNEL); \
		init->enter = &phase_##phase_name##_enter; \
		init->exit = &phase_##phase_name##_exit; \
		init->run = &phase_##phase_name##_run; \
		return init; \
	}\

#define attach_phase(phase_name) \
		(*profiles)->phase_name = init_##phase_name##_profile();

#define set_phase_name_string(phase_string) \
		profiles->phase_string->phase_name = \
			kmalloc(strlen(phase_name_string_##phase_string ) + 1, GFP_KERNEL); \
		if(profiles->phase_string->phase_name != NULL) \
			strcpy(profiles->phase_string->phase_name, phase_name_string_##phase_string);

#define GET_ATTRIBUTES(phase) \
		((struct phase_##phase##_attributes*)profiles->phase->profile_attributes)

//INIT
unsigned int enter_init_phase(void* attributes)
{

}

unsigned int exit_init_phase(void* attributes)
{

}

unsigned int run_init_phase(void* attributes)
{

}

init_phase_struct(init);

//FRAMERATE
unsigned int enter_framerate_phase(void* attributes)
{

}

unsigned int exit_framerate_phase(void* attributes)
{

}

unsigned int run_framerate_phase(void* attributes)
{

}

init_phase_struct(framerate);

//PRIORITY
unsigned int enter_priority_phase(void* attributes)
{

}

unsigned int exit_priority_phase(void* attributes)
{

}

unsigned int run_priority_phase(void* attributes)
{

}

init_phase_struct(priority);

//TIME
unsigned int enter_time_phase(void* attributes)
{

}

unsigned int exit_time_phase(void* attributes)
{

}

unsigned int run_time_phase(void* attributes)
{

}

init_phase_struct(time);

//POWERSAVE
unsigned int enter_powersave_phase(void* attributes)
{

}

unsigned int exit_powersave_phase(void* attributes)
{

}

unsigned int run_powersave_phase(void* attributes)
{

}

init_phase_struct(powersave);

//PRIORITY
unsigned int enter_performance_phase(void* attributes)
{

}

unsigned int exit_performance_phase(void* attributes)
{

}

unsigned int run_performance_phase(void* attributes)
{

}

init_phase_struct(performance);

//RESPONSE
unsigned int enter_response_phase(void* attributes)
{

}

unsigned int exit_response_phase(void* attributes)
{

}

unsigned int run_response_phase(void* attributes)
{

}

init_phase_struct(response);

//EXIT
unsigned int enter_exit_phase(void* attributes)
{

}

unsigned int exit_exit_phase(void* attributes)
{

}

unsigned int run_exit_phase(void* attributes)
{

}

init_phase_struct(exit);

unsigned char AI_phases_set_defaults(struct phase_profiles* profiles)
{
//	profiles->init->phase_name = kmalloc(strlen(PHASE_NAME_INIT) + 1, GFP_KERNEL);
//	if(profiles->init->phase_name != NULL) strcpy(profiles->init->phase_name, PHASE_NAME_INIT);

	set_phase_name_string(init);
	set_phase_name_string(framerate);
	set_phase_name_string(priority);
	set_phase_name_string(time);
	set_phase_name_string(powersave);
	set_phase_name_string(performance);
	set_phase_name_string(response);
	set_phase_name_string(exit);

	//framerate
	GET_ATTRIBUTES(framerate)->desired_framerate = FRAMERATE_DESIRED_FRAMERATE;
	//THIS LINE IS PROBABLY WRONG VVVVV
	GET_ATTRIBUTES(framerate)->timestamp_history =
			kmalloc(sizeof(int)*FRAMERATE_HISTORY_LENGTH, GFP_KERNEL);
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	if(GET_ATTRIBUTES(framerate)->timestamp_history == NULL) return -ENOMEM;

	//priority
	GET_ATTRIBUTES(priority)->maximum_priority = MAXIMUM_PRIORITY;
	GET_ATTRIBUTES(priority)->minimum_priority = MINIMUM_PRIORITY;
	GET_ATTRIBUTES(priority)->priority_scalar = DEFAULT_PRIORITY_SCALAR;

	//time
	GET_ATTRIBUTES(time)->alarm_mode = DEFAULT_TIME_MODE;

	//response
	GET_ATTRIBUTES(response)->user_input_importance = DEFAULT_USER_IMPORTANCE;

	return 0;
}

unsigned char AI_phases_init_profiles(struct phase_profiles** profiles)
{
	(*profiles) = kmalloc(sizeof(struct phase_profiles), GFP_KERNEL);
	if((*profiles) == NULL) return -ENOMEM;

	attach_phase(init);
	attach_phase(framerate);
	attach_phase(priority);
	attach_phase(time);
	attach_phase(powersave);
	attach_phase(performance);
	attach_phase(response);
	attach_phase(exit);

	AI_phases_set_defaults((*profiles));

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

void AI_phases_init(void)
{
	//TODO
}
