/**
 * @file AI_gov.h
 * @author Alex Hoffman
 * @date 10 March 2018
 * @brief Main routines for the AI governor.
 * @section About
 * The AI governor enters and exits use through the routines outlines within
 * the file. All backend logic is initialized from this file, timers, tasks etc.
 * @mainpage Application Interface (AI) CPUfreq Governor
 * 
 * @section intro_sec Introduction
 * The AI governor aims to create an API style interface between user space 
 * applications and kernel space power management logic to allow user space 
 * applications to influence the power management logic being executed within 
 * the kernel. 
 * <br><br>
 * Classical power management governors implement their own power management 
 * algorithms, contained within a kernel module that has to be unloaded and loaded
 * to change the current governing logic within the CPUfreq subsystem. Classical
 * governors are purley kernel based, with no input coming from user space, thus
 * meaning the governors can only use measurements or properties found within kernel
 * space within their power optimization algorithms. The AI governor presents a
 * standardized interface and state machine template to allow for information transfer
 * from user space to kernel space in a way that it can influence the back end
 * power optimization logic being executed within the kernel. The backend logic
 * is also run within a state machine framework that allows for governing logic
 * to be modularized into states that can have unique power optimization logic as
 * well as unique user space-kernel space properties that can be read and written
 * through the IOctl interface.
 * <br><br>
 * The AI governor also provides a dynamic sysfs interface for kernel space 
 * interfacing with the governor.
 * 
 * @section how_sec How the AI Governor works
 * The governor can be broken down into three components, the IOctl interface,
 * the sysfs interface and the state machine. Below each of the components is
 * explained in detail.
 * 
 * @subsection state_machine_sec State Machine
 * The states in the governor are refered to as phases, the phases are mostly
 * defined within the AI_gov_phases.h header. There is a large number of macros
 * in place to reduce the code base, but more importantly, make adding phases,
 * phase attributes and phase attribute defaults a lot easier.
 * <br><br>
 * @subsubsection phase_info_sec Phase profile data structure
 * Each phase in the governor has a phase_profile data structure that stores
 * the phase's enum that is used to index the profile within the system, the
 * phase's name as well as the profile's attributes. Pointers to the phases's
 * init, run and exit functions as well as required sysfs objects. These phase
 * profile objects are generated automatically through the macros in place.
 * <br><br>
 * @subsubsection phsee_attributes_sec Adding a phase and attributes
 * Each phase must be added to the FOR_EACH_PHASE macro such that it is included
 * in all the initialisation macros. Attributes for each phase are specified in 
 * structures labeled in a specific way such that they can be included into the
 * macros. The phase attribute structs must be labeled such that they follow the
 * structure of "phase_##phase name##_attributes". Default values for each 
 * attribute can be set by using a define and set within the function
 * AI_phases_set_defaults. Below an example can be seen where a phase is retrieved
 * using the function AI_phases_get_name, where the phases enum value is used
 * to as an index in the string array of corresponding names for each phase, the
 * enum index and string array both generated from the FOR_EACH_PHASE macro. 
 * The macro GET_ATTRIBUTES_W_PROFILE can then be used to typecast the void
 * pointer within the phase profile, specifying the phase_profile pointer to
 * store the phase temporarily. The attributes can then be set.
 @verbatim
 struct phase_profile* set_defaults;
 set_defaults = AI_phases_get_name(PHASE_STRINGS[AI_framerate]);
 GET_ATTRIBUTES_W_PROFILE(AI_framerate, set_defaults)->desired_framerate
    = FRAMERATE_DESIRED_FRAMERATE;
 @endverbatim
 *
 * Each phase has three functions that are called in each of each phases'
 * three states, init, exit and run. The macro init_phase_struct is run
 * FOR_EACH_PHASE which prototypes the three functions of the format:
 * enter_##phase name##_phase, exit_##phase name##_phase and 
 * run_##phase name##_phase. These three functions are called at the respective
 * times when a phase it loaded or unloaded from the state machine. The
 * init_phase_struct macro also allocates the phase_profile and phase
 * attribute data structures.
 *  
 * @subsection ioctl_sec IOctl
 * The IOctl interface is split into two parts, the IOctl interface portion within
 * the kernel and then the IOctl portion that can be used within the user space.
 * 
 * @subsubsection ioctl_userspace_sec User Space
 * As explained in the state machine section, the AI governor revolves around 
 * having settable states with each state having unique properties that can 
 * be set. The state and properties can be set via an IOctl interface that 
 * accepts user space input and resolves within the kernel.<br><br>
 * There are 5 implemented IOctl commands within the AI governor's IOctl interface.
 * GOVERNOR_GET_PHASE, GOVERNOR_SET_PHASE, GOVERNOR_CLR_PHASE_VARIABLES, 
 * GOVERNOR_GET_PHASE_VARIABLE and GOVERNOR_SET_PHASE_VARIABLE. The IOctl commands
 * are prototyped in the AI_gov_ioctl.h header which is included in user and kernal
 * space such that both are aware of the commands available. The functionality
 * of the commands is implemented within kernel space, user space only knowing what
 * to send to kernel space or what to recieve from kernel space, both done via
 * copy_to_user and copy_from_user calls. Below all the components required for
 * implementing and using an IOctl command.
 *
 * @subsubsubsection ioctl_defining_comands How to define new IOctl commands
 * In the header file AI_gov_ioctl.h all the IOctl commands must be defined
 * using on of the following IOctl macros: _IO, _IOR, _IOW or _IORW, all of which
 * can be found within the ioctl.h header. The macro must be given the magic number
 * unique to the device, in this case 'g', a IOcrlt command number, unique to that
 * command and the data type which the command will use, if it requires a data
 * type. The structure is thus _IO##(magic_number, command_number, data_type).
 * <br><br>
 *
 * @subsubsubsection ioctl_kernelspace_implementation Kernel space implementation
 * All kernel side handling of IOctl commands is implemented within the file
 * AI_gov_ioctl.c, in the function AI_gov_ioctl. All the IOctl file operations
 * functions can be found in the structure AI_governor_fops, which is used when
 * initializing the cdev using in the IOctl interface. The command number
 * defined within the IOctl header is used in a switch statement within this function
 * to handle each IOctl call, thus implementing a new IOctl command is as easy as
 * adding a case to this switch. Getting and sending data to user space is done
 * via copy_to_user and copy_from_user as mentioned before.
 *
 * @subsubsubsection ioctl_userspace_implementation User space implementation
 * In a user space application, the IOctl interface can be accessed using the
 * ioctl call found in sys/ioctl.h. The call takes the IOctl file, IOctl command
 * name and the data structure to be passed to the command, if applicable. 
 * An example call
 @verbatim
 char *file_name = "/dev/AI_governor_ioctl"; 
 fd = open(file_name, O_RDWR);      
 struct AI_gov_ioctl_phase_variable g;
 ioctl(fd, GOVERNOR_GET_PHASE_VARIABLE, &g);
 @endverbatim
 * 
 * @subsubsection ioctl_kernelspace_sec Kernel Space
 * All kernel space backend can be implemented in a normal fashion, simply 
 * triggered from the AI_gov_ioctl function.
 *
 * @subsection sysfs_sec Sysfs
 * The sysfs interface shows the phase profiles and their attributes in
 * real time, dynamically changing what is displayed when different 
 * states are entered into in the state machine. The sysfs interface is also
 * generated using the FOR_EACH_PHASE marcro. The sysfs system comes down to 
 * initializing a kobject for each phase, having a show and store functions
 * for each phase, creating the attribute group and the attributes for each phase
 * and then allowing the state machine to load and unload the active phases'
 * kobject into the CPUfreq sysfs folder such that the active phases' sysfs
 * attribtues can be seen.
 * <br><br>
 * Within the AI_gov_phases.h header, each phase must have its attributes
 * declared using a macro of the style 
 @verbatim 
    SYSFS_##phase name##_ATTRIBS(ATTRB) \
    ATTRB(phase name, attribute name)
 @endverbatim
 * This addition of the attributes will allow for each attribute for each
 * phase to have functions and sysfs attributes created such that they can 
 * be shown and stored. Each sysfs attribute is shown via a function of the 
 * format "show_##phase name##_##attribute name##" and stored with a similar
 * function "store_##phase name##_##attribute name##". All phases attributes
 * are added to a sysfs group for each phase which are in turn attached to
 * each phases' kobject, stored within the phase_profile data structure for
 * each phase.
 *
 * @section exec_sec Execution of the AI governor
 * The overall execution "loop" that the AI governor executes is very similar
 * to that of the chrome governor. The governor is registered as a kernel module
 * using the struct cpufreq_gov_AI where the function cpufreq_governor_AI is given
 * as the managing function. Within cpufreq_governor_AI the modules initilization,
 * deinitilization, starting and stopping is handled. 
 * <br><br>
 * The meat of the governor happens within cpufreq_AI_governor_speedchange_task
 * where AI_coordiator is called each time the task executes. In AI_coordinator
 * all task logic can be added. In the most basic case, all that is done is the
 * current phase profile's run function pointer is called, thus executing the 
 * run function of the current phase, executing it's background power management
 * logic. This can be seen in AI_gov_power_manager.c.
 */

#ifndef AI_GOV_H_
#define AI_GOV_H_

/* -- Includes -- */
/* Kernel includes. */
#include <linux/timer.h>
#include <linux/rwsem.h>

/* Governor includes. */
#include "AI_gov_types.h"

extern struct cpufreq_policy *AI_policy;
/**
* @brief 
*/
extern struct cpufreq_AI_gov_tunables *common_tunables_AI;
/**
* @brief 
*/
extern struct cpufreq_AI_gov_tunables *tuned_parameters_AI;

/**
* @brief The main global data struct of the AI_gov.
*
* The global AI_gov data structure stores all important information relating to 
* the governor, its state and its profiles. It also contans a number of pointers
* to important system structs.
*/
extern struct AI_gov_info* AI_gov;

/**
* @brief Removes the kobject from sysfs for a specific phase profile
*
* This macro must have access to a phase_profile* called temp_phase
* that it will use to get the appropriate kobject for the specified
* phase which it will then deinit and remove for the sysfs heirarchy.
*/
#define DEINIT_PHASE_KOBJECT(PHASE)	\
				temp_phase = AI_phases_get_name(#PHASE); \
				kobject_del(temp_phase->kobj); \

/**
* @brief Sets the permissable task name length
*/
#define TASK_NAME_LEN 15

#ifndef FAST_RESCHEDULE
#define FAST_RESCHEDULE (2 * USEC_PER_MSEC)
#endif

/**
* @brief Sysfs owner
*/
#ifndef AID_SYSTEM
#define AID_SYSTEM	(1000)
#endif

/**
* @brief Default timer period
*/
#define DEFAULT_TIMER_RATE (20 * USEC_PER_MSEC)

/**
* @brief Governor's default target workload
*/
#define DEFAULT_TARGET_LOAD 90

//static int AI_touch_nb_callback(void);

#endif /*  AI_GOV_H_ */ 
