/*
 * AI_gov_phases.h
 *
 *  Created on: Aug 31, 2017
 *      Author: alxhoff
 */

#ifndef AI_GOV_PHASES_H_
#define AI_GOV_PHASES_H_

#include "test_flags.h"

typedef enum{
	AI_phase_init,
	AI_phase_response,
	AI_phase_animation,
	AI_phase_idle,
	AI_phase_load
} phase_state;

phase_state AI_phases_getBrowsingPhase(void);
int AI_phases_touch_nb(void);
void AI_phases_init(void);

#endif /* AI_GOV_PHASES_H_ */
