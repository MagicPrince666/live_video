################################################################################
# Automatically-generated file. Do not edit!
################################################################################
##Added by Dunken,2015-4-9
##indicate header path
CPLUS_INCLUDE_PATH += ../live/BasicUsageEnvironment/include:../live/UsageEnvironment/include:../live/liveMedia/include:../live/groupsock/include:../Head
export CPLUS_INCLUDE_PATH
#################################

#export PATH=$PATH:/Volumes/OpenWrt/openwrt_widora/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin
#export STAGING_DIR=/Volumes/OpenWrt/openwrt_widora/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin:$STAGING_DIR
COMIPLE_TOOL_PREFIX := mipsel-openwrt-linux-uclibc-
CC := $(COMIPLE_TOOL_PREFIX)gcc
GPP := $(COMIPLE_TOOL_PREFIX)g++
AR := $(COMIPLE_TOOL_PREFIX)ar
AS := $(COMIPLE_TOOL_PREFIX)as
STRIP := $(COMIPLE_TOOL_PREFIX)strip
LD := $(COMIPLE_TOOL_PREFIX)ld

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Src/subdir.mk
-include objects.mk

-include $(C++_DEPS)
-include $(C_DEPS)
-include $(CC_DEPS)
-include $(CPP_DEPS)
-include $(CXX_DEPS)
-include $(C_UPPER_DEPS)

-include ../makefile.defs


# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: RTSPServer


livedir := ../live/
livemedia := $(livedir)liveMedia
usageenviroment := $(livedir)UsageEnvironment 
basicusage := $(livedir)BasicUsageEnvironment 
groupsock := $(livedir)groupsock
live := $(livedir)
LIBSDIR += -L$(usageenviroment) -L$(basicusage) -L$(live) -L$(livemedia) -L$(groupsock)

# Tool invocations
RTSPServer: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	$(GPP) -rdynamic -O0 -Wl,-Map=RTSPServer.map $(LIBSDIR) -o "RTSPServer" $(OBJS) $(USER_OBJS) $(LIBS) -lpthread -ldl
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C++_DEPS)$(C_DEPS)$(CC_DEPS)$(CPP_DEPS)$(EXECUTABLES)$(CXX_DEPS)$(C_UPPER_DEPS) RTSPServer
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
