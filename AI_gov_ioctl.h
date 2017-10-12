/**
 * @file AI_gov_ioctl.h
 * @author Alex Hoffman
 * @date 11 October 2017
 * @brief IOctl interface for the AI_governor.
 *
 * Defines the functionality of the AI governor's IOctl interface
 *
 * @section IOctl IO Control
 * 
 */

#ifndef AI_GOV_H
#define AI_GOV_H

/* -- Includes -- */
/* Kernel includes. */
#include <linux/ioctl.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>

/* Governor includes. */
#include "AI_gov_phases.h"
#include "AI_gov.h"

/**
* @def Defaults for char dev minor number
*/
#define FIRST_MINOR     0
/**
* @def Defaults for char dev minor number allocation count
*/
#define MINOR_CNT       1

/**
 * @struct AI_gov_ioctl_phase_variable
 * @brief Used to interact with phase profile attributes through IOctl
 */
struct AI_gov_ioctl_phase_variable{
	enum PHASE_ENUM phase;

	uint8_t variable_index;

	unsigned long variable_value;
};

extern struct file_operations AI_governor_fops;

#define GOVERNOR_GET_PHASE _IOR('g', 1, unsigned long)
#define GOVERNOR_SET_PHASE _IOW('g', 2, unsigned long)
#define GOVERNOR_CLR_PHASE_VARIABLES _IO('g', 3)
#define GOVERNOR_GET_PHASE_VARIABLE _IOWR('g', 4, struct AI_gov_ioctl_phase_variable*)
#define GOVERNOR_SET_PHASE_VARIABLE _IOW('g', 5, struct AI_gov_ioctl_phase_variable*)

/**
* @brief Sets a variable specified by a AI_gov_ioctl_phase_variable struct
*
* @param var Specifies the index value of the variable that is required to be
* set, the value to be set is taken from var's variable value field
* @return On sucess returns 0
*/
signed int AI_gov_ioctl_set_variable(struct AI_gov_ioctl_phase_variable var);

/**
* @brief Retrieves a variable specified by a AI_gov_ioctl_phase_variable struct
*
* @param var A pointer to a AI_gov_ioctl_phase_variable struct whos index value 
* specifies the variable index of the variable that is required to be
* retrieved and loaded into the var struct's variable value.
* @return On sucess returns 0
*/
unsigned int AI_gov_ioctl_get_variable(struct AI_gov_ioctl_phase_variable* var);

/**
* @brief Clears the profile attributes for the current phase
*
* @return On sucess returns 0
*/
signed int AI_gov_ioctl_clear_phase(void);

/**
* @brief Overwrites a class' permissions to 0666
*
* This function is used to overwrite the default permissions
* function for a class.
*
* @param dev Device whose permissions are to be changed
* @param mode Permissions value to be set
* @return NULL
*/
char *device_node(struct device *dev, umode_t *mode);

/**
* @brief Required by IOctl fops
*
* @return On sucess returns 0
*/
int AI_gov_open(struct inode *i, struct file *f);

/**
* @brief Required by IOctl fops
*
* @return On sucess returns 0
*/
int AI_gov_close(struct inode *i, struct file *f);

/**
* @brief Governor's IOcontrol callable commands
*
* Contains a switch statement that handles all of the IOctl callable 
* commands from userspace
*
* @param f
* @param cmd
* @param arg
* @return On sucess returns 0
*/
long AI_gov_ioctl(struct file *f, unsigned int cmd, unsigned long arg);

/**
* @brief Governor's IOcontrol initilisation function
*
* Initialises the character device and class required by IOctl. Character
* device will be mounted as '/dev/AI_governor_ioctl'
*
* @return On sucess returns 0
*/
unsigned int AI_gov_ioctl_init(void);

/**
* @brief Governor's IOcontrol deinitialization function
*
* Called when IOctl is deinitialized as a result of the governor being
* unloaded from the kernel.
*
* @return On sucess returns 0
*/
int AI_gov_ioctl_exit(void);

#endif
