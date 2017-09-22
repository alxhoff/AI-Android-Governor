#ifndef GOVERNOR_DATATYPE_H
#define GOVERNOR_DATATYPE_H
#include <linux/ioctl.h>

typedef enum{
	response,
	animation,
	idle,
	load
} phase_state;

typedef struct{
	unsigned int min_freq;
	unsigned int max_freq;

	unsigned int desired_frame_rate;
	float current_frame_rate;
} AI_gov_profile;

typedef struct{
	AI_gov_profile profile;
	phase_state phase;
}AI_governor;

#define GOVERNOR_GET_VARIABLES _IOR('g', 1, AI_governor*)
#define GOVERNOR_CLR_VARIABLES _IO('g', 2)
#define GOVERNOR_SET_VARIABLES _IOW('g', 3, AI_governor*)
#define GOVERNOR_OTHER_FUNCT _IO('g', 4)

#endif
