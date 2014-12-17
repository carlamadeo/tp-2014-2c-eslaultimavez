################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Kernel_Proceso.c \
../src/Loader.c \
../src/Planificador.c \
../src/boot.c \
../src/kernelConfig.c \
../src/kernelMSP.c \
../src/planificadorMensajesCPU.c 

OBJS += \
./src/Kernel_Proceso.o \
./src/Loader.o \
./src/Planificador.o \
./src/boot.o \
./src/kernelConfig.o \
./src/kernelMSP.o \
./src/planificadorMensajesCPU.o 

C_DEPS += \
./src/Kernel_Proceso.d \
./src/Loader.d \
./src/Planificador.d \
./src/boot.d \
./src/kernelConfig.d \
./src/kernelMSP.d \
./src/planificadorMensajesCPU.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"../../Commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


