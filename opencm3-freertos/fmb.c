/*
 * FreeModbus+libopencm3+FreeRTOS demo
 * Consider to be BSD2 Clause, Apache 2.0, MIT, or ISC licensed, at your
 * pleasure.
 * karl Palsson <karlp@tweak.net.au>
 */

#include <assert.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "mb.h"
#include "syscfg.h"

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#define REG_BASE	0x2000
static USHORT table[10];

static void clock_setup(void)
{
	rcc_clock_setup_pll(&clock_config[CLOCK_VRANGE1_HSI_PLL_32MHZ]);
	/* setup systick at ms for FreeRTOS */
}

static void gpio_setup(void)
{
	/* blinken lights */
	rcc_periph_clock_enable(RCC_GPIOB);
	gpio_mode_setup(LED_BLUE_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_BLUE_PIN);
#if defined(LED_GREEN_PORT)
	gpio_mode_setup(LED_GREEN_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_GREEN_PIN);
#endif
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


extern void xPortSysTickHandler(void);

void sys_tick_handler(void)
{
	xPortSysTickHandler();
}

static void prvTimerBlue(TimerHandle_t xTimer)
{
	(void) xTimer;
	gpio_toggle(LED_BLUE_PORT, LED_BLUE_PIN);
}

#if defined (LED_GREEN_PORT)
static void prvTaskGreenBlink1(void *pvParameters)
{
	(void) pvParameters;
	while (1) {
		vTaskDelay(portTICK_PERIOD_MS * 1000);
		gpio_toggle(LED_GREEN_PORT, LED_GREEN_PIN);
	}

	/* Tasks must not attempt to return from their implementing
	function or otherwise exit.  In newer FreeRTOS port
	attempting to do so will result in an configASSERT() being
	called if it is defined.  If it is necessary for a task to
	exit then have the task call vTaskDelete( NULL ) to ensure
	its exit is clean. */
	vTaskDelete(NULL);
}
#endif

static TimerHandle_t xBlueTimer;

static void prvTaskModbus(void *pvParameters)
{
	(void) pvParameters;
	while (1) {
		eMBErrorCode eStatus;
		eStatus = eMBInit(MB_RTU, 0x0A, 1, 19200, MB_PAR_EVEN);
		assert(eStatus == MB_ENOERR);

		const char *report_data = "karlwashere";
		eStatus = eMBSetSlaveID(0x34, TRUE, (UCHAR *) report_data, strlen(report_data));
		assert(eStatus == MB_ENOERR);

		eStatus = eMBEnable();
		assert(eStatus == MB_ENOERR);
		/* TODO - either exit the task, or let the task restart and try
		 * and fix itself if these asserts failed */

		while (1) {
			eMBPoll();
			/* TODO - should I yield here? */
		}
	}
}

int main(void)
{
	clock_setup();
	gpio_setup();
	setup_systick();
	table[1] = 0xcafe;
	table[9] = 0xdead;
	scb_set_priority_grouping(SCB_AIRCR_PRIGROUP_GROUP16_NOSUB);

	// FIXME - this works, but what priority is what really?!
#define IRQ2NVIC_PRIOR(x)	((x)<<4)
        nvic_set_priority(NVIC_SYSTICK_IRQ, IRQ2NVIC_PRIOR(1));
        nvic_set_priority(MB_USART_NVIC, IRQ2NVIC_PRIOR(6));
        nvic_set_priority(MB_TIMER_NVIC, IRQ2NVIC_PRIOR(7));


#if defined (LED_GREEN_PORT)
	xTaskCreate(prvTaskGreenBlink1, "green.blink", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
#endif
	xTaskCreate(prvTaskModbus, "modbus", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
	xBlueTimer = xTimerCreate("blue.blink", 200 * portTICK_PERIOD_MS, true, 0, prvTimerBlue);
	if (xBlueTimer) {
		if (xTimerStart(xBlueTimer, 0) != pdTRUE) {

		}
	} else {
		/* FIXME - trace here please */
	}

	vTaskStartScheduler();

	return 0;
}

void vAssertCalled(const char * const pcFileName, unsigned long ulLine)
{
	volatile unsigned long ulSetToNonZeroInDebuggerToContinue = 0;

	/* Parameters are not used. */
	(void) ulLine;
	(void) pcFileName;

	taskENTER_CRITICAL();
	{
		while (ulSetToNonZeroInDebuggerToContinue == 0) {
			/* Use the debugger to set ulSetToNonZeroInDebuggerToContinue to a
			non zero value to step out of this function to the point that raised
			this assert(). */
			__asm volatile( "NOP");
			__asm volatile( "NOP");
		}
	}
	taskEXIT_CRITICAL();
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
	/*
	 * FreeModbus has a hard++, converting address to reg#, but calling it address
	 * We prefer raw addresses please, 0 based.
	 */
	usAddress--;
	eMBErrorCode status = MB_ENOERR;
	table[0]++;

	if ((usAddress >= REG_BASE)
		&& (usAddress + usNRegs <= REG_BASE + (sizeof(table) / sizeof(table[0])))) {
		int idx = usAddress - REG_BASE;
		if (eMode == MB_REG_READ) {
			for (int i = 0; i < usNRegs; i++) {
				*pucRegBuffer++ = (UCHAR) (table[idx+i] >> 8);
				*pucRegBuffer++ = (UCHAR) (table[idx+i] & 0xFF);
			}
		} else if (eMode == MB_REG_WRITE) {
			for (int i = 0; i < usNRegs; i++) {
				table[idx+i] = *pucRegBuffer++ << 8;
				table[idx+i] |= *pucRegBuffer++;
			}
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
