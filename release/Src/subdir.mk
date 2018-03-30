################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Src/testOnDemandRTSPServer.cpp \
../Src/DD_H264VideoFileServerMediaSubsession.cpp \
../Src/Tiam335xH264Source.cpp \
../Src/FetchData.cpp \
../Src/CONSTANT.cpp \
../Src/cbuf.cpp \
../Src/thread.cpp \
../Src/v4l2uvc.cpp \
../Src/h264_xu_ctrls.cpp 


OBJS += \
./Src/testOnDemandRTSPServer.o \
./Src/DD_H264VideoFileServerMediaSubsession.o \
./Src/CONSTANT.o \
./Src/FetchData.o \
./Src/cbuf.o \
./Src/thread.o \
./Src/Tiam335xH264Source.o \
./Src/v4l2uvc.o \
./Src/h264_xu_ctrls.o 


CPP_DEPS += \
./Src/testOnDemandRTSPServer.d \
./Src/DD_H264VideoFileServerMediaSubsession.d \
./Src/Tiam335xH264Source.d \
./Src/FetchData.d \
./Src/CONSTANT.d \
./Src/cbuf.d \
./Src/thread.d \
./Src/v4l2uvc.d \
./Src/h264_xu_ctrls.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.cpp 
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	$(GPP) -rdynamic -I"./Head" -O0 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


