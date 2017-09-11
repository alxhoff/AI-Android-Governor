/*
 * cpufreq_alextest.h
 *
 *  Created on: Aug 31, 2017
 *      Author: alxhoff
 */

#ifndef CPUFREQ_ALEXTEST_H_
#define CPUFREQ_ALEXTEST_H_

#include <linux/timer.h>
#include <linux/rwsem.h>

#include "AI_gov_hardware.h"
#include "AI_gov_phases.h"

#define TASK_NAME_LEN 15

//#define CPU_IS_BIG_LITTLE	1


extern struct AI_gov_info AI_gov;

struct AI_gov_profile{
	unsigned int min_freq;
	unsigned int max_freq;

	unsigned int desired_frame_rate;
	float current_frame_rate;
};

struct AI_gov_info{
	//AI
	struct AI_gov_cur_HW hardware;

	struct AI_gov_profile profile;
	phase_state phase;
	phase_state prev_phase;
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
	/* Hi speed to bump to from lo speed when load burst (default max) */
	unsigned int hispeed_freq;
	/* Go to hi speed when CPU load at or above this value. */
#define DEFAULT_GO_HISPEED_LOAD 99
	unsigned long go_hispeed_load;
	/* Target load. Lower values result in higher CPU speeds. */
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
	/*
	 * Wait this long before raising speed above hispeed, by default a
	 * single timer interval.
	 */
	spinlock_t above_hispeed_delay_lock;
	unsigned int *above_hispeed_delay;
	int nabove_hispeed_delay;
	/* Non-zero means indefinite speed boost active */
	int boost_val;
	/* Duration of a boot pulse in usecs */
	int boostpulse_duration_val;
	/* End time of boost pulse in ktime converted to usecs */
	u64 boostpulse_endtime;
	/*
	 * Max additional time to wait in idle, beyond timer_rate, at speeds
	 * above minimum before wakeup to reduce speed, or -1 if unnecessary.
	 */
#define DEFAULT_TIMER_SLACK (4 * DEFAULT_TIMER_RATE)
	int timer_slack_val;
	bool io_is_busy;

#define PHASE_NAME_LEN 15
	const char * phase_state;

#define TASK_NAME_LEN 15
	/* realtime thread handles frequency scaling */
	struct task_struct *speedchange_task;

	/* handle for get cpufreq_policy */
	unsigned int *policy;
};

#ifndef FAST_RESCHEDULE
#define FAST_RESCHEDULE (2 * USEC_PER_MSEC)
#endif

void AI_phase_change(void);
//static int AI_touch_nb_callback(void);

void cpufreq_AI_governor_timer_resched(unsigned long expires);


#endif /* CPUFREQ_ALEXTEST_H_ */
