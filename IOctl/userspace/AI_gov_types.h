/*
 * AI_gov_types.h
 *
 *  Created on: Sep 15, 2017
 *      Author: alxhoff
 */

#ifndef AI_GOV_TYPES_H_
#define AI_GOV_TYPES_H_

#include "AI_gov_phases.h"

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

	struct kobject*		kobj;

	void* stats;
};


struct AI_gov_profile{
	unsigned long min_freq;
	unsigned long max_freq;

	struct kobject*		kobj;

	unsigned int desired_frame_rate;
	unsigned int current_frame_rate;
};

struct AI_gov_info{
	struct AI_gov_cur_HW* hardware;

	struct AI_gov_profile* profile;

	phase_state phase;
	phase_state prev_phase;

	int test;

	struct kobject*		kobj;
	struct completion*	kobj_unregister;
};

struct cpufreq_AI_governor_cpuinfo {
	struct timer_list cpu_timer;
	struct timer_list cpu_slack_timer;
	spinlock_t load_lock; /* protects the next 4 fields */
	u64 time_in_idle;
	u64 time_in_idle_timestamp;
	u64 cputime_speedadj;
	u64 cputime_speedadj_timestamp;
	struct cpufreq_policy *policy;
	struct cpufreq_frequency_table *freq_table;
	unsigned int target_freq;
	unsigned int floor_freq;
	u64 floor_validate_time;
	u64 hispeed_validate_time;
	struct rw_semaphore enable_sem;
	int governor_enabled;

};

struct cpufreq_AI_governor_tunables {
	int usage_count;
	spinlock_t target_loads_lock;
	unsigned int *target_loads;
	int ntarget_loads;
	/*
	 * The minimum amount of time to spend at a frequency before we can ramp
	 * down.
	 */
#define DEFAULT_MIN_SAMPLE_TIME (80 * USEC_PER_MSEC)
	unsigned long min_sample_time;
	/*
	 * The sample rate of the timer used to increase frequency
	 */
	unsigned long timer_rate;
	bool io_is_busy;

#define PHASE_NAME_LEN 15
	const char * phase_state;

#define TASK_NAME_LEN 15
	/* realtime thread handles frequency scaling */
	struct task_struct *speedchange_task;

	/* handle for get cpufreq_policy */
	unsigned int *policy;
};

#endif /* AI_GOV_TYPES_H_ */
