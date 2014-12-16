################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/CPU_Proceso.c \
../src/codigoESO.c \
../src/cpuConfig.c \
../src/cpuKernel.c \
../src/cpuMSP.c \
../src/ejecucion.c 

OBJS += \
./src/CPU_Proceso.o \
./src/codigoESO.o \
./src/cpuConfig.o \
./src/cpuKernel.o \
./src/cpuMSP.o \
./src/ejecucion.o 

C_DEPS += \
./src/CPU_Proceso.d \
./src/codigoESO.d \
./src/cpuConfig.d \
./src/cpuKernel.d \
./src/cpuMSP.d \
./src/ejecucion.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"../../Commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


