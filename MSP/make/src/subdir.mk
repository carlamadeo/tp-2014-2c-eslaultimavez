################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/MSP.c \
../src/Memoria.c \
../src/mspCPU.c \
../src/mspConfig.c \
../src/mspConsola.c \
../src/mspKernel.c \
../src/mspPrograma.c 

OBJS += \
./src/MSP.o \
./src/Memoria.o \
./src/mspCPU.o \
./src/mspConfig.o \
./src/mspConsola.o \
./src/mspKernel.o \
./src/mspPrograma.o 

C_DEPS += \
./src/MSP.d \
./src/Memoria.d \
./src/mspCPU.d \
./src/mspConfig.d \
./src/mspConsola.d \
./src/mspKernel.d \
./src/mspPrograma.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"../../Commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


