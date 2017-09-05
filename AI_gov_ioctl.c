/*
 * AI_gov_ioctl.c
 *
 *  Created on: Aug 31, 2017
 *      Author: alxhoff
 */


#include "AI_gov_ioctl.h"

static dev_t dev;
static struct cdev c_dev;
static struct class *cl;

int AI_governor_open(struct inode *i, struct file *f)
{
	return 0;
}

int AI_governor_close(struct inode *i, struct file *f)
{
	return 0;
}

long AI_governor_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	AI_governor g;

	switch(cmd){
	case GOVERNOR_GET_VARIABLES:
		memcpy(&g, &AI_gov, sizeof(AI_governor));
		if(copy_to_user((AI_governor *)arg, &g, sizeof(AI_governor)))
				return -EACCES;
		break;
	case GOVERNOR_CLR_VARIABLES:
		//check against hardware min freq (TODO)
		//default values (TODO)
		AI_gov.profile.min_freq = 1000;
		AI_gov.profile.max_freq = 1500;
		AI_gov.profile.desired_frame_rate = 60;
		AI_gov.profile.current_frame_rate = 0;
		AI_gov.phase = 0;
		break;
	case GOVERNOR_SET_VARIABLES:
		if(copy_from_user(&g, (AI_governor *)arg, sizeof(AI_governor)))
			return -EACCES;
		AI_gov.profile.min_freq = g.profile.min_freq;
		AI_gov.profile.max_freq = g.profile.max_freq;
		AI_gov.profile.desired_frame_rate = g.profile.desired_frame_rate;
		AI_gov.profile.current_frame_rate = g.profile.current_frame_rate;
		AI_gov.phase = g.phase;
		break;
	case GOVERNOR_OTHER_FUNCT:
		AI_gov.profile.min_freq = 1;
		AI_gov.profile.max_freq = 2;
		AI_gov.profile.desired_frame_rate = 3;
		AI_gov.profile.current_frame_rate = 4;
		AI_gov.phase = 5;
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

int AI_governor_ioctl_init(void)
{
	int ret;
	struct device *dev_ret;

	//register char dev numbers, call with zero to get dynamic number allocated
	if((ret = alloc_chrdev_region(&dev, FIRST_MINOR, MINOR_CNT, "AI_governor_ioctl")))
		return ret;

	//init cdev struct with file operations
	cdev_init(&c_dev, &AI_governor_fops);

	//add char device to system
	if((ret = cdev_add(&c_dev, dev, MINOR_CNT)) < 0)
		return ret;

	if (IS_ERR(cl = class_create(THIS_MODULE, "char")))
	{
		cdev_del(&c_dev);
		unregister_chrdev_region(dev, MINOR_CNT);
		return PTR_ERR(cl);
	}

	//permissions again
	cl->devnode = device_node;

	if (IS_ERR(dev_ret = device_create(cl, NULL, dev, NULL, "AIgov")))
	{
		class_destroy(cl);
		cdev_del(&c_dev);
		unregister_chrdev_region(dev, MINOR_CNT);
		return PTR_ERR(dev_ret);
	}
	return 0;
}

void AI_governor_ioctl_exit(void)
{
	device_destroy(cl, dev);
	class_destroy(cl);
	cdev_del(&c_dev);
	unregister_chrdev_region(dev, MINOR_CNT);
}
