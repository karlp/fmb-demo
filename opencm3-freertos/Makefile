
PROJECT	    = fmb-opencm3-freertos

#FREERTOS_DIR = /home/karlp/src/FreeRTOSV8.2.1/FreeRTOS
#FREERTOS_DIR = /home/karlp/src/FreeRTOSv9.0.0/FreeRTOS
FREERTOS_DIR = /home/karlp/src/FreeRTOSv10.2.1/FreeRTOS
FREERTOS_PORT = $(FREERTOS_DIR)/Source/portable/GCC/ARM_CM3
FREERTOS_INC = $(FREERTOS_DIR)/Source/include
FREERTOS_SRC = $(FREERTOS_DIR)/Source
FREERTOS_MMG = $(FREERTOS_DIR)/Source/portable/MemMang
FREERTOS_SRCS = list.c queue.c tasks.c timers.c port.c heap_1.c
FREERTOS_SRCS += FreeRTOS-openocd.c

VPATH += $(FREERTOS_SRC) $(FREERTOS_PORT) $(FREERTOS_MMG)

# Inclusion of header files
INCLUDES += $(patsubst %,-I%, . $(FREERTOS_INC) $(FREERTOS_PORT))

LDFLAGS += -Wl,--undefined=uxTopUsedPriority

CFILES = fmb.c
CFILES += $(FREERTOS_SRCS)

include ../rules.mk
