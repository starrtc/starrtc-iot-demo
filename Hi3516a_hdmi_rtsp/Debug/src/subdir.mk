################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/CAudioAac.cpp \
../src/V3516A.cpp \
../src/g711.cpp \
../src/cJSON.cpp \
../src/CInterfaceUrls.cpp \
../src/main.cpp 

OBJS += \
./src/CAudioAac.o \
./src/V3516A.o \
./src/g711.o \
./src/cJSON.o \
./src/CInterfaceUrls.o \
./src/main.o 

CPP_DEPS += \
./src/CAudioAac.d \
./src/V3516A.d \
./src/g711.d \
./src/cJSON.d \
./src/CInterfaceUrls.d \
./src/main.d 

starrtc_include=/root/starrtc_libs/jni
ffmpeg_include=/root/ffmpeg-build
# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-hisiv300-linux-g++ -I"../src" -I"../hi_sdk_50/include" -I"../third_lib/include/fdk-aac" -I"../third_lib/include" -I"$(ffmpeg_include)/include/" -O0 -g3  -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	#arm-hisiv300-linux-g++ -I"../src" -I"../hi_sdk_50/include" -I"../third_lib/include/fdk-aac" -I"../third_lib/include" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


