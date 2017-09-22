/*
 * AI_gov_phases.h
 *
 *  Created on: Aug 31, 2017
 *      Author: alxhoff
 */

#ifndef AI_GOV_PHASES_H_
#define AI_GOV_PHASES_H_

#include "test_flags.h"

struct phase_profile{

	unsigned char phase;

	char* phase_name;

	void* profile_attributes;

	struct kobject* kobj;

	unsigned int (*enter)(void* attributes);
	unsigned int (*exit)(void* attributes);
	unsigned int (*run)(void* attributes);
};

struct phase_profiles{
	struct phase_profile* init;
	struct phase_profile* framerate;
	struct phase_profile* priority;
	struct phase_profile* time;
	struct phase_profile* powersave;
	struct phase_profile* performance;
	struct phase_profile* response;
	struct phase_profile* exit;
};

//INIT
#define phase_name_string_init			"init"
struct phase_init_attributes{
	int initialized;
};

//FRAMERATE
#define phase_name_string_framerate		"framerate"
#define FRAMERATE_HISTORY_LENGTH		20
#define FRAMERATE_DESIRED_FRAMERATE		60

struct phase_framerate_attributes{
	int desired_framerate;
	int current_frametate;

	int (*timestamp_history)[FRAMERATE_HISTORY_LENGTH];
};

//PRIORITY
#define phase_name_string_priority		"priority"
#define DEFAULT_PRIORITY_SCALAR			1
#define MINIMUM_PRIORITY				1
#define MAXIMUM_PRIORITY				10

struct phase_priority_attributes{
	int priority_scalar;

	int minimum_priority;
	int maximum_priority;
};

//TIME
#define phase_name_string_time			"time"
#define DEFAULT_TIME_MODE				1

struct phase_time_attributes{
	long time_till_completion;
	long time_at_completion;

	bool alarm_mode;
};

//POWERSAVE
#define phase_name_string_powersave		"powersave"
struct phase_powersave_attributes{
};

//PERFORMANCE
#define phase_name_string_performance	"performance"
struct phase_performance_attributes{
};

//RESPONSE
#define phase_name_string_response		"response"
#define DEFAULT_USER_IMPORTANCE			1
struct phase_response_attributes{
	int user_input_importance;
};

//EXIT
#define phase_name_string_exit			"exit"
struct phase_exit_attributes{

};

unsigned char AI_phases_init_profiles(struct phase_profiles** profiles);
unsigned char AI_phases_getBrowsingPhase(void);
unsigned char AI_phases_getPrevBrowsingPhase(void);
int AI_phases_touch_nb(void);
void AI_phases_enter(void);

#endif /* AI_GOV_PHASES_H_ */
