/**
 * @file AI_gov_phases.h
 * @author Alex Hoffman
 * @date 10 March 2018
 * @brief Phase specific data structures
*/

#ifndef AI_GOV_PHASES_H_
#define AI_GOV_PHASES_H_

#include "test_flags.h"

/**
* Typecases a given phase_profile pointer to a given phase type
*/
#define GET_ATTRIBUTES_W_PROFILE(phase, defaults) \
		((struct phase_##phase##_attributes*)defaults->profile_attributes)

/**
* Typecases a the current phase's profile's attributes
*/
#define GET_ATTRIBUTES(phase) \
		((struct phase_##phase##_attributes*)AI_gov->current_profile->profile_attributes)

/**
* Macro used for initialisation called to initilise each phase.
* New phases MUST be added to this macro.
*/
#define FOR_EACH_PHASE(PHASE)		\
				PHASE(AI_init) 		\
				PHASE(AI_ondemand)	\
				PHASE(AI_framerate)	\
				PHASE(AI_priority)	\
				PHASE(AI_time)	\
				PHASE(AI_powersave)	\
				PHASE(AI_performance)	\
				PHASE(AI_response)	\
				PHASE(AI_exit)

/**
* @defgroup sysfs_attrb_init
* Each phase must have a macro defined that specifies each
* attribute that is to be added to the phase's sysfs profile
* folder. The macro must have the form of: <br>
* SYSFS_#phase name#_ATTRIBS(ATTRB) <br>
* 		ATTRB(#phase name#, #attribute 1 name#) \ <br>
* 		ATTRB(#phase name#, #attribute 2 name#) \ <br>
*		etc... <br>
* This is because the macro creates the initilisation code
* for each specified attribute and connects it with the
* specified phase. Each phase's attibutes must be in a single
* macro, you cannot mix the phase within a single macro.
*/
/**
* @struct phase_AI_init_attributes
* @brief Stores the attributes for the phase AI_init
*/
/**
* @ingroup sysfs_attrb_init
* @brief Sysfs attibutes for the phase: AI_init
*/
#define SYSFS_AI_init_ATTRIBS(ATTRB) \
			ATTRB(AI_init, initialized)

/**
* @struct phase_AI_framerate_attributes
* @brief Stores the attributes for the phase AI_framerate
*/
/**
* @ingroup sysfs_attrb_init
* @brief Sysfs attibutes for the phase: AI_framerate
*/
#define SYSFS_AI_framerate_ATTRIBS(ATTRB)\
			ATTRB(AI_framerate, desired_framerate) \
			ATTRB(AI_framerate, current_framerate)

/**
* @struct phase_AI_ondemand_attributes
* @brief Stores the attributes for the phase AI_ondemand
*/
/**
* @ingroup sysfs_attrb_init
* @brief Sysfs attibutes for the phase: AI_ondemand
*/
#define SYSFS_AI_ondemand_ATTRIBS(ATTRB)\
			ATTRB(AI_ondemand, sampling_rate) \
			ATTRB(AI_ondemand, io_is_busy) \
			ATTRB(AI_ondemand, up_threshold) \
			ATTRB(AI_ondemand, sampling_down_factor) \
			ATTRB(AI_ondemand, ignore_nice_load) \
			ATTRB(AI_ondemand, powersave_bias) \
			ATTRB(AI_ondemand, sampling_rate_min)

/**
* @struct phase_AI_priority_attributes
* @brief Stores the attributes for the phase AI_priority
*/
/**
* @ingroup sysfs_attrb_init
* @brief Sysfs attibutes for the phase: AI_priority
*/
#define SYSFS_AI_priority_ATTRIBS(ATTRB) \
			ATTRB(AI_priority, priority_scalar) \
			ATTRB(AI_priority, minimum_priority) \
			ATTRB(AI_priority, maximum_priority)

/**
* @struct phase_AI_time_attributes
* @brief Stores the attributes for the phase AI_time
*/
/**
* @ingroup sysfs_attrb_init
* @brief Sysfs attibutes for the phase: AI_time
*/
#define SYSFS_AI_time_ATTRIBS(ATTRB) \
			ATTRB(AI_time, time_till_completion) \
			ATTRB(AI_time, time_at_completion) \
			ATTRB(AI_time, alarm_mode) \

/**
* @struct phase_AI_powersave_attributes
* @brief Stores the attributes for the phase AI_powersave
*/
/**
* @ingroup sysfs_attrb_init
* @brief Sysfs attibutes for the phase: AI_powersave
*/
#define SYSFS_AI_powersave_ATTRIBS(ATTRB) \
			ATTRB(AI_powersave, initialized) \

/**
* @struct phase_AI_performance_attributes
* @brief Stores the attributes for the phase AI_performance
*/
/**
* @ingroup sysfs_attrb_init
* @brief Sysfs attibutes for the phase: AI_performance
*/
#define SYSFS_AI_performance_ATTRIBS(ATTRB) \
			ATTRB(AI_performance, initialized) \

/**
* @struct phase_AI_response_attributes
* @brief Stores the attributes for the phase AI_response
*/
/**
* @ingroup sysfs_attrb_init
* @brief Sysfs attibutes for the phase: AI_response
*/
#define SYSFS_AI_response_ATTRIBS(ATTRB) \
			ATTRB(AI_response, user_input_importance) \

/**
* @struct phase_AI_exit_attributes
* @brief Stores the attributes for the phase AI_exit
*/
/**
* @ingroup sysfs_attrb_init
* @brief Sysfs attibutes for the phase: AI_exit
*/
#define SYSFS_AI_exit_ATTRIBS(ATTRB) \
			ATTRB(AI_exit, deinitialized) \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING)	#STRING,

/**
* @enum PHASE_ENUM
* @brief Automatic ENUM generation for the governo's phases
*/
enum PHASE_ENUM {
	FOR_EACH_PHASE(GENERATE_ENUM) 	/**< Generates an ENUM entry for each phase*/
	AI_END 							/**< Added to keep track of the ENUM's end*/
};

/**
* @brief String literal array for each phase's name
* 
* String array to keep string literal representations of each phase's
* name. The string literal for each phase can be accessed by passing
* the phase's enum to the string array. <br>
* Example: PHASE_STRINGS[AI_powersave] will return the string literal
* "AI_powersave"
*/
extern char* PHASE_STRINGS[];

/**
* @typdef phase_profile_t Typedef of phase_profile
*/
typedef struct phase_profile phase_profile_t;

/**
* @struct phase_profile
* @brief Contains userspace accessable attributes for each phase
*
* Each phase must have a profile, the profile contrains a void pointer
* to its attributes struct which contains the attributes unique to itself.
* The profile will set the attributes that are accessable by userspace,
* the modification of these profile values will be what gives the governor
* its application interface.
*/
struct phase_profile{

	enum PHASE_ENUM phase; 		/**< The phase's index, used to find the phase
									and its profile from the linked list */

	char* phase_name;			/**< String representation of the phase's name */

	void* profile_attributes;	/**< Pointer to the phase's attributes struct */

	struct attribute_group* 
				sysfs_attr_grp;	/**< Pointer the the phase's sysfs attribute group */

	phase_profile_t* next;		/**< Next profile in the linked list */

	struct kobject* kobj;		/**< Profiles kobject used to register the profile
									with sysfs */

	unsigned int (*enter)(void);/**< Function called upon entering the phase */
	unsigned int (*exit)(void);	/**< Function called upon exiting the phase */
	unsigned int (*run)(void);	/**< Function called with each tick of the governor's
									task */
};

/**
* @defgroup profile_defaults Default value initilisation for phase profiles
* @brief Values and data types to store and set phase profile attribute
* defaults
*
* Each phase profile must has a stuct declared for it's attributes in the 
* style of <br> phase_#phase name#_attributes. Within this struct each
* attribute and it's type can be specified. Default values should also be
* defined within #defines. The #define default values are set into the 
* attribute structs in the function AI_phases_set_defaults which is called
* during initialisation of the governor.
*/

/**
* @struct phase_AI_init_attributes
* @ingroup profile_defaults
* @brief Attributes struct for the phase
*/
struct phase_AI_init_attributes{
	int initialized;
};

/**
* @defgroup AI_framerate_defaults
* @ingroup profile_defaults
* @brief Default values for AT_framerate's profile's attributes
* @{
*/
/**
* The number of framerate values that should be stored. Please note
* this value is staticallyt set at compile time.
*/
#define FRAMERATE_HISTORY_LENGTH		20
/**
* The desired framerate default
*/
#define FRAMERATE_DESIRED_FRAMERATE		60
/** @} */ //end of group

/**
* @struct phase_AI_framerate_attributes
* @ingroup profile_defaults
* @brief Attributes struct for the phase AI_framerate
*/
struct phase_AI_framerate_attributes{
	int desired_framerate;
	int current_frametate;

	int (*timestamp_history)[FRAMERATE_HISTORY_LENGTH];
};

/**
* @defgroup AI_ondemand_defaults
* @ingroup profile_defaults
* @brief Default values for AI_ondemand's profile's attributes
* @{
*/
/**
* 
*/
#define DEF_FREQUENCY_DOWN_DIFFERENTIAL		(10)
/**
* 
*/
#define DEF_FREQUENCY_UP_THRESHOLD		(80)
/**
* 
*/
#define DEF_SAMPLING_DOWN_FACTOR		(1)
/**
* 
*/
#define MAX_SAMPLING_DOWN_FACTOR		(100000)
/**
* 
*/
#define MICRO_FREQUENCY_DOWN_DIFFERENTIAL	(3)
/**
* 
*/
#define MICRO_FREQUENCY_UP_THRESHOLD		(95)
/**
* 
*/
#define MICRO_FREQUENCY_MIN_SAMPLE_RATE		(10000)
/**
* 
*/
#define MIN_FREQUENCY_UP_THRESHOLD		(11)
/**
* 
*/
#define MAX_FREQUENCY_UP_THRESHOLD		(100)
/** @} */ //end of group

/**
* @struct phase_AI_ondemand_attributes
* @ingroup profile_defaults
* @brief Attributes struct for the phase AI_ondemand
*/
struct phase_AI_ondemand_attributes{
	int sampling_rate;
	int io_is_busy;
	int up_threshold;
	int sampling_down_factor;
	int ignore_nice_load;
	int powersave_bias;
	int sampling_rate_min;
};

/**
* @defgroup AI_priority_defaults
* @ingroup profile_defaults
* @brief Default values for AI_prioority's profile's attributes
* @{
*/
/**
* 
*/
#define DEFAULT_PRIORITY_SCALAR			1
/**
* Minimum priority value, should be lest as 1
*/
#define MINIMUM_PRIORITY				1
/**
* Sets the maximum proprity value, as float values should be
* avoided within the kernel, using an integer scalar is therefore
* recommended. Increasing the maximum priority will increase the
* backend logic's resolution.
*/
#define MAXIMUM_PRIORITY				10
/** @} */ //end of group

/**
* @struct phase_AI_priority_attributes
* @ingroup profile_defaults
* @brief Attributes struct for the phase AI_priority
*/
struct phase_AI_priority_attributes{
	int priority_scalar;

	int minimum_priority;
	int maximum_priority;
};

/**
* @defgroup AI_time_defaults
* @ingroup profile_defaults
* @brief Default values for AI_time's profile's attributes
* @{
*/
/**
* Sets the default time mode. 1 being a coundown mode, counting until
* a specified period has been met. 0 being alarm mode, triggering when
* a certain time is reached.
*/
#define DEFAULT_TIME_MODE				1
/** @} */ //end of group

/**
* @struct phase_AI_time_attributes
* @ingroup profile_defaults
* @brief Attributes struct for the phase AI_time
*/
struct phase_AI_time_attributes{
	long time_till_completion;
	long time_at_completion;

	bool alarm_mode;
};

/**
* @struct phase_AI_powersave_attributes
* @ingroup profile_defaults
* @brief Attributes struct for the phase AI_powersave
*/struct phase_AI_powersave_attributes{
	int initialized;
};

/**
* @struct phase_AI_performance_attributes
* @ingroup profile_defaults
* @brief Attributes struct for the phase AI_performance
*/struct phase_AI_performance_attributes{
	int initialized;
};

/**
* @defgroup AI_response_defaults
* @ingroup profile_defaults
* @brief Default values for AI_response's profile's attributes
* @{
*/
/**
* Specified the default value for how highly weighted a user's
* input should be on a scale of 1 to 10.
*/
#define DEFAULT_USER_IMPORTANCE			1
/** @} */ //end of group

/**
* @struct phase_AI_response_attributes
* @ingroup profile_defaults
* @brief Attributes struct for the phase AI_response
*/
struct phase_AI_response_attributes{
	int user_input_importance;
	//TODO max a min values
};

/**
* @struct phase_AI_exit_attributes
* @ingroup profile_defaults
* @brief Attributes struct for the phase AI_exit
*/struct phase_AI_exit_attributes{
	int deinitialized;
};

/**
* @brief Unloads the current phase and it's profile and loads a new phase and profile
*
* Firstly the old profile's sysfs entry (kobject) is removed from the sysfs heirarchy 
* followed by the changing the phase within the AI_gov struct. This phase's profile is
* then actualised by loading the respective profile.
*
* @param new_phase - The new phase that is to be loaded, must be an already registered
* phase
* @return 0 on success
*
*/
signed int AI_gov_sysfs_load_profile(enum PHASE_ENUM new_phase);

/**
* @brief Adds a profile to the end of the governor's phase profile linked list
*
* Adds a phase profile pointer to the end of the phase profile linked list
* stored within AI_gov_info
*
* @param to_add - pointer to phase_profile struct to be added
* @return 0 on success
*/
unsigned char AI_phases_add_profile(struct phase_profile* to_add);

/**
* @brief Sets phase profile attribute default values
*
* All default attributes must be set within this function.
* To add profile defaults the temp phase_profile pointer must
* first be set to the profile to be set. This can be done using
* AI_phases_get_name. The macro GET_ATTRIBUTES_W_PROFILE can be
* used to retrieve the attributes of the current phase through
* typecasting the attributes void pointer. The attributes can thusly
* be set to the default values. As all phase profiles are calloc'd
* initially, all default values are 0 unless set within this function
*
* @return 0 on success
*/
unsigned char AI_phases_set_defaults(void);

/**
* @brief Initialises and attached each phase and its profile.
*
* Invoking the GENERATE_PROFILES macro, each phase's init function is called
* followed by each phase profile being appended to the phase profile linked list
*
* @return 0 on success
*/
unsigned char AI_phases_init_profiles(void);

/**
* @brief Retrieves the governor's current phase
*
* @return phase index of current profile
*/
unsigned char AI_phases_getBrowsingPhase(void);

/**
* @brief Retrieves the governor's current previous phase
*
* @return phase index of previous profile
*/
unsigned char AI_phases_getPrevBrowsingPhase(void);

/**
* @brief
*
*
*
* @return
*/
int AI_phases_touch_nb(void);

/**
* @brief Retrieves a phase profile given the phase's name
*
* A phase profile pointer can be requested and retreived
* by passing the function the name of the phase who's profile
* needs to be retieved.
*
* @param name - string name of the phase to be loaded
* @return 0 on success NULL otherwise
*/
struct phase_profile* AI_phases_get_name(char* name);

#endif /* AI_GOV_PHASES_H_ */
