/*
 * AI_gov_hardware.c
 *
 *  Created on: Sep 7, 2017
 *      Author: alxhoff
 */

#include <linux/slab.h>

#include "AI_gov_hardware.h"

signed int AI_gov_hardware_init(struct AI_gov_cur_HW* hardware, struct AI_gov_freq_table* freq_table, uint32_t little_freq
#ifdef CPU_IS_BIG_LITTLE
							,uint32_t big_freq, bool big_state
#endif	
	)
{
	if(hardware->has_table == TRUE)
		return -1;

#ifdef CPU_IS_BIG_LITTLE
	hardware->is_big_little = TRUE;
	hardware->big_state = big_state;
	hardware->big_freq = big_freq;
#endif	
	hardware->little_freq = little_freq;

	hardware->freq_table = freq_table;
	hardware->has_table = TRUE;

	return 0;
}

signed int AI_gov_change_freq_table(struct AI_gov_cur_HW* hardware, struct AI_gov_freq_table* freq_table )
{
	if(hardware->has_table == TRUE)
		kfree(hardware->freq_table);
	else
		return -1;

	hardware->freq_table = freq_table;

	return 0;
}
