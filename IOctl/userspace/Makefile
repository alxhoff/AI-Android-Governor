CLANG = $(shell man clang 2>/dev/null)

CC_android ?= arm-linux-androideabi-gcc
CXX_android ?= arm-linux-androideabi-g++

GCC_VERSIONGT4 := $(shell expr `gcc -dumpversion | cut -f1 -d.` \> 4)
ANDROIDCC_VERSIONGT4 := $(shell expr `$(CC_android) -dumpversion | cut -f1 -d.` \> 4)

ifeq "$(CLANG)" ""
CC = gcc
CXX = g++
else
CC = clang
CXX = clang++
endif

FLAGS = -Wall -O0

CORE_SRC_DIR = .
CORE_INCL_DIR = .
BUILD_DIR = ./build
CORE_SOURCES = $(wildcard $(CORE_SRC_DIR)/*.c)
# if there are any files not matched by *.cc, add them below
CORE_SOURCES +=

FLAGS += -I$(CORE_INCL_DIR)

# don't modify
CORE_OBJS = $(patsubst $(CORE_SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(CORE_SOURCES))

ifdef TARGET
ifeq ($(TARGET), android)
COMPILER = $(CC_android)
COMPILER_XX = $(CXX_android)
EXECUTABLE_OUTPUT = "android_executable"
else
COMPILER = $(CC)
COMPILER_XX = $(CXX)
EXECUTABLE_OUTPUT = "executable"
endif
else
COMPILER = $(CC_android)
COMPILER_XX = $(CXX_android)
EXECUTABLE_OUTPUT = "android_executable"
endif

default:
	echo $(CORE_SOURCES)
	$(MAKE) linux

.PHONY: all
all: $(BUILD_DIR) $(CORE_OBJS)
	$(COMPILER) $(FLAGS) $(CORE_OBJS) -o $(BUILD_DIR)/$(EXECUTABLE_OUTPUT)
	ln -s $(BUILD_DIR)/$(EXECUTABLE_OUTPUT)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(CORE_SRC_DIR)/%.c
	echo "building" $<
	$(COMPILER) $(FLAGS) -c $^ -o $@


.PHONY: install
install: android
	adb push android_executable /data/local/tmp/ioctl_tester

.PHONY: clean
clean:
	rm -f executable android_executable
	rm -f $(CORE_OBJS)
	rm -rf $(BUILD_DIR)

.PHONY: linux android
android:
	$(MAKE) clean
	$(MAKE) TARGET=android all

linux:
	$(MAKE) clean
	$(MAKE) TARGET=linux all
