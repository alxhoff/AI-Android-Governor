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

struct AI_gov_ioctl_phase_variable{
	enum PHASE_ENUM phase;

	uint8_t variable_index;

	unsigned long variable_value;
};

static dev_t dev;
static struct cdev c_dev;
static struct class *cl;

extern struct file_operations AI_governor_fops;

#define GOVERNOR_GET_PHASE _IOR('g', 1, unsigned long)
#define GOVERNOR_SET_PHASE _IOW('g', 2, unsigned long)
#define GOVERNOR_CLR_PHASE_VARIABLES _IO('g', 3)
#define GOVERNOR_GET_PHASE_VARIABLE _IOWR('g', 4, struct AI_gov_ioctl_phase_variable*)
#define GOVERNOR_SET_PHASE_VARIABLE _IOW('g', 5, struct AI_gov_ioctl_phase_variable*)

int AI_gov_open(struct inode *i, struct file *f);
int AI_gov_close(struct inode *i, struct file *f);
long AI_gov_ioctl(struct file *f, unsigned int cmd, unsigned long arg);
int AI_gov_ioctl_init(void);
int AI_gov_ioctl_exit(void);

#endif
