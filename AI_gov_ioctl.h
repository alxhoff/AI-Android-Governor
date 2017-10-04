#ifndef AI_GOV_H
#define AI_GOV_H

#include <linux/ioctl.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <asm/uaccess.h>

#include <linux/ioctl.h>

#include "AI_gov_phases.h"
#include "AI_gov.h"


#define FIRST_MINOR     0
#define MINOR_CNT       1


//static dev_t dev;
//static struct cdev c_dev;
//static struct class *cl;

extern struct file_operations AI_governor_fops;

#define GOVERNOR_GET_PROFILE _IOR('g', 1, struct AI_gov_profile*)
#define GOVERNOR_SET_PROFILE _IOW('g', 2, struct AI_gov_profile*)
#define GOVERNOR_CLR_PROFILE _IO('g', 3)

#define GOVERNOR_SET_PHASE _IOW('g', 4, enum PHASE_ENUM*)
#define GOVERNOR_GET_PHASE _IOR('g', 5, enum PHASE_ENUM*)

int AI_gov_open(struct inode *i, struct file *f);
int AI_gov_close(struct inode *i, struct file *f);
long AI_gov_ioctl(struct file *f, unsigned int cmd, unsigned long arg);
int AI_gov_ioctl_init(void);
int AI_gov_ioctl_exit(void);

#endif
