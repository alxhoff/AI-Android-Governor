#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include "AI_gov_ioctl.h"
#include "AI_gov_phases.h"

#define FOR_EACH_PHASE(PHASE) \
		PHASE(AI_init) \
		PHASE(AI_framerate) \
		PHASE(AI_priority) \
		PHASE(AI_time) \
		PHASE(AI_powersave) \
		PHASE(AI_performance) \
		PHASE(AI_response) \
		PHASE(AI_exit) 

#define GENERATE_ENUM(ENUM) ENUM,

void get_phase(int fd)
{
	enum PHASE_ENUM g;

	if(ioctl(fd, GOVERNOR_GET_PHASE, &g) == -1)
		perror("AI_gov_apps ioctlt get_phase");
	else{
		switch(g){
		case AI_init:
			printf("AI_init \n");
			break;
		case AI_framerate:
			printf("AI_framerate \n");
			break;
		case AI_priority:
			printf("AI_priority \n");
			break;
		case AI_time:
			printf("AI_time \n");
			break;
		case AI_powersave:
			printf("AI_powersave \n");
			break;
		case AI_performance:
			printf("AI_performance \n");
			break;
		case AI_response:
			printf("AI_response \n");
			break;
		case AI_exit:
			printf("AI_exit \n");
			break;
		default:
			printf("INVALID \n");
			break;
		}
	}
}

void set_phase(int fd)
{
	int v;
	enum PHASE_ENUM g;

	printf("Enter phase to set: ");
	scanf("%d", &v);
	getchar();
	g = v;

	if(ioctl(fd, GOVERNOR_SET_PHASE, &g) == -1)
	{
		perror("AI_gov_apps profile set");
	}
}

void clr_phase_variables(int fd)
{
	if(ioctl(fd, GOVERNOR_CLR_PHASE_VARIABLES) == -1)
		perror("AI_gov_apps ioctl clr phase variables");
}

void set_phase_variable(int fd)
{
	int v;
	struct AI_gov_ioctl_phase_variable g;

	printf("Enter variable index: ");
	scanf("%d", &v);
	getchar();
	g.variable_index = (unsigned char)v;
	printf("Enter variable's new value: ");
	scanf("%d", &v);
	getchar();
	g.variable_value = (unsigned long)v;
	if(ioctl(fd, GOVERNOR_SET_PHASE_VARIABLE, &g) == -1)
		perror("AI_gov_apps ioctlt get_profile");
	else
		
		printf("Set phase: %d variable at index %d to %lu\n", 
			g.phase, g.variable_index, g.variable_value);
}

void get_phase_variable(int fd)
{
	int v;
	struct AI_gov_ioctl_phase_variable g;

	printf("Enter variable index: ");
	scanf("%d", &v);
	getchar();
	g.variable_index = (unsigned char)v;
	
	if(ioctl(fd, GOVERNOR_GET_PHASE_VARIABLE, &g) == -1)
		perror("AI_gov_apps profile set");
	else
		printf("Variable from phase %d at index %d has value %lu \n",
			g.phase, g.variable_index, g.variable_value);	
}

int main(int argc, char *argv[])
{
	int fd;

	enum{
		e_get_p,
		e_set_p,
		e_clr,
		e_get_v,
		e_set_v
	}command;

	if(argc == 1){
		command = e_get_p;
		fprintf(stderr, "Please specify a command, default case get phase will"
			"execute \n");
	}else if(argc == 2){
		if (strcmp(argv[1], "-gp") == 0){
			command = e_get_p;
		}
		else if (strcmp(argv[1], "-sp") == 0){
			command = e_set_p;
		}
		else if (strcmp(argv[1], "-clr") == 0){
			command = e_clr;
		}
		else if (strcmp(argv[1], "-gv") == 0){
			command = e_get_v;
		}
		else if (strcmp(argv[1], "-sv") == 0){
			command = e_set_v;
		}
		else{
			fprintf(stderr, "Usage: %s [-gp (get phase) | -sp (set phase) | -clr (clear "
			       "phase variables) | -gv (get variable) | -sv (set variable)]\n", argv[0]);
			return 1;
		}
	}
	else{
		fprintf(stderr, "Usage: %s [-gp (get phase) | -sp (set phase) | -clr (clear "
		       "phase variables) | -gv (get variable) | -sv (set variable)]\n", argv[0]);
		return 1;
	}

	char *file_name = "/dev/AI_governor_ioctl";
	fd = open(file_name, O_RDWR);

	if(fd == -1){
		perror("AI_gov_app open");
		return 2;
	}

	switch(command){
	case e_get_p:
		get_phase(fd);
		break;
	case e_set_p:
		set_phase(fd);
		break;
	case e_clr:
		clr_phase_variables(fd);
		break;
	case e_get_v:
		get_phase_variable(fd);
		break;
	case e_set_v:
		set_phase_variable(fd);
		break;
	default:
		break;
	}

	close(fd);

	return 0;
}
