/*
 * AI_gov_task_handling.c
 *
 *  Created on: Sep 8, 2017
 *      Author: alxhoff
 */


#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/types.h>

#include "AI_gov_task_handling.h"
#include "AI_gov_kernel_write.h"
#include "AI_gov_sched.h"

void AI_tasks_init_ring_buffer(ring_buffer_t * ring_buffer, unsigned int size)
{
	int i = 0;

	ring_buffer->size = size;
	//KERNEL_DEBUG_MSG("[TASKS] Init ring buffer\n");
	for (i = 0; i < (ring_buffer->size - 1); i++) {
		ring_buffer->data[i].next = &(ring_buffer->data[i + 1]);
		ring_buffer->data[i].workload = 0;
	}
	// point last element to first element
	ring_buffer->data[ring_buffer->size - 1].next = &(ring_buffer->data[0]);
	ring_buffer->data[ring_buffer->size - 1].workload = 0;
	// set first element
	ring_buffer->first = &(ring_buffer->data[0]);
	ring_buffer->first->next = ring_buffer->data[0].next;
}

/**
 * Add the number and move first pointer;
 */
void AI_tasks_add_data_to_ringbuffer(ring_buffer_t * ring_buffer,
		int64_t workload)
{

	if (ring_buffer->first == 0)
		KERNEL_DEBUG_MSG("[TASKS] Error, first element is null\n");
	else {
		ring_buffer->first->workload = workload;
		if (ring_buffer->first->next == 0) {
			KERNEL_DEBUG_MSG("[TASKS] Error, ring_buffer->first->next is null %x\n", (int) ring_buffer->first);
			return;
		} else
			ring_buffer->first = ring_buffer->first->next;
	}
}
