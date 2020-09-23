################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include common/subdir.mk
-include Libraries/STM32F10x_StdPeriph_Driver/src/subdir.mk
-include Driver/src/subdir.mk
-include Application/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
Ocm_deom.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
Ocm_deom.list \

OBJCOPY_HEX += \
Ocm_deom.hex \

OBJCOPY_BIN += \
Ocm_deom.bin \


# All Target
all: Ocm_deom.elf secondary-outputs

# Tool invocations
Ocm_deom.elf: $(OBJS) $(USER_OBJS) ../stm32_flash.ld
	arm-none-eabi-gcc -o "Ocm_deom.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m3 -T"../stm32_flash.ld" --specs=nosys.specs -Wl,-Map="Ocm_deom.map" -Wl,--gc-sections -static  -mfloat-abi=soft -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

Ocm_deom.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "Ocm_deom.list"
	@echo 'Finished building: $@'
	@echo ' '

Ocm_deom.hex: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "Ocm_deom.hex"
	@echo 'Finished building: $@'
	@echo ' '

Ocm_deom.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "Ocm_deom.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
