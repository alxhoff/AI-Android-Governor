/*
 * AI_gov_sysfs.h
 *
 *  Created on: Sep 20, 2017
 *      Author: alxhoff
 */

#ifndef AI_GOV_SYSFS_H_
#define AI_GOV_SYSFS_H_

#include <linux/kobject.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/cpufreq.h>

#include "AI_gov_types.h"
#include "AI_gov.h"

extern const char *AI_governor_sysfs[];

signed int AI_gov_sysfs_init_profiles(void);
signed int AI_gov_sysfs_init(void);
struct attribute_group* AI_get_sysfs_attr(void);
struct kobject *AI_get_governor_parent_kobj(struct cpufreq_policy *policy);
signed int AI_gov_sysfs_load_profile(enum PHASE_ENUM new_phase);

#endif /* AI_GOV_SYSFS_H_ */
