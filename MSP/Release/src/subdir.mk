################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/CPU.c \
../src/ConfigMSP.c \
../src/Consola.c \
../src/Kernel.c \
../src/MSP.c \
../src/Memoria.c \
../src/Programa.c 

OBJS += \
./src/CPU.o \
./src/ConfigMSP.o \
./src/Consola.o \
./src/Kernel.o \
./src/MSP.o \
./src/Memoria.o \
./src/Programa.o 

C_DEPS += \
./src/CPU.d \
./src/ConfigMSP.d \
./src/Consola.d \
./src/Kernel.d \
./src/MSP.d \
./src/Memoria.d \
./src/Programa.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


