/**
 * @file AI_gov_power_manager.h
 * @author Alex Hoffman
 * @date 10 March 2018
 * @brief Handles the looping power management of the governor
 */

#ifndef AI_GOV_POWER_MANAGER_H_
#define AI_GOV_POWER_MANAGER_H_

/* -- Includes -- */
/* Governor includes. */
#include "AI_gov.h"

/**
* @brief
*
*
*
* @param
* @return
*/
void AI_pm_init_wma_buffers(void);

/**
* @brief Co-ordinates the governor's backend logic
*
* The governor's task calls this task to recalculate (co-ordinate)
* the backend logic of the governor's current phase. It does this
* by calculating the system's current workload and calling the 
* current profile's run function.
*
* @return void
*/
void AI_coordinator(void);


#endif /* AI_GOV_POWER_MANAGER_H_ */
