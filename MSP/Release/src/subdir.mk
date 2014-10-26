################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ConfigMSP.c \
../src/Consola.c \
../src/MSP.c \
../src/Memoria.c \
../src/Programa.c \
../src/hiloCPU.c 

OBJS += \
./src/ConfigMSP.o \
./src/Consola.o \
./src/MSP.o \
./src/Memoria.o \
./src/Programa.o \
./src/hiloCPU.o 

C_DEPS += \
./src/ConfigMSP.d \
./src/Consola.d \
./src/MSP.d \
./src/Memoria.d \
./src/Programa.d \
./src/hiloCPU.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


