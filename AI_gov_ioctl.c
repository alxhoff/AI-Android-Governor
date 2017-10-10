/*
 * AI_gov_ioctl.c
 *
 *  Created on: Aug 31, 2017
 *      Author: alxhoff
 */

#include <linux/fs.h>
#include <linux/cpumask.h>
#include <linux/kernel.h>

#include "AI_gov_ioctl.h"
#include "test_flags.h"
#include "AI_gov_kernel_write.h"
#include "AI_gov_types.h"
#include "AI_gov_phases.h"
#include "AI_gov_sysfs.h"

signed int AI_gov_ioctl_set_variable(struct AI_gov_ioctl_phase_variable var);
unsigned int AI_gov_ioctl_get_variable(struct AI_gov_ioctl_phase_variable* var);
signed int AI_gov_ioctl_clear_phase();
char *device_node(struct device *dev, umode_t *mode);

static dev_t dev;
static struct cdev c_dev;
static struct class *cl;

const unsigned AI_baseminor = 0, AI_minorCount = 1;

static uint8_t initialized = 0;

struct file_operations AI_governor_fops =
{
	.owner = THIS_MODULE,
	.open = AI_gov_open,
	.release = AI_gov_close,
//#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
//	.ioctl = AI_gov_ioctl
//#else
        .unlocked_ioctl = AI_gov_ioctl
//#endif
};

//#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
//int AI_gov_ioctl(struct inode *i, struct file *f, unsigned int cmd, unsigned long arg)
//#else
long AI_gov_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
//#endif
{
	switch(cmd){
	case GOVERNOR_GET_PHASE:{
		unsigned long g;
		g = AI_gov->phase;
		if(copy_to_user((unsigned long *)arg, &g, sizeof(unsigned long)))
			return -EACCES;
	}
		break;
	case GOVERNOR_SET_PHASE:{
		unsigned long g;
		enum PHASE_ENUM phase;
		if(copy_from_user(&g, (unsigned long*)arg, sizeof(unsigned long)))
			return -EACCES;
		phase = (enum PHASE_ENUM)g;
//		AI_gov->phase = phase;
		AI_gov_sysfs_load_profile(phase);
	}
		break;
	case GOVERNOR_CLR_PHASE_VARIABLES:{
		AI_gov_ioctl_clear_phase();
	}
		break;
	case GOVERNOR_SET_PHASE_VARIABLE:{
		struct AI_gov_ioctl_phase_variable g;
		if(copy_from_user(&g, (struct AI_gov_ioctl_phase_variable*)arg,
				sizeof(struct AI_gov_ioctl_phase_variable)))
			return -EACCES;
		AI_gov_ioctl_set_variable(g);
	}
		break;
	case GOVERNOR_GET_PHASE_VARIABLE:{
		struct AI_gov_ioctl_phase_variable g;
		if(copy_from_user(&g, (struct AI_gov_ioctl_phase_variable*)arg,
				sizeof(struct AI_gov_ioctl_phase_variable)))
			return -EACCES;
		AI_gov_ioctl_get_variable(&g);
		if(copy_to_user((struct AI_gov_ioctl_phase_variable *)arg,
				&g, sizeof(struct AI_gov_ioctl_phase_variable)))
			return -EACCES;
	}
		break;
	default:
		return -EINVAL;
		break;
	}
	return 0;
}

int AI_gov_ioctl_init(void)
{
	int ret = 0;
	struct device *dev_ret;

	KERNEL_DEBUG_MSG(
			"[GOVERNOR] IOctl STARTING INIT \n");

	if (initialized > 0) {
		return ret;
	}

	//register char dev numbers, call with zero to get dynamic number allocated
	if((ret = alloc_chrdev_region(&dev, AI_baseminor, AI_minorCount, "AI_governor_ioctl")) < 0){
		KERNEL_ERROR_MSG(
				"[IOCTL] AI_Governor: Error allocating char device region. "
				"Aborting!\n");
		return ret;
	}

	KERNEL_DEBUG_MSG(
				"[GOVERNOR] IOctl char dev allocated \n");

	//init cdev struct with file operations
	cdev_init(&c_dev, &AI_governor_fops);

	KERNEL_DEBUG_MSG(
				"[GOVERNOR] IOctl char dev init'd with file ops \n");

	//add char device to system
	if((ret = cdev_add(&c_dev, dev, AI_minorCount)) < 0){
		KERNEL_ERROR_MSG(
				"[IOCTL] AI_Governor: Error adding char device. Aborting!\n");
		return ret;
	}

	KERNEL_DEBUG_MSG(
					"[GOVERNOR] IOctl char dev added \n");

	if (IS_ERR(cl = class_create(THIS_MODULE, "AI_governor_ioctl")))
	{
		cdev_del(&c_dev);
		unregister_chrdev_region(dev, AI_minorCount);
		KERNEL_ERROR_MSG(
				"[IOCTL] AI_Governor: Error initializing char device."
				" Aborting!\n");

		return PTR_ERR(cl);
	}

	//permissions again
	cl->devnode = device_node;

	KERNEL_DEBUG_MSG(
				"[GOVERNOR] IOctl class permissions set \n");

	if (IS_ERR(dev_ret = device_create(cl, NULL, dev, NULL, "AI_governor_ioctl")))
	{
		class_destroy(cl);
		cdev_del(&c_dev);
		unregister_chrdev_region(dev, AI_minorCount);
		KERNEL_ERROR_MSG(
						"[IOCTL] AI_Governor: Error initializing char device."
						" Aborting!\n");

		return PTR_ERR(dev_ret);
	}

	KERNEL_DEBUG_MSG(
				"[GOVERNOR] IOctl dev device created \n");

	initialized = 1;

	KERNEL_VERBOSE_MSG("[IOCTL] AI_Governor: Char device initialized! \n");
	KERNEL_VERBOSE_MSG("[IOCTL] AI_Governor: Device name: %s\n",
			dev_name(dev_ret));

	KERNEL_DEBUG_MSG(
				"[GOVERNOR] IOctl FINISHED INIT \n");

	return 0;
}

int AI_gov_ioctl_exit(void)
{
	initialized = 0;
	device_destroy(cl, dev);
	class_destroy(cl);
cdev_del(&c_dev);
	unregister_chrdev_region(dev, AI_minorCount);

	KERNEL_ERROR_MSG("[IOCTL] AI_Governor: IOCTL closed\n");
	return 0;
}

int AI_gov_open(struct inode *i, struct file *f)
{
	return 0;
}

int AI_gov_close(struct inode *i, struct file *f)
{
	return 0;
}

unsigned int AI_gov_ioctl_get_variable(struct AI_gov_ioctl_phase_variable* var)
{
	var->phase = AI_gov->phase;

	switch(AI_gov->phase){
		case AI_init:
			switch(var->variable_index){
			case 0:
				var->variable_value =  GET_ATTRIBUTES(AI_init)->initialized;
				break;
			default:
				break;
			}
			break;
		case AI_framerate:
			switch(var->variable_index){
			case 0:
				var->variable_value = GET_ATTRIBUTES(AI_framerate)->desired_framerate;
				break;
			case 1:
				var->variable_value = GET_ATTRIBUTES(AI_framerate)->current_frametate;
				break;
			default:
				break;
			}
			break;
		case AI_priority:
			switch(var->variable_index){
			case 0:
				var->variable_value = GET_ATTRIBUTES(AI_priority)->priority_scalar;
				break;
			case 1:
				var->variable_value = GET_ATTRIBUTES(AI_priority)->minimum_priority;
				break;
			case 2:
				var->variable_value = GET_ATTRIBUTES(AI_priority)->maximum_priority;
				break;
			}
			break;
		case AI_time:
			switch(var->variable_index){
			case 0:
				var->variable_value = GET_ATTRIBUTES(AI_time)->time_till_completion;
				break;
			case 1:
				var->variable_value = GET_ATTRIBUTES(AI_time)->time_at_completion;
				break;
			case 2:
				var->variable_value = GET_ATTRIBUTES(AI_time)->alarm_mode;
				break;
			}
			break;
		case AI_powersave:
			switch(var->variable_index){
			case 0:
				var->variable_value = GET_ATTRIBUTES(AI_powersave)->initialized;
				break;
			}
			break;
		case AI_performance:
			switch(var->variable_index){
			case 0:
				var->variable_value = GET_ATTRIBUTES(AI_performance)->initialized;
				break;
			}
			break;
		case AI_response:
			switch(var->variable_index){
			case 0:
				var->variable_value = GET_ATTRIBUTES(AI_response)->user_input_importance;
				break;
			}
			break;
		case AI_exit:
			switch(var->variable_index){
			case 0:
				var->variable_value = GET_ATTRIBUTES(AI_exit)->deinitialized;
				break;
			}
			break;
		default:
			break;
		}

	return 0;
}

signed int AI_gov_ioctl_set_variable(struct AI_gov_ioctl_phase_variable var)
{
	var.phase = AI_gov->phase;

	switch(AI_gov->phase){
		case AI_init:
			switch(var.variable_index){
			case 0:
				GET_ATTRIBUTES(AI_init)->initialized = var.variable_value;
				break;
			default:
				break;
			}
			break;
		case AI_framerate:
			switch(var.variable_index){
			case 0:
				GET_ATTRIBUTES(AI_framerate)->desired_framerate = var.variable_value;
				break;
			case 1:
				GET_ATTRIBUTES(AI_framerate)->current_frametate = var.variable_value;
				break;
			default:
				break;
			}
			break;
		case AI_priority:
			switch(var.variable_index){
			case 0:
				GET_ATTRIBUTES(AI_priority)->priority_scalar = var.variable_value;
				break;
			case 1:
				GET_ATTRIBUTES(AI_priority)->minimum_priority = var.variable_value;
				break;
			case 2:
				GET_ATTRIBUTES(AI_priority)->maximum_priority = var.variable_value;
				break;
			}
			break;
		case AI_time:
			switch(var.variable_index){
			case 0:
				GET_ATTRIBUTES(AI_time)->time_till_completion = var.variable_value;
				break;
			case 1:
				GET_ATTRIBUTES(AI_time)->time_at_completion = var.variable_value;
				break;
			case 2:
				GET_ATTRIBUTES(AI_time)->alarm_mode = var.variable_value;
				break;
			}
			break;
		case AI_powersave:
			switch(var.variable_index){
			case 0:
				GET_ATTRIBUTES(AI_powersave)->initialized = var.variable_value;
				break;
			}
			break;
		case AI_performance:
			switch(var.variable_index){
			case 0:
				GET_ATTRIBUTES(AI_performance)->initialized = var.variable_value;
				break;
			}
			break;
		case AI_response:
			switch(var.variable_index){
			case 0:
				GET_ATTRIBUTES(AI_response)->user_input_importance = var.variable_value;
				break;
			}
			break;
		case AI_exit:
			switch(var.variable_index){
			case 0:
				GET_ATTRIBUTES(AI_exit)->deinitialized = var.variable_value;
				break;
			}
			break;
		default:
			break;
		}

	return 0;
}

signed int AI_gov_ioctl_clear_phase(void)
{
	switch(AI_gov->phase){
	case AI_init:
		GET_ATTRIBUTES(AI_init)->initialized = 0;
		break;
	case AI_framerate:
		GET_ATTRIBUTES(AI_framerate)->current_frametate = 0;
		GET_ATTRIBUTES(AI_framerate)->desired_framerate = 0;
		//TODO Dynamic free
		GET_ATTRIBUTES(AI_framerate)->timestamp_history = NULL;
		break;
	case AI_priority:
		GET_ATTRIBUTES(AI_priority)->maximum_priority = 0;
		GET_ATTRIBUTES(AI_priority)->minimum_priority = 0;
		GET_ATTRIBUTES(AI_priority)->priority_scalar = 0;
		break;
	case AI_time:
		GET_ATTRIBUTES(AI_time)->alarm_mode = 0;
		GET_ATTRIBUTES(AI_time)->time_at_completion = 0;
		GET_ATTRIBUTES(AI_time)->time_till_completion = 0;
		break;
	case AI_powersave:
		GET_ATTRIBUTES(AI_powersave)->initialized = 0;
		break;
	case AI_performance:
		GET_ATTRIBUTES(AI_performance)->initialized = 0;
		break;
	case AI_response:
		GET_ATTRIBUTES(AI_response)->user_input_importance = 0;
		break;
	case AI_exit:
		GET_ATTRIBUTES(AI_exit)->deinitialized = 0;
		break;
	default:
		break;
	}

	return 0;
}


//permissions change
char *device_node(struct device *dev, umode_t *mode)
{
	if(!mode)
		return NULL;
	*mode = 0666;
	return NULL;
}

