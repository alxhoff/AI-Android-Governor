/*
 * AI_gov_sched.h
 *
 *  Created on: Sep 5, 2017
 *      Author: alxhoff
 */

#ifndef AI_GOV_SCHED_H_
#define AI_GOV_SCHED_H_



enum AI_CORE {
	L0 = 0,
	L1 = 1,
	L2 = 2,
	L3 = 3,
#ifdef CPU_IS_BIG_LITTLE
	B0 = 4,
	B1 = 5,
	B2 = 6,
	B3 = 7
#endif
};

enum AI_CPU{
	CPU_NONE 		= 0x00,
	LITTLE 			= 0x0F,
#ifdef CPU_IS_BIG_LITTLE
	BIG				= 0xF0,
	BIG_AND_LITTLE	= 0xFF
#endif
};

#endif /* AI_GOV_SCHED_H_ */
