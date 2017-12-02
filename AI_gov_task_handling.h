/*
 * AI_gov_task_handling.h
 *
 *  Created on: Sep 8, 2017
 *      Author: alxhoff
 */

#ifndef AI_GOV_TASK_HANDLING_H_
#define AI_GOV_TASK_HANDLING_H_

#include "AI_gov.h"

/**
* @brief
*
*
*
* @param
* @return
*/
void AI_tasks_init_ring_buffer(ring_buffer_t * ring_buffer,
		unsigned int size);

/**
* @brief
*
*
*
* @param
* @return
*/
void AI_tasks_add_data_to_ringbuffer(ring_buffer_t * ring_buffer,
		int64_t workload);

#endif /* AI_GOV_TASK_HANDLING_H_ */
