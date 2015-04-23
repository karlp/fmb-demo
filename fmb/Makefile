
PROJECT	    = fmb
BUILD_DIR   = bin

PREFIX	    ?= arm-none-eabi-
CC	    = $(PREFIX)gcc
LD	    = $(PREFIX)gcc
OBJCOPY	    = $(PREFIX)objcopy
OBJDUMP	    = $(PREFIX)objdump

OPENCM3_DIR =../libopencm3
OPENCM3_INC = $(OPENCM3_DIR)/include
OPENCM3_DEFS = -DSTM32L1
FREERTOS_DIR = /home/karlp/src/FreeRTOSV8.2.1/FreeRTOS
FREERTOS_PORT = $(FREERTOS_DIR)/Source/portable/GCC/ARM_CM3
FREERTOS_INC = $(FREERTOS_DIR)/Source/include
FREERTOS_SRC = $(FREERTOS_DIR)/Source
FREERTOS_MMG = $(FREERTOS_DIR)/Source/portable/MemMang
FREERTOS_SRCS = list.c queue.c tasks.c timers.c port.c heap_1.c
FREERTOS_SRCS += FreeRTOS-openocd.c

VPATH += $(FREERTOS_SRC) $(FREERTOS_PORT) $(FREERTOS_MMG)

# Inclusion of header files
INCLUDES = $(patsubst %,-I%, . $(FREERTOS_INC) $(FREERTOS_PORT) $(OPENCM3_INC) )

CFLAGS += -Os -ggdb3 -std=c99
CFLAGS += -mcpu=cortex-m3 -msoft-float -mthumb
CFLAGS +=  $(INCLUDES) -fno-common -MD
CFLAGS += -Wall -Wextra -Wshadow -Wno-unused-variable -Wimplicit-function-declaration
CFLAGS += -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += $(OPENCM3_DEFS)

LDSCRIPT = $(OPENCM3_DIR)/lib/stm32/l1/stm32l15xxb.ld

LDFLAGS += -I . -lc -T$(LDSCRIPT) -L$(OPENCM3_DIR)/lib -nostartfiles -Wl,--gc-sections
LDFLAGS += -lopencm3_stm32l1
LDFLAGS += -specs=nosys.specs
LDFLAGS += -Wl,--undefined=uxTopUsedPriority

CFILES = fmb.c
CFILES += $(FREERTOS_SRCS)

OBJS = $(CFILES:%.c=$(BUILD_DIR)/%.o)

all: $(PROJECT).elf $(PROJECT).bin
flash: $(PROJECT).flash

# Need a special rule to have a bin dir
$(BUILD_DIR)/%.o: %.c
	@#printf "  CC      $(subst $(shell pwd)/,,$(@))\n"
	@mkdir -p $(dir $@)
	$(Q)$(CC) $(CFLAGS) -o $@ -c $<

$(PROJECT).elf: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

%.bin: %.elf
	$(OBJCOPY) -O binary  $< $@

%.lss: %.elf
	$(OBJDUMP) -h -S $< > $@

%.list: %.elf
	$(OBJDUMP) -S $< > $@

%.flash: %.elf
	@printf "  FLASH   $<\n"
	$(Q)$(OOCD) -f interface/$(OOCD_INTERFACE).cfg \
		-f board/$(OOCD_BOARD).cfg \
		-c "program $(*).elf" \
		-c "reset" \
		-c "shutdown" $(NULL)

clean:
	rm -rf $(BUILD_DIR) $(PROJECT).{elf,bin} $(PROJECT).{list,lss}

.PHONY: all clean

OOCD            ?= openocd
OOCD_INTERFACE  ?= stlink-v2
OOCD_BOARD      ?= stm32ldiscovery

