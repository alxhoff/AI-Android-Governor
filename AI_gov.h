
/**
 * @file AI_gov.h
 * @author Alex Hoffman
 * @date 11 October 2017
 * @brief Main routines for the AI governor.
 * @section About
 * The AI governor enters and exits use through the routines outlines within
 * the file. All backend logic is initialized from this file, timers, tasks etc.
 * @mainpage Application Interface (AI) CPUfreq Governor
 * @section intro_sec Introduction
 * The AI governor aim to be act as an API interface for userspace applications
 * to dynamically change and interact with the backend governing logic managing
 * the CPU frequency.
 */

#ifndef AI_GOV_H_
#define AI_GOV_H_

/* -- Includes -- */
/* Kernel includes. */
#include <linux/timer.h>
#include <linux/rwsem.h>

/* Governor includes. */
#include "AI_gov_types.h"

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
