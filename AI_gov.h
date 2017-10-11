
/**
 * @file AI_gov.h
 * @author Alex Hoffman
 * @date 11 October 2017
 * @brief Main routines for the AI governor.
 *
 * The AI governor enters and exits use throughn the routines outlines within
 * the file. All backend logic is initialized from this file, timers, tasks etc.
 * @mainpage Application Interface (AI) CPUfreq Governor
 * @section intro_sec Introduction
 * The AI governor aim to be act as an API interface for userspace applications
 * to dynamically change and interact with the backend governing logic managing
 * the CPU frequency.
 */

#ifndef AI_GOV_H_
#define AI_GOV_H_

#include <linux/timer.h>
#include <linux/rwsem.h>

#include "AI_gov_types.h"

extern struct cpufreq_AI_gov_tunables *common_tunables_AI;
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
* @def Removes the kobject from sysfs for a specific phase profile
*
* This macro must have access to a phase_profile* called temp_phase
* that it will use to get the appropriate kobject for the specified
* phase which it will then deinit and remove for the sysfs heirarchy.
*/
#define DEINIT_PHASE_KOBJECT(PHASE)	\
				temp_phase = AI_phases_get_name(#PHASE); \
				kobject_del(temp_phase->kobj); \

/**
* @def Sets the permissable task name length
*/
#define TASK_NAME_LEN 15

#ifndef FAST_RESCHEDULE
#define FAST_RESCHEDULE (2 * USEC_PER_MSEC)
#endif

//static int AI_touch_nb_callback(void);

#endif /* AI_GOV_H_ 
