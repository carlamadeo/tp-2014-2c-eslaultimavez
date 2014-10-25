################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Commons/commons/collections/dictionary.c \
../Commons/commons/collections/list.c \
../Commons/commons/collections/queue.c 

OBJS += \
./Commons/commons/collections/dictionary.o \
./Commons/commons/collections/list.o \
./Commons/commons/collections/queue.o 

C_DEPS += \
./Commons/commons/collections/dictionary.d \
./Commons/commons/collections/list.d \
./Commons/commons/collections/queue.d 


# Each subdirectory must supply rules for building sources it contributes
Commons/commons/collections/%.o: ../Commons/commons/collections/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


