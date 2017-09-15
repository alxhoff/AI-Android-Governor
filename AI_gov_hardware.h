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

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

struct AI_gov_freq_table{
	uint32_t LITTLE_MIN;
	uint32_t LITTLE_MAX;
	uint8_t num_freq_steps_LITTLE;
	uint32_t *freq_steps_LITTLE;

	#ifdef CPU_IS_BIG_LITTLE
	uint32_t BIG_MIN;
	uint32_t BIG_MAX;
	uint8_t num_freq_steps_BIG;
	uint32_t *freq_steps_BIG;
	#endif
};

struct AI_gov_cur_HW {
	bool is_big_little;

	uint8_t cpu_count;

	uint32_t little_freq;

	bool has_table;
	struct AI_gov_freq_table* freq_table;

#ifdef CPU_IS_BIG_LITTLE
	bool big_state;

	uint32_t big_freq;
#endif /* Enable or disable second core frequency */

	void* stats;
};

#endif /* AI_GOV_HARDWARE_H_ */
