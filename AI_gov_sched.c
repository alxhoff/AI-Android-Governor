/*
 * AI_gov_sched.c
 *
 *  Created on: Sep 5, 2017
 *      Author: alxhoff
 */

#include <linux/cpu.h>
#include <linux/sched.h>
#include <linux/cpufreq.h>
#include <linux/kernel_stat.h>
#include <linux/percpu-defs.h>
#include <linux/netdevice.h>

#include <linux/tick.h>
#include <linux/time.h>
#include <asm/cputime.h>
#include "cpu_load_metric.h"

#include "AI_gov_sched.h"
#include "test_flags.h"
#include "AI_gov_hardware.h"
#include "AI_gov_kernel_write.h"

static struct AI_cores {
	uint8_t LC0 :1;
	uint8_t LC1 :1;
	uint8_t LC2 :1;
	uint8_t LC3 :1;
#ifdef CPU_IS_BIG_LITTLE
	uint8_t BC0 :1;
	uint8_t BC1 :1;
	uint8_t BC2 :1;
	uint8_t BC3 :1;
#endif
} _managedCores = { 0 };

#define managedCores (*((uint8_t *)&_managedCores));

#define AI_SCHED_WORKLOAD_HISTORY_SIZE_TOTAL 3
#define AI_SCHED_WORKLOAD_HISTORY_SIZE_LONG 10
#define CRITICAL_WORKLOAD_MARGIN_LITTLE 90
#define CRITICAL_WORKLOAD_MARGIN_BIG 90

// avoiding race conditions while updating the data
//static DEFINE_RWLOCK(hotplug_rwlock);
static DEFINE_MUTEX(hotplugMutex);

// workload margin for load idle phase
#define LOAD_PHASE_MARGIN 200

//AI_gov->hardware

static unsigned int AI_total_workload_BIG[AI_SCHED_WORKLOAD_HISTORY_SIZE_LONG] = {0};
static unsigned int AI_total_workload_LITTLE[AI_SCHED_WORKLOAD_HISTORY_SIZE_LONG] = {0};

static bool AI_critical_workload_BIG[AI_SCHED_WORKLOAD_HISTORY_SIZE_LONG] =
		{ 0 };
static bool AI_critical_workload_LITTLE[AI_SCHED_WORKLOAD_HISTORY_SIZE_LONG] =
		{ 0 };

extern DEFINE_PER_CPU(struct cpufreq_AI_governor_cpuinfo, cpuinfo);

unsigned int AI_sched_get_smoothed_workload(enum AI_CPU cpu) {
	// we cannot divide in kernel space, so we simply shift two positions what is divide by 4
#ifdef CPU_IS_BIG_LITTLE
	if (cpu == BIG)
		return (AI_total_workload_BIG[3] + AI_total_workload_BIG[2]
				+ AI_total_workload_BIG[1] + AI_total_workload_BIG[0]) >> 2;
	if (cpu == BIG_AND_LITTLE)
		return (AI_total_workload_BIG[3] + AI_total_workload_BIG[2]
				+ AI_total_workload_BIG[1] + AI_total_workload_BIG[0]
				+ AI_total_workload_LITTLE[3] + AI_total_workload_LITTLE[2]
				+ AI_total_workload_LITTLE[1] + AI_total_workload_LITTLE[0]) >> 4;
#endif
	if (cpu == LITTLE)
			return (AI_total_workload_LITTLE[3] + AI_total_workload_LITTLE[2]
					+ AI_total_workload_LITTLE[1] + AI_total_workload_LITTLE[0]) >> 2;
	return 0;
}

bool AI_sched_criticalWorkloadPhase(uint8_t history_length, enum AI_CPU cpu) {
	bool ret = true;
	uint8_t i = 0;
	// the history value is true if the workload is actually critical
	if (cpu == LITTLE) {
			for (i = 0; i < history_length; i++) {
				ret &= AI_critical_workload_BIG[i];
			}
	}
#ifdef CPU_IS_BIG_LITTLE
	else if (cpu == BIG) {
		for (i = 0; i < history_length; i++) {
			ret &= AI_critical_workload_BIG[i];
		}
	} else if (cpu == BIG_AND_LITTLE) {
		for (i = 0; i < history_length; i++) {
			ret &= (AI_critical_workload_BIG[i] & AI_critical_workload_BIG[i]);
		}
	}
#endif
	//KERNEL_LOGGG_MSG("[PHASES] workload %d %d\n",history_length, ret);
	return ret;
}

bool AI_sched_workloadBelowLoadingMargin(uint8_t history_length) {
	uint8_t i = 0;
	bool ret = true;
	for (i = 0; i < history_length; i++) {
		ret &= (AI_total_workload_LITTLE[i] + AI_total_workload_BIG[i])
				< LOAD_PHASE_MARGIN;
	}
	//KERNEL_LOGGG_MSG("[PHASES] workload %d %d\n",history_length, ret);
	return ret;
}

void AI_sched_updateWorkLoadHistory(unsigned int total_workload,
		bool critial_workload, uint8_t cpu) {
	uint8_t i = 0;

	if (cpu == LITTLE) {
		for (i = AI_SCHED_WORKLOAD_HISTORY_SIZE_LONG - 1; i > 0; i--) {
			AI_critical_workload_LITTLE[i] = AI_critical_workload_LITTLE[i - 1];
			AI_total_workload_LITTLE[i] = AI_total_workload_LITTLE[i - 1];
		}
		AI_total_workload_LITTLE[0] = total_workload;
		AI_critical_workload_LITTLE[0] = critial_workload;
	} else {
		for (i = AI_SCHED_WORKLOAD_HISTORY_SIZE_LONG - 1; i > 0; i--) {
			AI_critical_workload_BIG[i] = AI_critical_workload_BIG[i - 1];
			AI_total_workload_BIG[i] = AI_total_workload_BIG[i - 1];
		}
		AI_total_workload_BIG[0] = total_workload;
		AI_critical_workload_BIG[0] = critial_workload;
	}
}

void AI_sched_update_workload_interactive(int workload, uint8_t core) {
	static uint8_t LITTLE_counter = 0;
	static uint8_t BIG_counter = 0;

	static bool LITTLE_critical = false;
	static bool BIG_critical = false;

	static unsigned int LITTLE_tmp_workload = 0;
	static unsigned int BIG_tmp_workload = 0;

	unsigned long flags;
	struct cpufreq_AI_governor_cpuinfo *pcpu_tmp = 0;

	pcpu_tmp = &per_cpu(cpuinfo, core);
	spin_lock_irqsave(&pcpu_tmp->load_lock, flags);
	// workload for
	if (core <= LC0) {
		LITTLE_tmp_workload += (unsigned int) workload;
		LITTLE_critical |= (workload > CRITICAL_WORKLOAD_MARGIN_LITTLE);
		LITTLE_counter++;
		if (LITTLE_counter == 4) {
			AI_sched_updateWorkLoadHistory(LITTLE_tmp_workload, LITTLE_critical, LITTLE);
			LITTLE_critical = false;
			LITTLE_tmp_workload = 0;
			LITTLE_counter = 0;
		}
	}
#ifdef CPU_IS_BIG_LITTLE
	else {
		BIG_tmp_workload += (unsigned int) workload;
		BIG_critical |= (workload > CRITICAL_WORKLOAD_MARGIN_BIG);
		BIG_counter++;
		if (BIG_counter == 4) {
			AI_sched_updateWorkLoadHistory(BIG_tmp_workload,
					BIG_critical, BIG);
			BIG_critical = false;
			BIG_tmp_workload = 0;
			BIG_counter = 0;
		}
	}
#endif
	spin_unlock_irqrestore(&pcpu_tmp->load_lock, flags);
}

void AI_sched_update_workload(void) {
	unsigned int total_workload = 0, i = 0;
	unsigned int total_workload_LITTLE = 0;
	unsigned total_workload_BIG = 0;
	unsigned long flags;
	unsigned int workload[8] = { 0 };
	// find if one of the cores is overloaded
	bool critical_workload_LITTLE = false;
	bool critical_workload_BIG = false;
	struct cpufreq_AI_governor_cpuinfo *pcpu_tmp = 0;

	for_each_online_cpu(i) {
		pcpu_tmp = &per_cpu(cpuinfo, i);
		spin_lock_irqsave(&pcpu_tmp->load_lock, flags);
		if (cpu_online(i)) {
			workload[i] = AI_sched_update_load(i, pcpu_tmp);
			if (workload[i] < 0 || workload[i] > 100)
				workload[i] = 0;
		} else
			workload[i] = 0;
		total_workload += workload[i];
//#ifdef TASK_LOGGER
//		if (AI_log_is_logging()) {
//			log_struct.workload[log_struct.global_index][i] = workload[i];
//		}
//#endif
		//		AI_tasks_add_data_to_ringbuffer(&(workload_history_cpus[i]), workload[i]);
		//		prediction[i] = wma(workload_history_cpus[i], SIZE_WORKLOAD_HISTORY);
		//				if (prediction[i] > 95) {
		//					KERNEL_ERROR_MSG("[PM] Expected high workload %d on core %d\n", prediction[i], i);
		//
		//				}
		spin_unlock_irqrestore(&pcpu_tmp->load_lock, flags);
	};
#ifdef CPU_IS_BIG_LITTLE
	total_workload_BIG = workload[BC0] + workload[BC1] + workload[BC2]
			+ workload[BC3];
	critical_workload_BIG = (workload[BC0] > CRITICAL_WORKLOAD_MARGIN_BIG)
				|| (workload[BC1] > CRITICAL_WORKLOAD_MARGIN_BIG)
				|| (workload[BC2] > CRITICAL_WORKLOAD_MARGIN_BIG)
				|| (workload[BC3] > CRITICAL_WORKLOAD_MARGIN_BIG);
	AI_sched_updateWorkLoadHistory(total_workload_BIG, critical_workload_BIG,
				BIG);
#endif
	total_workload_LITTLE = workload[LC0] + workload[LC1] + workload[LC2]
			+ workload[LC3];

	critical_workload_LITTLE = (workload[LC0] > CRITICAL_WORKLOAD_MARGIN_LITTLE)
			|| (workload[LC1] > CRITICAL_WORKLOAD_MARGIN_LITTLE)
			|| (workload[LC2] > CRITICAL_WORKLOAD_MARGIN_LITTLE)
			|| (workload[LC3] > CRITICAL_WORKLOAD_MARGIN_LITTLE);

	AI_sched_updateWorkLoadHistory(total_workload_LITTLE, critical_workload_LITTLE, LITTLE);
}
//TODO FREQ TABLE FROM CPUINFO POLICY?
#ifdef CPU_IS_BIG_LITTLE
bool AI_sched_get_BIG_state(void) {
	return AI_gov->hardware->big_state;
}

void AI_sched_set_BIG_state(bool state)
{
	AI_gov->hardware->big_state = state;
}

uint32_t AI_sched_get_BIG_freq(void) {
	if (AI_gov->hardware->big_state) {
		if (AI_gov->hardware->big_freq == 0)
			AI_gov->hardware->big_freq = AI_gov->hardware->freq_table->BIG_MIN;
		return AI_gov->hardware->freq_table->BIG_MIN;
	} else
		return 0;
}

void AI_sched_set_BIG_freq(uint32_t frequency) {
	AI_gov->hardware->big_freq = frequency;
}

uint32_t AI_sched_get_BIG_min_freq(void)
{
	return AI_gov->hardware->freq_table->BIG_MIN;
}

uint32_t AI_sched_get_BIG_max_freq(void)
{
	return AI_gov->hardware->freq_table->BIG_MAX;
}

#endif

uint32_t AI_sched_get_LITTLE_freq(void) {
	return AI_gov->hardware->little_freq;
}

uint32_t AI_sched_get_LITTLE_min_freq(void)
{
	return AI_gov->hardware->freq_table->LITTLE_MIN;
}

uint32_t AI_sched_get_LITTLE_max_freq(void)
{
	return AI_gov->hardware->freq_table->LITTLE_MAX;
}

void AI_sched_set_LITTLE_freq(uint32_t frequency) {
	AI_gov->hardware->little_freq = frequency;
}

// int AI_sched_assignFrequency(unsigned int frequency, enum AI_CPU cpu) {
// 	int retval = -EINVAL;
// 	enum AI_CORE core = 0;
// 	struct cpufreq_policy *policy = 0;

// 	//	if (!AI_sched_cpuManaged(cpu)) {
// 	//		KERNEL_ERROR_MSG( "[SCHED] AI_Governor: Errorneous input: %d %d\n", frequency, cpu);
// 	//		return retval;
// 	//	}
// #ifdef CPU_IS_BIG_LITTLE
// 	if (cpu == BIG && !AI_sched_get_BIG_state) {
// 		KERNEL_ERROR_MSG( "[SCHED] AI_Governor: A15 down, no freq change possible: %d %d\n", frequency, cpu);
// 		return retval;
// 	}

// 	// cpufreq_cpu_get needs core, NOT CPU. Get the first core on cpu by default
// 	core = (cpu == LITTLE) ? LC0 : BC0;
// #else
// 	core = LC0;
// #endif	
// 	policy = cpufreq_cpu_get(core);

// 	if (policy == 0) {
// 		KERNEL_ERROR_MSG( "[SCHED] AI_Governor: Not possible to acquire cpu: %d %d\n", frequency, cpu);
// 		return retval;
// 	}

// 	if ((retval = cpufreq_driver_target(policy, frequency, CPUFREQ_RELATION_H))
// 			!= 0) {
// 		KERNEL_ERROR_MSG( "[SCHED] AI_Governor: Error setting frequency"
// 				" for cores %x\n", cpu);
// 	}

// 	cpufreq_cpu_put(policy);
// 	//	if ((A7_freq != frequency && cpu == A7) || (A15_freq != frequency && cpu
// 	//			== A15))
// 	//		KERNEL_ERROR_MSG("[SCHED] AI_Governor: assign frequency %d to cpu %d!\n", frequency, (cpu == A15 ? 15 : 7));

// 	if (retval == 0) {
// 		if (cpu == LITTLE)
// 			AI_sched_set_BIG_freq(frequency);
// 		else
// 			AI_sched_set_BIG_freq(frequency);
// 	}
// 	return retval;
// }

int AI_sched_assignFrequency(unsigned int frequency)
{
	int retval = -EINVAL;
	enum AI_CORE core = 0;
	struct cpufreq_policy *policy = 0;

#ifdef CPU_IS_BIG_LITTLE
	//check big core is online
	if(!AI_gov->hardware->big_state){
		//TODO FIX ERROR MESSAGE
		KERNEL_ERROR_MSG( "[SCHED] AI_Governor: BIG down, no freq change possible: %d\n", frequency);
		return retval;
	}

	//big little enabled and big is online
	//get policy
	core = BC0;
	policy = cpufreq_cpu_get(core);

	//TODO ERROR CHECK SOMEHOW

	//TODO deal with freq
	if( (retval = cpufreq_driver_target(policy, frequency, CPUFREQ_RELATION_H)) != 0){
		KERNEL_ERROR_MSG( "[SCHED] AI_Governor: Error setting frequency"
						" for cores \n");
	}

	if(retval == 0)
		AI_gov->hardware->big_freq = frequency;

	cpufreq_cpu_put(policy);
#endif

	//now set little freq

	core = LC0;
	policy = cpufreq_cpu_get(core);

	if( (retval = cpufreq_driver_target(policy, 1400000, CPUFREQ_RELATION_H)) != 0){
			KERNEL_ERROR_MSG( "[SCHED] AI_Governor: Error setting frequency"
							" for cores\n" );
		}

	if(retval == 0)
		AI_gov->hardware->little_freq = frequency;

	cpufreq_cpu_put(policy);

	return retval;
}


#ifdef CPU_IS_BIG_LITTLE
int AI_sched_setAffinity(struct task_struct *task, enum AI_CORE core) {
	if (!AI_sched_coreManaged(core)) {
		KERNEL_ERROR_MSG( "[SCHED] AI_Governor: Unmanaged core %u\n",
				core);
		return -EINVAL;
	}


	if (AI_sched_get_BIG_state() == 0 && core > LC3)
		return -EINVAL;

	if (!cpu_online(core)) {
		KERNEL_ERROR_MSG( "[SCHED] AI_Governor: Cannot assign to core"
				"%u\n", core);
		return -EINVAL;
	}

	return sched_setaffinity(task->pid, get_cpu_mask(core));
}
#endif

int AI_sched_pushToCpu(struct task_struct *task, enum AI_CPU cpu) {
	if (!AI_sched_cpuManaged(cpu)) {
		KERNEL_ERROR_MSG( "[SCHED] AI_Governor: Unmanaged core on cpu %u\n",
				cpu );
		return -EINVAL;
	}

	//	if (sched_enableCpu(cpu) != 0) {
	//		KERNEL_ERROR_MSG( "[SCHED] AI_Governor: Cannot assign to cpu %x\n",
	//				cpu);
	//		return -EINVAL;static uint8_t A15_enabled = 0;
	//	}

	return sched_setaffinity(task->pid, ((cpumask_t*) &cpu));
}

int AI_sched_pushGroupToCpu(struct task_struct *task, enum AI_CPU cpu) {
	size_t count = 0;
	struct task_struct *buffer, *groupTask;

	if (task == NULL)
		task = current;

	list_for_each_entry_safe( groupTask, buffer,
			&(sched_AI_groupLeader(task)->thread_group), thread_group ) {
		int errorCode = 0;
		if ((errorCode = AI_sched_pushToCpu(groupTask, cpu)))
			return errorCode;
		++count;
	}

	return AI_sched_pushToCpu(sched_AI_groupLeader(task), cpu);
}

cputime64_t AI_sched_getCurrentIdleTime(enum AI_CORE core) {
	//KERNEL_ERROR_MSG("[SCHED] AI_Governor: getCurrentIdleTime not tested!\n");
	if (!AI_sched_cpuManaged(core))
		return -EINVAL;

	return kcpustat_cpu(core).cpustat[CPUTIME_IDLE];
}
cputime64_t AI_sched_getCurrentWorkTime(enum AI_CORE core) {
	//KERNEL_ERROR_MSG("[SCHED] AI_Governor: getCurrentWorkTime not tested!\n");
	if (!AI_sched_coreManaged(core))
		return -EINVAL;

	return kcpustat_cpu(core).cpustat[CPUTIME_USER]
			+ kcpustat_cpu(core).cpustat[CPUTIME_SYSTEM];
}

unsigned long AI_sched_getCurrentNetworkRx(void) {
	//KERNEL_ERROR_MSG( "[SCHED] AI_Governor: getCurrentNetworkRx not tested!\n" );
	unsigned long retval = 0;
	struct net_device *dev;
	read_lock(&dev_base_lock);
	dev = first_net_device(&init_net);
	while (dev) {
		if (dev->type == HARDWARE_TYPE_ETHERNET || dev->type
				== HARDWARE_TYPE_WIFI)
			retval += dev->stats.rx_bytes;

		dev = next_net_device(dev);
	}
	read_unlock(&dev_base_lock);

	return retval;
}
unsigned long AI_sched_getCurrentNetworkTx(void) {
	//KERNEL_ERROR_MSG( "[SCHED] AI_Governor: getCurrentNetworkTx not tested!\n" );
	unsigned long retval = 0;
	struct net_device *dev;
	read_lock(&dev_base_lock);
	dev = first_net_device(&init_net);
	while (dev) {
		if (dev->type == HARDWARE_TYPE_ETHERNET || dev->type
				== HARDWARE_TYPE_WIFI)
			retval += dev->stats.tx_bytes;

		dev = next_net_device(dev);
	}
	read_unlock(&dev_base_lock);

	return retval;
}

uint8_t AI_shutdownCpu = CPU_NONE;


int AI_sched_disableCpu(enum AI_CPU cpu)
{
	int retval = -EINVAL;
#ifdef CPU_IS_BIG_LITTLE

	uint8_t i = 0;
	bool tmp_state = false;

	// we do not shutdown A7 as it is the default CPU
	if (cpu == LITTLE)
		return 0;
	if (cpu == BIG && !AI_sched_get_BIG_state()) {
		//KERNEL_DEBUG_MSG( "[SCHED] AI_Governor: A15 already down.\n");
		return retval;
	}

	//	if (!AI_sched_cpuManaged(cpu))
	//		return retval;

	//KERNEL_DEBUG_MSG( "[SCHED] AI_Governor: AI_Governor: shutting down %x\n", cpu);

	//cpu_hotplug_driver_lock();

	// we have to reduce the frequency to the minimum before turning off A15
	// otherwise frequency of CPU will not go down
	AI_sched_assignFrequency(AI_sched_get_BIG_min_freq());
	mutex_lock(&hotplugMutex);
	tmp_state = AI_sched_get_BIG_state();
	AI_sched_set_BIG_state(false);
	AI_sched_set_BIG_freq(0);
	#ifdef CONFIG_HOTPLUG_CPU
	// Dominik did some shutdown_counter thingy, what's that?
	// make absolutely sure that we access the CPUs only once
	if (cpu == BIG && tmp_state) {
		for (i = BC3; i > LC3; i--) {
			if (((cpu & (0x01 << i)) > 0) && cpu_online(i)) {
				//KERNEL_DEBUG_MSG( "[SCHED] AI_Governor: Try shutdown %d\n", i);
				AI_shutdownCpu |= (0x01 << i);
				retval += cpu_down(i);
				if (retval > 0) {
					KERNEL_ERROR_MSG( "[SCHED] AI_Governor: Failed shutting down"
							"core %u\n", i );
					return retval;
				}
			}
		}
	}
	#endif //CONFIG_HOTPLUG_CPU
	mutex_unlock(&hotplugMutex);
	//cpu_hotplug_driver_unlock();
#endif

	return retval;

}


int AI_sched_enableCpu(enum AI_CPU cpu)
{
	int retval = 0;
#ifdef CPU_IS_BIG_LITTLE
	uint8_t i = 0;

	if (cpu == BIG && AI_sched_get_BIG_state()) {
		//KERNEL_DEBUG_MSG( "[SCHED] AI_Governor: A15 already up.\n");
		return -EINVAL;
	}

	// A7 should be enabled by default
	if (cpu == LITTLE) {
		//KERNEL_DEBUG_MSG( "[SCHED] AI_Governor: A15 already up.\n");
		return 0;
	}

	//KERNEL_DEBUG_MSG( "[SCHED] AI_Governor: AI_Governor: bringing back up cpu %x\n", cpu);

	mutex_lock(&hotplugMutex);
	//write_lock(&hotplug_rwlock);
	//cpu_hotplug_driver_lock();
#ifdef CONFIG_HOTPLUG_CPU
	// make absolutely sure that we are accessing the CPUs only once
	if (cpu == BIG && !AI_sched_get_BIG_state()) {
		for (i = 4; i < 8; i++) {
			if (((cpu & (0x01 << i)) > 0) && !cpu_online(i)) {
				retval += cpu_up(i);
				if (retval > 0)
					KERNEL_ERROR_MSG( "[SCHED] AI_Governor: Failed starting"
							"core %u up\n", i);
			}
		}
	}
#endif //CONFIG_HOTPLUG_CPU
	AI_sched_set_BIG_state(true);
	AI_sched_set_BIG_freq(AI_sched_get_BIG_min_freq());
	//cpu_hotplug_driver_unlock();
	mutex_unlock(&hotplugMutex);
	//write_unlock(&hotplug_rwlock);
#endif
	return retval;
}

int AI_sched_coreManaged(enum AI_CORE core) {
	//TODO GET TO COMPILE
//	return (managedCores & (0x0F << (core / 4))) > 0;
	return 0;
}

int AI_sched_cpuManaged(enum AI_CPU cpu) {
//	return !((managedCores & cpu) ^ cpu);
	return 0;
}

void AI_sched_addCoresToManaged(uint8_t cpus_bitmask) {
//	managedCores |= cpus_bitmask;
}

void AI_sched_removeCoresFromManaged(uint8_t cpus_bitmask) {
//	managedCores &= ~(cpus_bitmask);
}

uint8_t AI_sched_getManagedCores(void) {
	return managedCores;
}

struct task_struct *sched_AI_groupLeader(struct task_struct *task) {
	return (task != NULL) ? task->group_leader : NULL;
}

static inline cputime64_t get_cpu_idle_time_jiffy(unsigned int cpu,
		cputime64_t *wall) {
	u64 idle_time;
	u64 cur_wall_time;

	cur_wall_time = jiffies64_to_cputime64(get_jiffies_64());

	idle_time = kcpustat_cpu(cpu).cpustat[CPUTIME_IDLE];
	if (wall)
		*wall = jiffies_to_usecs(cur_wall_time);

	return jiffies_to_usecs(idle_time);
}

inline cputime64_t AI_sched_get_cpu_idle_time(unsigned int cpu,
		cputime64_t *wall, bool io_is_busy) {
	u64 idle_time = get_cpu_idle_time_us(cpu, wall);

	if (idle_time == -1ULL)
		idle_time = get_cpu_idle_time_jiffy(cpu, wall);
	else if (!io_is_busy)
		idle_time += get_cpu_iowait_time_us(cpu, wall);

	return idle_time;
}

int AI_sched_update_load(int cpu, struct cpufreq_AI_governor_cpuinfo *pcpu) {
	struct cpufreq_AI_governor_tunables *tunables;
	u64 now = 0;
	u64 now_idle = 0;
	unsigned int delta_idle = 0;
	unsigned int delta_time = 0;
	u64 active_time = 0;

	//	KERNEL_DEBUG_MSG(" [GOVERNOR] AI_Governor: update load, cpu: %d \n",
	//			cpu);

	// check whether A15 is online - if not, do not access or update load
	#ifdef CPU_IS_BIG_LITTLE
	if (!AI_sched_get_BIG_state() && cpu > LC3) {
		return 0;
	}
	#endif

	if (pcpu != NULL) {
		if (&pcpu->policy == NULL) {
			KERNEL_ERROR_MSG(" [PM] AI_Governor: no pcpu->policy \n");
			return 0;
		} else {
			tunables = pcpu->policy->governor_data;
		}
	} else {
		KERNEL_ERROR_MSG(" [PM] AI_Governor: no pcpu \n");
		return 0;
	}

	//if (tunables != NULL) {
	//	if (&tunables->io_is_busy != NULL) {
	now_idle = AI_sched_get_cpu_idle_time(cpu, &now, false);
	//	} else {
	//		KERNEL_ERROR_MSG(" [PM] AI_Governor: tunables->io_is_busy is null, cpu %d\n", cpu);
	//	}
	//} else {
	//	KERNEL_ERROR_MSG(" [PM] AI_Governor: tunables is null, cpu %d\n", cpu);
	//}

	if (&pcpu->time_in_idle != NULL)
		delta_idle = (unsigned int) (now_idle - pcpu->time_in_idle);

	if (&pcpu->time_in_idle_timestamp != NULL)
		delta_time = (unsigned int) (now - pcpu->time_in_idle_timestamp);

	if (delta_time <= delta_idle)
		active_time = 0;
	else
		active_time = delta_time - delta_idle;

	//pcpu->cputime_speedadj += active_time * pcpu->policy->cur;

	update_cpu_metric(cpu, now, delta_idle, delta_time, pcpu->policy);

	//	if (cpu == A15C0)
	//		KERNEL_ERROR_MSG(" [PM] AI_Governor: cpu %d, cpu, now_idle %llu, pcpu->time_in_idle %llu, now %llu, pcpu->time_in_idle_timestamp %llu\n",
	//				cpu, now_idle, pcpu->time_in_idle, now, pcpu->time_in_idle_timestamp);

	if (&pcpu->time_in_idle != NULL)
		pcpu->time_in_idle = now_idle;

	if (&pcpu->time_in_idle_timestamp != NULL)
		pcpu->time_in_idle_timestamp = now;

	return cpu_load_metric_get_per_core(cpu);
}
