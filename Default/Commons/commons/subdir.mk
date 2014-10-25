################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Commons/commons/bitarray.c \
../Commons/commons/config.c \
../Commons/commons/error.c \
../Commons/commons/log.c \
../Commons/commons/process.c \
../Commons/commons/protocolStructInBigBang.c \
../Commons/commons/socketInBigBang.c \
../Commons/commons/string.c \
../Commons/commons/temporal.c \
../Commons/commons/txt.c 

OBJS += \
./Commons/commons/bitarray.o \
./Commons/commons/config.o \
./Commons/commons/error.o \
./Commons/commons/log.o \
./Commons/commons/process.o \
./Commons/commons/protocolStructInBigBang.o \
./Commons/commons/socketInBigBang.o \
./Commons/commons/string.o \
./Commons/commons/temporal.o \
./Commons/commons/txt.o 

C_DEPS += \
./Commons/commons/bitarray.d \
./Commons/commons/config.d \
./Commons/commons/error.d \
./Commons/commons/log.d \
./Commons/commons/process.d \
./Commons/commons/protocolStructInBigBang.d \
./Commons/commons/socketInBigBang.d \
./Commons/commons/string.d \
./Commons/commons/temporal.d \
./Commons/commons/txt.d 


# Each subdirectory must supply rules for building sources it contributes
Commons/commons/%.o: ../Commons/commons/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


