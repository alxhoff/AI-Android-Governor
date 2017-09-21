/*
 * cpufreq_alextest.h
 *
 *  Created on: Aug 31, 2017
 *      Author: alxhoff
 */

#ifndef AI_GOV_H_
#define AI_GOV_H_

#include <linux/timer.h>
#include <linux/rwsem.h>

#include "AI_gov_hardware.h"
#include "AI_gov_phases.h"
#include "AI_gov_types.h"

#define TASK_NAME_LEN 15

//typedef enum{
//	response,
//	animation,
//	idle,
//	load
//} phase_state;

extern struct cpufreq_AI_governor_tunables *common_tunables_AI;
extern struct AI_gov_info* AI_gov;

#ifndef FAST_RESCHEDULE
#define FAST_RESCHEDULE (2 * USEC_PER_MSEC)
#endif


void AI_phase_change(void);
//static int AI_touch_nb_callback(void);

void cpufreq_AI_governor_timer_resched(unsigned long expires);


#endif /* AI_GOV_H_ */
