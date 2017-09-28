/*
* AI_gov_phases.h
*
*  Created on: Aug 31, 2017
*      Author: alxhoff
*/

#ifndef AI_GOV_PHASES_H_
#define AI_GOV_PHASES_H_

#include "test_flags.h"

#define AI_GOV_NUM_OF_PHASES

typedef enum{
	AI_phase_init,
	AI_phase_framerate,
	AI_phase_priority,
	AI_phase_time,
	AI_phase_powersave,
	AI_phase_performance,
	AI_phase_response,
	AI_phase_end //must be here
} phase_state;

#define FOR_EACH_PHASE(PHASE)		\
				PHASE(AI_init) 		\
				PHASE(AI_framerate)	\
				PHASE(AI_priority)		\
				PHASE(AI_time)			\
				PHASE(AI_powersave)	\
				PHASE(AI_performance)	\
				PHASE(AI_response)


#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING)	AI_phase_##STRING,

enum PHASE_ENUM {
	FOR_EACH_PHASE(GENERATE_ENUM)
	END
};

static const char* PHASE_STRINGS[] = {
	FOR_EACH_PHASE(GENERATE_STRING)
};

typedef struct phase_profile phase_profile_t;

struct phase_profile{

	unsigned char phase;

	char* phase_name;

	void* profile_attributes;

	struct attributes_group* sysfs_attr_grp;

//	struct attribute** sysfs_attrs;

	phase_profile_t* next;

	struct kobject* kobj;

	unsigned int (*enter)(void* attributes);
	unsigned int (*exit)(void* attributes);
	unsigned int (*run)(void* attributes);
};

//INIT
// #define phase_name_string_init			"init"
struct phase_init_attributes{
	int initialized;
};

//FRAMERATE
// #define phase_name_string_framerate		"framerate"
#define FRAMERATE_HISTORY_LENGTH		20
#define FRAMERATE_DESIRED_FRAMERATE		60

struct phase_AI_framerate_attributes{
	int desired_framerate;
	int current_frametate;

	int (*timestamp_history)[FRAMERATE_HISTORY_LENGTH];
};

//PRIORITY
// #define phase_name_string_priority		"priority"
#define DEFAULT_PRIORITY_SCALAR			1
#define MINIMUM_PRIORITY				1
#define MAXIMUM_PRIORITY				10

struct phase_AI_priority_attributes{
	int priority_scalar;

	int minimum_priority;
	int maximum_priority;
};

//TIME
// #define phase_name_string_time			"time"
#define DEFAULT_TIME_MODE				1

struct phase_AI_time_attributes{
	long time_till_completion;
	long time_at_completion;

	bool alarm_mode;
};

//POWERSAVE
// #define phase_name_string_powersave		"powersave"
struct phase_AI_powersave_attributes{
};

//PERFORMANCE
// #define phase_name_string_performance	"performance"
struct phase_AI_performance_attributes{
};

//RESPONSE
// #define phase_name_string_response		"response"
#define DEFAULT_USER_IMPORTANCE			1
struct phase_AI_response_attributes{
	int user_input_importance;
};

//EXIT
// #define phase_name_string_exit			"exit"
struct phase_AI_exit_attributes{

};

unsigned char AI_phases_init_profiles();
unsigned char AI_phases_getBrowsingPhase(void);
unsigned char AI_phases_getPrevBrowsingPhase(void);
int AI_phases_touch_nb(void);
void AI_phases_enter(void);
struct phase_profile* AI_phases_get_name(char* name);

#endif /* AI_GOV_PHASES_H_ */
