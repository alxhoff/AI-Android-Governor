#if called directly from the command line, invoke the kernel build system.
KVERSION = $(shell uname -r)
ifeq ($(KERNELRELEASE),)
	KERNEL_SOURCE := /lib/modules/$(KVERSION)/build 
	#KERNEL_SOURCE := /usr/src/linux
	PWD := $(shell pwd)
default: module AI_governor_app
	 
module:
	$(MAKE) -C $(KERNEL_SOURCE) SUBDIRS=$(PWD) modules
	     
clean:
	$(MAKE) -C $(KERNEL_SOURCE) SUBDIRS=$(PWD) clean
	${RM} AI_governor_app
		 
# Otherwise KERNELRELEASE is defined; we've been invoked from the
# # kernel build system and can use its language.
else
  
obj-m := AI_governor_ioctl.o
       
endif
