######################################
# target
######################################
TARGET = trf


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -O0


#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES =  \
src/main.c \
src/stm32h7xx_it.c \
src/system_stm32h7xx.c \
src/trf_clock.c \
src/trf_system.c \
src/trf_scheduler.c \
src/trf_adc.c \
src/trf_gpio.c \
src/trf_usart.c \
src/trf_stepper.c \
src/trf_lcd.c \
hal/src/stm32h7xx_hal_cortex.c \
hal/src/stm32h7xx_hal_rcc.c \
hal/src/stm32h7xx_hal_rcc_ex.c \
hal/src/stm32h7xx_hal_flash.c \
hal/src/stm32h7xx_hal_flash_ex.c \
hal/src/stm32h7xx_hal_gpio.c \
hal/src/stm32h7xx_hal_hsem.c \
hal/src/stm32h7xx_hal_dma.c \
hal/src/stm32h7xx_hal_dma_ex.c \
hal/src/stm32h7xx_hal_mdma.c \
hal/src/stm32h7xx_hal_pwr.c \
hal/src/stm32h7xx_hal_pwr_ex.c \
hal/src/stm32h7xx_hal.c \
hal/src/stm32h7xx_hal_i2c.c \
hal/src/stm32h7xx_hal_i2c_ex.c \
hal/src/stm32h7xx_hal_exti.c \
hal/src/stm32h7xx_hal_eth.c \
hal/src/stm32h7xx_hal_eth_ex.c \
hal/src/stm32h7xx_hal_tim.c \
hal/src/stm32h7xx_hal_tim_ex.c \
hal/src/stm32h7xx_hal_uart.c \
hal/src/stm32h7xx_hal_uart_ex.c \
hal/src/stm32h7xx_hal_usart_ex.c \
hal/src/stm32h7xx_hal_adc.c \
hal/src/stm32h7xx_hal_adc_ex.c

# ASM sources
ASM_SOURCES =  \
startup_stm32h723xx.s


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m7

# fpu
FPU = -mfpu=auto

# float-abi
FLOAT-ABI = -mfloat-abi=soft

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32H723xx

# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-Iinclude \
-Ihal/cmsis/device/st/stm32h7xx/include \
-Ihal/cmsis/include \
-Ihal/include \
-Ihal/include/Legacy

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = STM32H723ZGTx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@
	
$(BUILD_DIR):
	mkdir -p $@

#######################################
# clean up
#######################################
clean:
	-rm -rf $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***