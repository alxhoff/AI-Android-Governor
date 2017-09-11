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

#define FIRST_MINOR     0
#define MINOR_CNT       1


static dev_t dev;
static struct cdev c_dev;
static struct class *cl;

extern struct file_operations AI_governor_fops;

#define GOVERNOR_GET_VARIABLES _IOR('g', 1, AI_governor*)
#define GOVERNOR_CLR_VARIABLES _IO('g', 2)
#define GOVERNOR_SET_VARIABLES _IOW('g', 3, AI_governor*)
#define GOVERNOR_OTHER_FUNCT _IO('g', 4)

int AI_gov_open(struct inode *i, struct file *f);
int AI_gov_close(struct inode *i, struct file *f);
long AI_gov_ioctl(struct file *f, unsigned int cmd, unsigned long arg);
int AI_gov_ioctl_init(void);
void AI_gov_ioctl_exit(void);

#endif
