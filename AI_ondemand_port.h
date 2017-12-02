/*
 * AI_ondemand_port.h
 *
 *  Created on: Dec 2, 2017
 *      Author: alxhoff
 */

#ifndef AI_ONDEMAND_PORT_H_
#define AI_ONDEMAND_PORT_H_

static void OD_OD_ondemand_powersave_bias_init_cpu(int cpu);
static int OD_should_io_be_busy(void);
static unsigned int OD_generic_powersave_bias_target(struct cpufreq_policy *policy,
		unsigned int freq_next, unsigned int relation);
static void OD_ondemand_powersave_bias_init(void);
static void OD_dbs_freq_increase(struct cpufreq_policy *p, unsigned int freq);
static void OD_od_check_cpu(int cpu, unsigned int load_freq);
static void OD_od_dbs_timer(struct work_struct *work);
static void OD_update_sampling_rate(struct dbs_data *dbs_data,
		unsigned int new_rate);
static int OD_od_init(struct dbs_data *dbs_data);
static void OD_od_exit(struct dbs_data *dbs_data);
static void OD_od_set_powersave_bias(unsigned int powersave_bias);
void OD_od_register_powersave_bias_handler(unsigned int (*f)
		(struct cpufreq_policy *, unsigned int, unsigned int),
		unsigned int powersave_bias);
void OD_od_unregister_powersave_bias_handler(void);
static int OD_od_cpufreq_governor_dbs(struct cpufreq_policy *policy,
		unsigned int event);

#endif /* AI_ONDEMAND_PORT_H_ */
