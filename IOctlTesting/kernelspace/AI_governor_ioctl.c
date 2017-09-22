#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <asm/uaccess.h>

#include "AI_governor_ioctl.h"

#define FIRST_MINOR 	0
#define MINOR_CNT	1

static dev_t dev;
static struct cdev c_dev;
static struct class *cl;
static AI_governor current_AI_governor = {0};

static int AI_gov_open(struct inode *i, struct file *f){
	return 0;
}

static int AI_gov_close(struct inode *i, struct file *f){
	return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
static int AI_gov_ioctl(struct inode *i, struct file *f, unsigned int cmd, unsigned long arg)
#else
static long AI_gov_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
#endif
{
	AI_governor g;

	switch(cmd){
	case GOVERNOR_GET_VARIABLES:
		memcpy(&g, &current_AI_governor, sizeof(AI_governor));
		if(copy_to_user((AI_governor *)arg, &g, sizeof(AI_governor)))
				return -EACCES;
		break;
	case GOVERNOR_CLR_VARIABLES:
		//check against hardware min freq (TODO)
		//default values (TODO)
		current_AI_governor.profile.min_freq = 1000;
		current_AI_governor.profile.max_freq = 1500;
		current_AI_governor.profile.desired_frame_rate = 60;
		current_AI_governor.profile.current_frame_rate = 0;
		current_AI_governor.phase = 0;
		break;
	case GOVERNOR_SET_VARIABLES:
		if(copy_from_user(&g, (AI_governor *)arg, sizeof(AI_governor)))
			return -EACCES;
		current_AI_governor.profile.min_freq = g.profile.min_freq;
		current_AI_governor.profile.max_freq = g.profile.max_freq;
		current_AI_governor.profile.desired_frame_rate = g.profile.desired_frame_rate;
		current_AI_governor.profile.current_frame_rate = g.profile.current_frame_rate;
		current_AI_governor.phase = g.phase;
		break;
	case GOVERNOR_OTHER_FUNCT:
		current_AI_governor.profile.min_freq = 1;
		current_AI_governor.profile.max_freq = 2;
		current_AI_governor.profile.desired_frame_rate = 3;
		current_AI_governor.profile.current_frame_rate = 4;
		current_AI_governor.phase = 5;
		break;
	default:
		return -EINVAL;
		break;
	
	}
	return 0;
}

static struct file_operations AI_governor_fops = 
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
static char *device_node(struct device *dev, umode_t *mode)
{
	if(!mode)
		return NULL;
	*mode = 0666;
	return NULL;
}

static int __init AI_gov_ioctl_init(void){
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

static void __exit AI_gov_ioctl_exit(void)
{
	device_destroy(cl, dev);
	class_destroy(cl);
	cdev_del(&c_dev);
	unregister_chrdev_region(dev, MINOR_CNT);
}
 
module_init(AI_gov_ioctl_init);
module_exit(AI_gov_ioctl_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alex");
MODULE_DESCRIPTION("AI governor IOctl driver");
