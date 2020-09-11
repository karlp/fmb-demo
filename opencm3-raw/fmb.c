/*
 * FreeModbus+libopencm3 demo
 * Consider to be BSD2 Clause, Apache 2.0, MIT, or ISC licensed, at your
 * pleasure.
 * karl Palsson <karlp@tweak.net.au>
 */

#include <assert.h>
#include <string.h>

#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "mb.h"
#include "syscfg.h"

#define REG_BASE	0x2000
static USHORT table[10];

static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_clock_config[RCC_CLOCK_VRANGE1_HSI_PLL_32MHZ]);
}

static void gpio_setup(void)
{
	/* blinken lights */
	rcc_periph_clock_enable(RCC_GPIOA);
	gpio_mode_setup(LED_BLUE_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_BLUE_PIN);
}

/**
 * Configure the systick timer for 1 msec
 */
static void setup_systick(void)
{
	/* 32MHz / 8 => 4000000 counts per second. */
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
	/* 4000000/4000 = 1000 overflows per second - every 1ms one interrupt */
	systick_set_reload(3999);
	systick_interrupt_enable();
	systick_counter_enable();
}

/* Just a ticker for led fun */
volatile uint64_t ksystick;

void sys_tick_handler(void)
{
	ksystick++;
	if (ksystick % 500 == 0) {
		gpio_toggle(LED_BLUE_PORT, LED_BLUE_PIN);
	}
}

int main(void)
{
	clock_setup();
	gpio_setup();
	setup_systick();
	table[1] = 0xcafe;
	table[9] = 0xdead;

	eMBErrorCode eStatus;
	eStatus = eMBInit(MB_RTU, 0x0A, 1, 19200, MB_PAR_EVEN);
	assert(eStatus == MB_ENOERR);

	const char *report_data = "karlwashere";
	eStatus = eMBSetSlaveID(0x34, TRUE, (UCHAR *) report_data, strlen(report_data));
	assert(eStatus == MB_ENOERR);

	eStatus = eMBEnable();
	assert(eStatus == MB_ENOERR);

	while (1) {
		eMBPoll();
	}

	return 0;
}

eMBErrorCode eMBRegInputCB(UCHAR * pucRegBuffer, USHORT usAddress,
	USHORT usNRegs)
{
	(void)pucRegBuffer;
	(void)usAddress;
	(void)usNRegs;
	return MB_ENOREG;
}

eMBErrorCode eMBRegHoldingCB(UCHAR * pucRegBuffer, USHORT usAddress,
	USHORT usNRegs, eMBRegisterMode eMode)
{
	usAddress--; /* register to address conversion */
	eMBErrorCode status = MB_ENOERR;
	table[0]++;
	if (eMode == MB_REG_WRITE) {
		/* For now... */
		return MB_EINVAL;
	}

	if ((usAddress >= REG_BASE)
		&& (usAddress + usNRegs <= REG_BASE + (sizeof(table) / sizeof(table[0])))) {
		for (int i = 0; i < usNRegs; i++) {
			*pucRegBuffer++ = (UCHAR) (table[i] >> 8);
			*pucRegBuffer++ = (UCHAR) (table[i] & 0xFF);
		}
	} else {
		status = MB_ENOREG;
	}

	return status;

}

eMBErrorCode eMBRegCoilsCB(UCHAR * pucRegBuffer, USHORT usAddress,
	USHORT usNCoils, eMBRegisterMode eMode)
{
	(void)pucRegBuffer;
	(void)usAddress;
	(void)usNCoils;
	(void)eMode;
	return MB_ENOREG;
}

eMBErrorCode eMBRegDiscreteCB(UCHAR * pucRegBuffer, USHORT usAddress,
	USHORT usNDiscrete)
{
	(void)pucRegBuffer;
	(void)usAddress;
	(void)usNDiscrete;
	return MB_ENOREG;
}
