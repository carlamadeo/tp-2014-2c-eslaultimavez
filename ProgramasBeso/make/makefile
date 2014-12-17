################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: ProgramaBeso

dependents:
	-cd ../../Commons/make && $(MAKE) all

# Tool invocations
ProgramaBeso: $(OBJS) $(USER_OBJS) ../../Commons/make/libCommons.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"../../Commons/make" -o "ProgramaBeso" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) ProgramaBeso
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
../../Commons/make/libCommons.so:

-include ../makefile.targets
