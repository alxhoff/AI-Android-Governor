#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include "AI_governor_ioctl.h"

void other_funct(int fd){
	printf("other function executed");
	if(ioctl(fd, GOVERNOR_OTHER_FUNCT) == -1)
		perror("AI_gov_apps ioctl other");
}

void get_vars(int fd)
{
	AI_governor g;

	if(ioctl(fd, GOVERNOR_GET_VARIABLES, &g) == -1){
		perror("AI_gov_apps ioctl get");
	}else{
		printf("Min freq: %d\n", g.profile.min_freq);
		printf("Max freq: %d\n", g.profile.max_freq);
		printf("Desired frame rate: %d\n", g.profile.desired_frame_rate);
		printf("Current frame rate: %.2f\n", g.profile.current_frame_rate);
		printf("Current phase: %d\n", g.phase);
	}
}

void clr_vars(int fd)
{
	if(ioctl(fd, GOVERNOR_CLR_VARIABLES) == -1)
		perror("AI_gov_apps ioctl clr");
}

void set_vars(int fd)
{
	int v;
	AI_governor g;

	printf("Enter min freq: ");
	scanf("%d", &v);
	getchar();
	g.profile.min_freq = v;
	printf("Enter max freq: ");
	scanf("%d", &v);
	getchar();
	g.profile.max_freq = v;
	printf("Desired frame rate: ");
	scanf("%d", &v);
	getchar();
	g.profile.desired_frame_rate = v;
	printf("Current frame rate: ");
	scanf("%d", &v);
	getchar();
	g.profile.current_frame_rate = v;
	printf("Current phase: ");
	scanf("%d", &v);
	getchar();
	g.phase = v;

	if(ioctl(fd, GOVERNOR_SET_VARIABLES, &g) == -1)
	{
		perror("AI_gov_apps ioctl set");
	}
}

int main(int argc, char *argv[])
{
	int fd;

	enum{
		e_get,
		e_clr,
		e_set,
		e_other
	}command;

	if(argc == 1){
		command = e_get;
	}else if(argc == 2){
		if (strcmp(argv[1], "-g") == 0){
			command = e_get;
		}
		else if (strcmp(argv[1], "-c") == 0){
			command = e_clr;
		}
		else if (strcmp(argv[1], "-s") == 0){
			command = e_set;
		}
		else if (strcmp(argv[1], "-o") == 0){
			command = e_other;
		}
		else{
			fprintf(stderr, "Usage: %s [-g | -c | -s | -o]\n", argv[0]);
			return 1;
		}
	}
	else{
		fprintf(stderr, "Usage: %s [-g | -c | -s | -o]\n", argv[0]);
		return 1;
	}

	char *file_name = "/dev/AIgov";
	fd = open(file_name, O_RDWR);

	if(fd == -1){
		perror("AI_gove_apps open");
		return 2;
	}

	switch(command){
	case e_get:
		get_vars(fd);
		break;
	case e_clr:
		clr_vars(fd);
		break;
	case e_set:
		set_vars(fd);
		break;
	case e_other:
		other_funct(fd);
		break;
	default:
		break;
	}

	close(fd);

	return 0;
}




