/*
 * AI_gov_hardware.h
 *
 *  Created on: Sep 5, 2017
 *      Author: alxhoff
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

signed int AI_gov_init(struct AI_gov_info** in);

#endif /* AI_GOV_HARDWARE_H_ */
