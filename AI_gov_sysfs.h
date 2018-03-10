/**
 * @file AI_gov_sysfs.h
 * @author Alex Hoffman
 * @date 11 October 2017
 * @brief sysfs interface for the AI_governor.
 * @section About
 *
 * Defines the functionality of the AI governor's stsfs interface
 */

#ifndef AI_GOV_SYSFS_H_
#define AI_GOV_SYSFS_H_

/* -- Includes -- */
/* Kernel includes. */
#include <linux/kobject.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/cpufreq.h>

/* Governor includes. */
#include "AI_gov_types.h"
#include "AI_gov.h"

#define GET_CURRENT_PROFILE \
		AI_phases_get_name(PHASE_STRINGS[AI_gov->phase])

/**
*
*/
extern const char *AI_governor_sysfs[];

/**
* @brief Initialises the sysfs profiles
*
* By calling the macro ATTACH_SYSFS_GROUPS which in turn
* calls ATTACH_SINGLE_SYSFS_GROUP for each phase. This retrieves
* the profile to be initialised into the sysfs heirarchy, creating
* and adding the kobject and sysfs attribute group. Before returning
* the function removed the kobject from the file heirarchy, this is
* because only one "profile" kobject can be registered with the sysfs
* heirarchy at one time.
*
* @return 0 on success
*/
signed int AI_gov_sysfs_init_profiles(void);

/**
* @brief Initialises the sysfs subsystem
*
* Creates and mounts the kobjects and sysfs attribute groups for the
* top level governor properties as well as the hardware properties.
* Before returning it mounts the current profile's kobject. As such
* the sys prfiles must be initialised with AI_gov_sysfs_init_profiles
* before calling AI_gov_sysfs_init
*
* @return 0 on success
*/
signed int AI_gov_sysfs_init(void);

/**
* @brief Gets the attribute group for the highest level of the AI
* governor
*
* @return Pointer to the attribute group.
*/
struct attribute_group* AI_get_sysfs_attr(void);

/**
* @brief Gets the parent kobject of the AI governor
*
* @return Pointer to the parent kobject of the AI governor
*/
struct kobject *AI_get_governor_parent_kobj(struct cpufreq_policy *policy);

#endif /* AI_GOV_SYSFS_H_ */
