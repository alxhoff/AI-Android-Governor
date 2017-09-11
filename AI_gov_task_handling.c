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
