 PHASE_ENUM values
/**
 * @file AI_gov_hardware.h
 * @author Alex Hoffman
 * @date 11 October 2017 
 * @brief Handles the governor's hardware functionality
 * @section About
 * The hardware file handles all of the governors hardware management, or will
 * in the future. The governor's init is also in here as it will have a lot of
 * hardare functions at a later date.
 */

#ifndef AI_GOV_HARDWARE_H_
#define AI_GOV_HARDWARE_H_

#include <linux/types.h>

#include "test_flags.h"
#include "AI_gov_types.h"

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

/**
* @brief Initialisation function used to allocate and initialise the core data
* structures used by the AI governor.
*
* Memory is allocated for the main AI_gov_info struct which is passed to the function
* as a double pointer, the structs hardware struct is also calloc'd.
*
* @param in Double pointer to the pointer that is to store the reference to the 
* struct AI_gov_info object.
* @return 0 on sucess
*/
signed int AI_gov_init(struct AI_gov_info** in);

/**
* @brief Changes the active AI_gov_freq_table currently loaded into the governor
*
* @param hardware Pointer to the AI gov hardware structer where the frequency
* table should be loaded
* @param freq_table Pointer to the frequency table object that is to be loaded
* @return 0 on sucess 
*/
signed int AI_gov_change_freq_table(struct AI_gov_cur_HW* hardware,
		struct AI_gov_freq_table* freq_table )

#endif /* AI_GOV_HARDWARE_H_ */
