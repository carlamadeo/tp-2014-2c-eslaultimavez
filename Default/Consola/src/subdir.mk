################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Consola/src/Consola.c 

OBJS += \
./Consola/src/Consola.o 

C_DEPS += \
./Consola/src/Consola.d 


# Each subdirectory must supply rules for building sources it contributes
Consola/src/%.o: ../Consola/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


