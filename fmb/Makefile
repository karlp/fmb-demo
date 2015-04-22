
BINARY = fmb

CFLAGS += -std=c99

OPENCM3_DIR=../libopencm3
LDSCRIPT = $(OPENCM3_DIR)/lib/stm32/l1/stm32l15xxb.ld

FR_DIR:=/home/karlp/src/FreeRTOSV8.2.1/FreeRTOS/Source
FR_PORT_DIR:=$(FR_DIR)/portable/GCC/ARM_CM3
FR_MEM_DIR:=$(FR_DIR)/portable/MemMang
FR_INC:=$(FR_DIR)/include
FR_PORT_INC:=$(FR_PORT_DIR)
FR_OBJS= list.o queue.o tasks.o port.o heap_1.c

CFLAGS+= -I$(FR_INC) -I$(FR_PORT_INC)

VPATH+= $(FR_DIR) $(FR_PORT_DIR) $(FR_MEM_DIR)

OBJS+=$(FR_OBJS)

include ../libopencm3.target.mk

