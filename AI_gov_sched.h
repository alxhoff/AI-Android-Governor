/*
 * AI_gov_sched.h
 *
 *  Created on: Sep 5, 2017
 *      Author: alxhoff
 */

#ifndef AI_GOV_SCHED_H_
#define AI_GOV_SCHED_H_

#include "AI_gov.h"
#include <asm/cputime.h>

/**
* @enum
* @brief
*/
enum AI_CORE {
	LC0 = 0,
	LC1 = 1,
	LC2 = 2,
	LC3 = 3,
#ifdef CPU_IS_BIG_LITTLE
	BC0 = 4,
	BC1 = 5,
	BC2 = 6,
	BC3 = 7
#endif
};

/**
* @enum
* @brief
*/
enum AI_CPU{
	CPU_NONE 		= 0x00,
	LITTLE 			= 0x0F,
#ifdef CPU_IS_BIG_LITTLE
	BIG				= 0xF0,
	BIG_AND_LITTLE	= 0xFF
#endif
};

//PROTOS

/**
 * @file functionality for cpufreq operations (e.g. assigning frequency or
 *        setting affinity)
 **/

#define NUM_CORES 8

/**
* @enum
* @brief
*/
enum FREQUENCIES {
	LITTLE_MIN = 1000000,
	LITTLE_MAX = 1400000,
	BIG_MIN = 1200000,
	BIG_MAX = 2000000,
	LITTLE_10 = 1000000,
	LITTLE_11 = 1100000,
	LITTLE_12 = 1200000,
	LITTLE_13 = 1300000,
	LITTLE_14 = 1400000,
	BIG_12 = 1200000,
	BIG_13 = 1300000,
	BIG_14 = 1400000,
	BIG_15 = 1500000,
	BIG_16 = 1600000,
	BIG_17 = 1700000,
	BIG_18 = 1800000,
	BIG_19 = 1900000,
	BIG_20 = 2000000
};

/**
* @enum
* @brief
*/
enum {
	HARDWARE_TYPE_ETHERNET = 1, //< type id for ethernet net_devices
	HARDWARE_TYPE_WIFI = -1
//< type id for wifi net_devices (-1 => not known and/or irrelevant)
};

/**
 * This function updates the workload from the interactive governor.
 * This is a workaround as calculating the workload  ourselves caused errors.
 *
 * @param[in] workload of the core
 * @param[in] core number
 */
void AI_sched_update_workload_interactive(int workload, uint8_t core);

/**
 * This function calculates the workload and updated the workload
 * histories for all available cores.
 */
void AI_sched_update_workload(void);

/**
 * @param[in] cpu (BIG, LITTLE or LITTLE_BIG)
 * @param[in] length of the history to be checked
 * @return 1 if the workload of the cpu has been critical more than history_length times
 */
bool AI_sched_criticalWorkloadPhase(uint8_t history_length, enum AI_CPU cpu);

/**
 * @param[in] cpu (BIG, LITTLE or LITTLE_BIG)
 * @return current frame rate, mean of the last 4 values of cpu's workload history
 **/
unsigned int AI_sched_get_smoothed_workload(enum AI_CPU cpu);

/**
 * Updates workload history for AI and for the total system
 *  @param[in] total_workload total workload of cpu
 *  @param[in] critial_workload is any of the cpu's workload above a critical threshold
 *  @param[in] cpu either BIG or LITTLE
 */
void AI_sched_updateWorkLoadHistory(unsigned int total_workload,
		bool critial_workload, uint8_t cpu);

/**
 * @param[in] length of the history to be checked
 * @return 1 if workload below a certain margin
 */
bool AI_sched_workloadBelowLoadingMargin(uint8_t history_length);

/**
 * Returns BIG frequency.
 * Returns 0 if BIG off.
 **/
uint32_t AI_sched_get_BIG_freq(void);

/**
 * Returns LITTLE frequency.
 **/
uint32_t AI_sched_get_LITTLE_freq(void);

/**
 * Returns BIG state.
 *
 * @return true if BIG is on and false if it is off
 **/
bool AI_sched_get_BIG_state(void);

/**
 * Assigns frequency to given core
 *
 * Aborts if core is not managed (as indicated by managedCores)
 * @param[in] target frequency in kHz
 * @param[in] target cpu
 * @return retval according to cpufreq_driver_target
 **/
int AI_sched_assignFrequency(unsigned int frequency);

/**
 * Sets the affinity of a given task to a given core
 *
 * @param[in] task to manipulate
 * @param[in] target core
 * @return retval according to sched_setaffinity
 **/
int AI_sched_setAffinity(struct task_struct *task, enum AI_CORE core);
/**
 * Sets the affinity of a given task to all cores belonging to given cpu
 *
 * @param given task
 * @param given cpu
 * @return retval according to sched_setaffinity
 **/
int AI_sched_pushToCpu(struct task_struct *task, enum AI_CPU cpu);
/**
 * pushes all group members task belongs to to specified CPU
 *
 * If task == NULL, current is assumed
 * @param task specifying the group (does NOT need to be the group leader)
 *
 *  May be NULL to use the current active task
 * @param CPU to push to
 * @return retval according to sched_setaffinity
 **/
int AI_sched_pushGroupToCpu(struct task_struct *task, enum AI_CPU cpu);

/**
 * Gets the current idle time of specified cpu
 *
 * @param core core of interest
 * @return current idle count, -EINVAL if core invalid
 **/
cputime64_t AI_sched_getCurrentIdleTime(enum AI_CORE core);
/**
 * Gets the current working time of specified cpu
 *
 * @param core core of interest
 * @return working time, -EINVAL if core invalid
 **/
cputime64_t AI_sched_getCurrentWorkTime(enum AI_CORE core);

unsigned long AI_sched_getCurrentNetworkRx(void);
unsigned long AI_sched_getCurrentNetworkTx(void);

/**
 * Disables a given CPU
 * We assume that the only CPU that can be disabled and enabled is BIG.
 *
 * @param cpu to disable
 * @return retval according to cpu_down
 **/
int AI_sched_disableCpu(enum AI_CPU cpu);
/**
 * Enables a given CPU.
 * We assume that the only CPU that can be disabled and enabled is BIG.
 *
 * @param cpu to enable
 * @return retval according to cpu_up
 **/
int AI_sched_enableCpu(enum AI_CPU cpu);

/**
 * Checks wether a given core is managed or not
 *
 * @param core to check
 * @return 1 if core is managed, 0 otherwise
 **/
int AI_sched_coreManaged(enum AI_CORE core);

/**
 * Checks wether a given cpu is managed or not
 *
 * @param cpu to check
 * @return 1 if core is managed, 0 otherwise
 **/
int AI_sched_cpuManaged(enum AI_CPU cpu);

/**
 * Gets the group leader to given task
 *
 * @param given task
 * @return group leader, NULL if task == NULL
 **/
struct task_struct *sched_AI_groupLeader(struct task_struct *task);

/**
 * adds cores to ManagedCores
 *
 * @param cpus_bitmask for BIG and LITTLE (4 bits each)
 **/
void AI_sched_addCoresToManaged(uint8_t cpus_bitmask);

/**
 * removes cores from ManagedCores
 *
 * @param cpus_bitmask for BIG and LITTLE (4 bits each)
 **/
void AI_sched_removeCoresFromManaged(uint8_t cpus_bitmask);

/**
 * Returns ManagedCores
 *
 * @return Bitmask of managed cores
 **/
uint8_t AI_sched_getManagedCores(void);

/**
 * Calculate the load of the given cpu core.
 * Copied from cpufreq_interactive.c
 *
 * @param cpu for which the load shall be calculated
 * @param cpuinfo from main governor file
 * @return CPU load, 0 if BIG not available
 **/
int AI_sched_update_load(int cpu, struct cpufreq_AI_governor_cpuinfo *pcpu);

/**
 * Calculate the idle time of the given cpu core.
 * Copied from cpufreq_interactive.c
 *
 * @param cpu for which the load shall be calculated
 * @param wall pointer to current time stamp
 * @param io_is_busy global tunables io_is_busy variable
 * @return idle time of current thread
 * @return current time stamp in *wall
 **/
inline cputime64_t AI_sched_get_cpu_idle_time(unsigned int cpu,
		cputime64_t *wall, bool io_is_busy);

#endif /* AI_GOV_SCHED_H_ */
