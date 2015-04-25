
BUILD_DIR   ?= bin

PREFIX	    ?= arm-none-eabi-
CC	    = $(PREFIX)gcc
LD	    = $(PREFIX)gcc
OBJCOPY	    = $(PREFIX)objcopy
OBJDUMP	    = $(PREFIX)objdump

OPENCM3_DIR =../libopencm3
OPENCM3_INC = $(OPENCM3_DIR)/include
OPENCM3_DEFS = -DSTM32L1

FREEMODBUS_DIR = ../freemodbus-v1.5.0
FREEMODBUS_INC = $(FREEMODBUS_DIR)/include
FREEMODBUS_DIRS = $(FREEMODBUS_DIR) $(patsubst %,$(FREEMODBUS_DIR)/%, functions rtu ascii tcp)
FREEMODBUS_SRCS := $(notdir $(foreach dir,$(FREEMODBUS_DIRS),$(wildcard $(dir)/*.c)))

FREEMODBUS_PORT_DIR = mb_port
FREEMODBUS_PORT_SRCS = $(notdir $(wildcard $(FREEMODBUS_PORT_DIR)/*.c))

VPATH += $(FREEMODBUS_DIRS) $(FREEMODBUS_PORT_DIR)

# Inclusion of header files
INCLUDES += $(patsubst %,-I%, . $(OPENCM3_INC) $(FREEMODBUS_INC) $(FREEMODBUS_DIRS) $(FREEMODBUS_PORT_DIR))

CFLAGS += -Os -ggdb3 -std=c99
CFLAGS += -mcpu=cortex-m3 -msoft-float -mthumb
CFLAGS +=  $(INCLUDES) -fno-common -MD
CFLAGS += -Wall -Wextra -Wshadow -Wno-unused-variable -Wimplicit-function-declaration
CFLAGS += -Wredundant-decls -Wstrict-prototypes
#CFLAGS += -Wmissing-prototypes
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += $(OPENCM3_DEFS)
CFLAGS += -DNDEBUG # squelch asserts in freemodbus lib

LDSCRIPT = $(OPENCM3_DIR)/lib/stm32/l1/stm32l15xxb.ld

LDFLAGS += -I . -lc -T$(LDSCRIPT) -L$(OPENCM3_DIR)/lib -nostartfiles -Wl,--gc-sections
LDFLAGS += -lopencm3_stm32l1
# nosys is only in newer gcc-arm-embedded...
#LDFLAGS += -specs=nosys.specs
LDFLAGS += -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group
LDFLAGS += -Wl,--undefined=uxTopUsedPriority

CFILES += $(FREEMODBUS_SRCS)
CFILES += $(FREEMODBUS_PORT_SRCS)

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

