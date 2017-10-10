#ifndef AI_GOV_H
#define AI_GOV_H

#include <linux/ioctl.h>

#include "AI_gov_phases.h"

#define FIRST_MINOR     0
#define MINOR_CNT       1

struct AI_gov_ioctl_phase_variable{
	enum PHASE_ENUM phase;

	unsigned char variable_index;

	unsigned long variable_value;
};

extern struct file_operations AI_governor_fops;

#define GOVERNOR_GET_PHASE _IOR('g', 1, unsigned long)
#define GOVERNOR_SET_PHASE _IOW('g', 2, unsigned long)
#define GOVERNOR_CLR_PHASE_VARIABLES _IO('g', 3)
#define GOVERNOR_GET_PHASE_VARIABLE _IOWR('g', 4, struct AI_gov_ioctl_phase_variable*)
#define GOVERNOR_SET_PHASE_VARIABLE _IOW('g', 5, struct AI_gov_ioctl_phase_variable*)

#endif

