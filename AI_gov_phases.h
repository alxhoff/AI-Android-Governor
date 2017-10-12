/*
* AI_gov_phases.h
*
*  Created on: Aug 31, 2017
*      Author: alxhoff
*/

#ifndef AI_GOV_PHASES_H_
#define AI_GOV_PHASES_H_

#include "test_flags.h"

#define GET_ATTRIBUTES_W_PROFILE(phase, defaults) \
		((struct phase_##phase##_attributes*)defaults->profile_attributes)

#define GET_ATTRIBUTES(phase) \
		((struct phase_##phase##_attributes*)AI_gov->current_profile->profile_attributes)

#define FOR_EACH_PHASE(PHASE)		\
				PHASE(AI_init) 		\
				PHASE(AI_framerate)	\
				PHASE(AI_ondemand)	\
				PHASE(AI_priority)	\
				PHASE(AI_time)	\
				PHASE(AI_powersave)	\
				PHASE(AI_performance)	\
				PHASE(AI_response)	\
				PHASE(AI_exit)

#define SYSFS_AI_init_ATTRIBS(ATTRB) \
			ATTRB(AI_init, initialized)

#define SYSFS_AI_framerate_ATTRIBS(ATTRB)\
			ATTRB(AI_framerate, desired_framerate) \
			ATTRB(AI_framerate, current_framerate)

#define SYSFS_AI_ondemand_ATTRIBS(ATTRB)\
			ATTRB(AI_ondemand, sampling_rate) \
			ATTRB(AI_ondemand, io_is_busy) \
			ATTRB(AI_ondemand, up_threshold) \
			ATTRB(AI_ondemand, sampling_down_factor) \
			ATTRB(AI_ondemand, ignore_nice_load) \
			ATTRB(AI_ondemand, powersave_bias) \
			ATTRB(AI_ondemand, sampling_rate_min)

#define SYSFS_AI_priority_ATTRIBS(ATTRB) \
			ATTRB(AI_priority, priority_scalar) \
			ATTRB(AI_priority, minimum_priority) \
			ATTRB(AI_priority, maximum_priority)

#define SYSFS_AI_time_ATTRIBS(ATTRB) \
			ATTRB(AI_time, time_till_completion) \
			ATTRB(AI_time, time_at_completion) \
			ATTRB(AI_time, alarm_mode) \

#define SYSFS_AI_powersave_ATTRIBS(ATTRB) \
			ATTRB(AI_powersave, initialized) \

#define SYSFS_AI_performance_ATTRIBS(ATTRB) \
			ATTRB(AI_performance, initialized) \

#define SYSFS_AI_response_ATTRIBS(ATTRB) \
			ATTRB(AI_response, user_input_importance) \

#define SYSFS_AI_exit_ATTRIBS(ATTRB) \
			ATTRB(AI_exit, deinitialized) \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING)	#STRING,

enum PHASE_ENUM {
	FOR_EACH_PHASE(GENERATE_ENUM)
	AI_END
};

extern char* PHASE_STRINGS[];// = {
//	FOR_EACH_PHASE(GENERATE_STRING)
//};

typedef struct phase_profile phase_profile_t;

struct phase_profile{

	enum PHASE_ENUM phase;

	char* phase_name;

	void* profile_attributes;

	struct attribute_group* sysfs_attr_grp;

	phase_profile_t* next;

	struct kobject* kobj;

	unsigned int (*enter)(void);
	unsigned int (*exit)(void);
	unsigned int (*run)(void);
};

//INIT
struct phase_AI_init_attributes{
	int initialized;
};

//FRAMERATE
#define FRAMERATE_HISTORY_LENGTH		20
#define FRAMERATE_DESIRED_FRAMERATE		60

struct phase_AI_framerate_attributes{
	int desired_framerate;
	int current_frametate;

	int (*timestamp_history)[FRAMERATE_HISTORY_LENGTH];
};

//ONDEMAND
#define DEF_FREQUENCY_DOWN_DIFFERENTIAL		(10)
#define DEF_FREQUENCY_UP_THRESHOLD		(80)
#define DEF_SAMPLING_DOWN_FACTOR		(1)
#define MAX_SAMPLING_DOWN_FACTOR		(100000)
#define MICRO_FREQUENCY_DOWN_DIFFERENTIAL	(3)
#define MICRO_FREQUENCY_UP_THRESHOLD		(95)
#define MICRO_FREQUENCY_MIN_SAMPLE_RATE		(10000)
#define MIN_FREQUENCY_UP_THRESHOLD		(11)
#define MAX_FREQUENCY_UP_THRESHOLD		(100)

struct phase_AI_ondemand_attributes{
	int sampling_rate;
	int io_is_busy;
	int up_threshold;
	int sampling_down_factor;
	int ignore_nice_load;
	int powersave_bias;
	int sampling_rate_min;
};

//PRIORITY
#define DEFAULT_PRIORITY_SCALAR			1
#define MINIMUM_PRIORITY				1
#define MAXIMUM_PRIORITY				10

struct phase_AI_priority_attributes{
	int priority_scalar;

	int minimum_priority;
	int maximum_priority;
};

//TIME
#define DEFAULT_TIME_MODE				1

struct phase_AI_time_attributes{
	long time_till_completion;
	long time_at_completion;

	bool alarm_mode;
};

//POWERSAVE
struct phase_AI_powersave_attributes{
	int initialized;
};

//PERFORMANCE
struct phase_AI_performance_attributes{
	int initialized;
};

//RESPONSE
#define DEFAULT_USER_IMPORTANCE			1
struct phase_AI_response_attributes{
	int user_input_importance;
};

//EXIT
struct phase_AI_exit_attributes{
	int deinitialized;
};

unsigned char AI_phases_init_profiles(void);
unsigned char AI_phases_getBrowsingPhase(void);
unsigned char AI_phases_getPrevBrowsingPhase(void);
int AI_phases_touch_nb(void);
void AI_phases_enter(void);
struct phase_profile* AI_phases_get_name(char* name);

#endif /* AI_GOV_PHASES_H_ */
