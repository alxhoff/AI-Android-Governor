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



typedef struct{
	unsigned int min_freq;
	unsigned int max_freq;

	unsigned int desired_frame_rate;
	float current_frame_rate;
} AI_gov_profile;

typedef struct{
	AI_gov_profile profile;
	phase_state phase;
	phase_state prev_phase;
}AI_governor;

dev_t dev;
struct cdev c_dev;
struct class *cl;
extern AI_governor AI_gov;

extern struct file_operations AI_governor_fops;

#define GOVERNOR_GET_VARIABLES _IOR('g', 1, AI_governor*)
#define GOVERNOR_CLR_VARIABLES _IO('g', 2)
#define GOVERNOR_SET_VARIABLES _IOW('g', 3, AI_governor*)
#define GOVERNOR_OTHER_FUNCT _IO('g', 4)

int AI_governor_open(struct inode *i, struct file *f);
int AI_governor_close(struct inode *i, struct file *f);
long AI_governor_ioctl(struct file *f, unsigned int cmd, unsigned long arg);
int AI_governor_ioctl_init(void);
void AI_governor_ioctl_exit(void);

#endif
