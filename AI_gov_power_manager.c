/*
 * AI_gov_power_manager.c
 *
 *  Created on: Sep 8, 2017
 *      Author: alxhoff
 */

/* -- Includes -- */
/* Kernel includes. */
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

/* Governor includes. */
#include "AI_gov_power_manager.h"
#include "test_flags.h"
#include "AI_gov_task_handling.h"
#include "AI_gov_kernel_write.h"
#include "AI_gov_sched.h"

extern DEFINE_PER_CPU(struct cpufreq_AI_governor_cpuinfo, cpuinfo);
#define NUMBER_CPUS 8

// data structure for the workload prediction
//TODO CHECKS ON CPU COUNT
ring_buffer_t AI_workload_history_cpus[NUMBER_CPUS];
static short ring_buffer_initialized = 0;

void AI_coordinator(void)
{
//	int i = 0;
//	unsigned int total_workload = 0;
//	unsigned flags, workload[8];
//	struct cpufreq_AI_governor_cpuinfo *pcpu_tmp = 0;
//	static enum PHASE_ENUM current_phase;
//	enum PHASE_ENUM previous_phase;



	//TODO workload calculation fix

//	for_each_online_cpu(i) {
//			pcpu_tmp = &per_cpu(cpuinfo, i);
//			KERNEL_DEBUG_MSG(" [GOVERNOR] IN COORDINATOR 1");
//
//			spin_lock_irqsave(&pcpu_tmp->load_lock, flags);
//			KERNEL_DEBUG_MSG(" [GOVERNOR] IN COORDINATOR 2");
//
//			workload[i] = AI_sched_update_load(i, pcpu_tmp);
//			KERNEL_DEBUG_MSG(" [GOVERNOR] IN COORDINATOR 3");
//
//			total_workload += workload[i];
//			KERNEL_DEBUG_MSG(" [GOVERNOR] IN COORDINATOR 4");
//
//			AI_tasks_add_data_to_ringbuffer(&(AI_workload_history_cpus[i]), workload[i]);
//			KERNEL_DEBUG_MSG(" [GOVERNOR] IN COORDINATOR 5");
//
//			spin_unlock_irqrestore(&pcpu_tmp->load_lock, flags);
//	}

	//TASK HANDLING HERE

//	//get cpu freq
//	uint32_t little_freq = AI_gov->hardware->little_freq;
//
//#ifdef CPU_IS_BIG_LITTLE
//	uint32_t big_freq = AI_gov->hardware->big_freq;
//#endif

	//RUN CURRENT PHASE

	if(AI_gov->current_profile->run != NULL) AI_gov->current_profile->run();

//	switch(AI_gov->phase){
//	case AI_init:
//		break;
//	case AI_framerate:
//		break;
//	case AI_priority:
//		break;
//	case AI_time:
//		break;
//	case AI_powersave:
//		break;
//	case AI_performance:
//		break;
//	case AI_response:
//		break;
//	case AI_exit:
//		break;
//	default:
//		break;
//	}
}

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
