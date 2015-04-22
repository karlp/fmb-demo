
PROJECT	    = fmb
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

VPATH += $(FREERTOS_SRC) $(FREERTOS_PORT) $(FREERTOS_MMG)

# Inclusion of header files
INCLUDES = $(patsubst %,-I%, . $(FREERTOS_INC) $(FREERTOS_PORT) $(OPENCM3_INC) )

CFLAGS += -Os -g -std=c99 
CFLAGS += -mcpu=cortex-m3 -msoft-float -mthumb
CFLAGS +=  $(INCLUDES) -fno-common -MD
CFLAGS += -Wall -Wextra -Wshadow -Wno-unused-variable -Wimplicit-function-declaration
CFLAGS += -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += $(OPENCM3_DEFS)

LDSCRIPT = $(OPENCM3_DIR)/lib/stm32/l1/stm32l15xxb.ld

LDFLAGS += -I . -lc -T$(LDSCRIPT) -L$(OPENCM3_DIR)/lib -nostartfiles -Wl,--gc-sections
LDFLAGS += -lopencm3_stm32l1
#LDFLAGS	   += -specs=nosys.specs
LDFLAGS += -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

CFILES = fmb.c
CFILES += $(FREERTOS_SRCS)

OBJS = $(CFILES:.c=.o)

all: $(PROJECT).elf $(PROJECT).bin
flash: $(PROJECT).flash

$(PROJECT).elf: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

$(PROJECT).bin: $(PROJECT).elf
	$(OBJCOPY) -O binary  $< $@

$(PROJECT).lss: $(PROJECT).elf
	$(OBJDUMP) -h -S $< > $@

$(PROJECT).list: $(PROJECT).elf
	$(OBJDUMP) -S $< > $@

%.flash: %.elf
	@printf "  FLASH   $<\n"
	$(Q)$(OOCD) -f interface/$(OOCD_INTERFACE).cfg \
		-f board/$(OOCD_BOARD).cfg \
		-c "program $(*).elf" \
		-c "reset" \
		-c "shutdown" $(NULL)

clean:
	rm -f *.elf *.o *.d *.hex *.list *.sym *.bin *.lss

.PHONY: all clean

OOCD            ?= openocd
OOCD_INTERFACE  ?= stlink-v2
OOCD_BOARD      ?= stm32ldiscovery

