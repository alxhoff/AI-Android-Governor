/*
 * AI_gov_power_manager.c
 *
 *  Created on: Sep 8, 2017
 *      Author: alxhoff
 */

#include <linux/kernel.h>
#include <linux/kernel_stat.h>

#include <linux/cpufreq.h>
#include <linux/module.h>
#include <linux/percpu-defs.h>
#include <linux/types.h>
#include <linux/cpu.h>

#include <linux/timer.h>

#ifdef CONFIG_ANDROID
#include <asm/uaccess.h>
#include <linux/syscalls.h>
#include <linux/android_aid.h>
#endif
#ifdef CONFIG_ARM_EXYNOS_MP_CPUFREQ
#include <mach/cpufreq.h>
#endif

#include "AI_gov_power_manager.h"
#include "AI_gov.h"
#include "AI_gov_task_handling.h"

#define NUMBER_CPUS 8

// data structure for the workload prediction
//TODO CHECKS ON CPU COUNT
ring_buffer_t AI_workload_history_cpus[NUMBER_CPUS];
static short ring_buffer_initialized = 0;

void AI_pm_init_wma_buffers(void)
{
	int i = 0;
	if (ring_buffer_initialized == 0) {
		for (i = 0; i < NUMBER_CPUS; i++) {
			AI_tasks_init_ring_buffer(&(AI_workload_history_cpus[i]),
					SIZE_WORKLOAD_HISTORY);
		}
		ring_buffer_initialized = 1;
	}
}
