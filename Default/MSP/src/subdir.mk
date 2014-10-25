################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../MSP/src/ConfigMSP.c \
../MSP/src/Consola.c \
../MSP/src/MSP.c \
../MSP/src/Memoria.c \
../MSP/src/Programa.c \
../MSP/src/hiloCPU.c 

OBJS += \
./MSP/src/ConfigMSP.o \
./MSP/src/Consola.o \
./MSP/src/MSP.o \
./MSP/src/Memoria.o \
./MSP/src/Programa.o \
./MSP/src/hiloCPU.o 

C_DEPS += \
./MSP/src/ConfigMSP.d \
./MSP/src/Consola.d \
./MSP/src/MSP.d \
./MSP/src/Memoria.d \
./MSP/src/Programa.d \
./MSP/src/hiloCPU.d 


# Each subdirectory must supply rules for building sources it contributes
MSP/src/%.o: ../MSP/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


