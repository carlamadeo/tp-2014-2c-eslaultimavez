################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../commons/bitarray.c \
../commons/config.c \
../commons/cpu.c \
../commons/error.c \
../commons/kernel.c \
../commons/log.c \
../commons/panel.c \
../commons/process.c \
../commons/protocolStructInBigBang.c \
../commons/socketInBigBang.c \
../commons/string.c \
../commons/temporal.c \
../commons/txt.c 

OBJS += \
./commons/bitarray.o \
./commons/config.o \
./commons/cpu.o \
./commons/error.o \
./commons/kernel.o \
./commons/log.o \
./commons/panel.o \
./commons/process.o \
./commons/protocolStructInBigBang.o \
./commons/socketInBigBang.o \
./commons/string.o \
./commons/temporal.o \
./commons/txt.o 

C_DEPS += \
./commons/bitarray.d \
./commons/config.d \
./commons/cpu.d \
./commons/error.d \
./commons/kernel.d \
./commons/log.d \
./commons/panel.d \
./commons/process.d \
./commons/protocolStructInBigBang.d \
./commons/socketInBigBang.d \
./commons/string.d \
./commons/temporal.d \
./commons/txt.d 


# Each subdirectory must supply rules for building sources it contributes
commons/%.o: ../commons/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


