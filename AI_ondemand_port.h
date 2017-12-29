/*
 * AI_ondemand_port.h
 *
 *  Created on: Dec 2, 2017
 *      Author: alxhoff
 */

#ifndef _AI_ONDEMAND_PORT_H_
#define _AI_ONDEMAND_PORT_H_

#include "cpufreq_governor.h"

extern struct cpufreq_governor cpufreq_gov_ondemand;

int od_cpufreq_governor_dbs(struct cpufreq_policy *policy,
		unsigned int event);

int od_cpufreq_gov_dbs_init(void);
void od_cpufreq_gov_dbs_exit(void);

int od_init(struct dbs_data *dbs_data);
void od_exit(struct dbs_data *dbs_data);


#endif /* _AI_ONDEMAND_PORT_H_ */
