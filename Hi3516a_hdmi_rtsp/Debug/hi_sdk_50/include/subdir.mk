################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../hi_sdk_50/include/sample_comm_vo.c 

OBJS += \
./hi_sdk_50/include/sample_comm_vo.o 

C_DEPS += \
./hi_sdk_50/include/sample_comm_vo.d 


# Each subdirectory must supply rules for building sources it contributes
hi_sdk_50/include/%.o: ../hi_sdk_50/include/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-hisiv300-linux-gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


