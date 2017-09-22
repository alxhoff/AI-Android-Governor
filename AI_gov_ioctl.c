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


static dev_t dev;
static struct cdev c_dev;
static struct class *cl;

const unsigned AI_baseminor = 0, AI_minorCount = 1;

static uint8_t initialized = 0;

int AI_gov_open(struct inode *i, struct file *f)
{
	return 0;
}

int AI_gov_close(struct inode *i, struct file *f)
{
	return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
int AI_gov_ioctl(struct inode *i, struct file *f, unsigned int cmd, unsigned long arg)
#else
long AI_gov_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
#endif
{

	switch(cmd){
	case GOVERNOR_GET_PROFILE:{
		struct AI_gov_profile g;
		memcpy(&g, AI_gov->profile, sizeof(struct AI_gov_profile));
		if(copy_to_user(
				(struct AI_gov_profile *)arg, &g, sizeof(struct AI_gov_profile)))
				return -EACCES;
	}
		break;
	case GOVERNOR_SET_PROFILE:{
		struct AI_gov_profile g;
		if(copy_from_user(&g, (struct AI_gov_profile*)arg, sizeof(struct AI_gov_profile)))
					return -EACCES;
		memcpy(AI_gov->profile, &g, sizeof(struct AI_gov_profile));
	}
		break;
	case GOVERNOR_CLR_PROFILE:{
		//TODO set to appropriate values, cpu freq table etc
		AI_gov->profile->current_frame_rate = 0;
		AI_gov->profile->desired_frame_rate = 0;
		AI_gov->profile->max_freq = 0;
		AI_gov->profile->min_freq = 0;
	}
		break;
	case GOVERNOR_SET_PHASE:{
		phase_state g;
		if(copy_from_user(&g, (phase_state*)arg, sizeof(phase_state)))
			return -EACCES;
		AI_gov->prev_phase = AI_gov->phase;
		AI_gov->phase = g;
	}
		break;
	case GOVERNOR_GET_PHASE:{
		phase_state g;
		g = AI_gov->phase;
		if(copy_to_user(
				(phase_state *)arg, &g, sizeof(phase_state)))
				return -EACCES;
	}
		break;
	default:
		return -EINVAL;
		break;

	}
	return 0;
}

struct file_operations AI_governor_fops =
{
	.owner = THIS_MODULE,
	.open = AI_gov_open,
	.release = AI_gov_close,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
	.ioctl = AI_gov_ioctl
#else
        .unlocked_ioctl = AI_gov_ioctl
#endif
};

//permissions change
char *device_node(struct device *dev, umode_t *mode)
{
	if(!mode)
		return NULL;
	*mode = 0666;
	return NULL;
}

int AI_gov_ioctl_init(void)
{
	int ret;
	struct device *dev_ret;

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

	//init cdev struct with file operations
	cdev_init(&c_dev, &AI_governor_fops);

	//add char device to system
	if((ret = cdev_add(&c_dev, dev, AI_minorCount)) < 0)
		KERNEL_ERROR_MSG(
						"[IOCTL] AI_Governor: Error adding char device. Aborting!\n");
		return ret;

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

	AI_phases_init();

	initialized = 1;

	KERNEL_VERBOSE_MSG("[IOCTL] AI_Governor: Char device initialized! \n");
	KERNEL_VERBOSE_MSG("[IOCTL] AI_Governor: Device name: %s\n",
			dev_name(dev_ret));

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
