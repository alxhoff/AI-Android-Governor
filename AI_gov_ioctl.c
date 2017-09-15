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

long AI_gov_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	struct AI_gov_info g;

//	switch(cmd){
//	case GOVERNOR_GET_VARIABLES:
//		memcpy(&g, &AI_gov, sizeof(AI_gov_info));
//		if(copy_to_user((AI_gov_info *)arg, &g, sizeof(AI_gov_info)))
//				return -EACCES;
//		break;
//	case GOVERNOR_CLR_VARIABLES:{
//		//check against hardware min freq (TODO)
//		//default values (TODO)
//		AI_gov.profile.min_freq = 1000;
//		AI_gov.profile.max_freq = 1500;
//		AI_gov.profile.desired_frame_rate = 60;
//		AI_gov.profile.current_frame_rate = 0;
//		AI_gov.phase = 0;
//	}
//		break;
//	case GOVERNOR_SET_VARIABLES:{
//		if(copy_from_user(&g, (AI_gov_info *)arg, sizeof(AI_gov_info)))
//			return -EACCES;
//		AI_gov.profile.min_freq = g.profile.min_freq;
//		AI_gov.profile.max_freq = g.profile.max_freq;
//		AI_gov.profile.desired_frame_rate = g.profile.desired_frame_rate;
//		AI_gov.profile.current_frame_rate = g.profile.current_frame_rate;
//		AI_gov.phase = g.phase;
//	}
//		break;
//	case GOVERNOR_OTHER_FUNCT:{
//		AI_gov.profile.min_freq = 1;
//		AI_gov.profile.max_freq = 2;
//		AI_gov.profile.desired_frame_rate = 3;
//		AI_gov.profile.current_frame_rate = 4;
//		AI_gov.phase = 5;
//	}
//		break;
//	default:
//		return -EINVAL;
//		break;
//
//	}
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
	if((ret = alloc_chrdev_region(&dev, AI_baseminor, AI_minorCount, "AI_governor_ioctl")) < 0)
		KERNEL_ERROR_MSG(
						"[IOCTL] AI_Governor: Error allocating char device region. "
						"Aborting!\n");
		return ret;

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
