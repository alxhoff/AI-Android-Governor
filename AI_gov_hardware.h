
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
* @brief
*
*
*
* @param
* @return
*/
signed int AI_gov_init(struct AI_gov_info** in);

#endif /* AI_GOV_HARDWARE_H_ */
