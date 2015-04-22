/*
 * FreeModbus+libopencm3+FreeRTOS demo
 * Consider to be BSD2 Clause, Apache 2.0, MIT, or ISC licensed, at your
 * pleasure.
 * karl Palsson <karlp@tweak.net.au>
 */

#include "FreeRTOS.h"
#include "task.h"
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

static void clock_setup(void)
{
	rcc_clock_setup_pll(&clock_config[CLOCK_VRANGE1_HSI_PLL_32MHZ]);
	/* setup systick at ms for FreeRTOS */
}

static void gpio_setup(void)
{
	/* blinken lights on disco board */
	rcc_periph_clock_enable(RCC_GPIOB);
	gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO6);
	gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO7);
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


/* Manage the blue led raw, green led from freertos */
extern void xPortSysTickHandler(void);
volatile uint64_t ksystick;
void sys_tick_handler(void)
{
	ksystick++;
	if (ksystick % 500 == 0) {
		gpio_toggle(GPIOB, GPIO6);
	}
	xPortSysTickHandler();
}

static void prvTaskGreenBlink1( void *pvParameters )
{
	(void)pvParameters;
        while (1)
        {
		vTaskDelay(portTICK_PERIOD_MS * 1000);
		gpio_toggle(GPIOB, GPIO7);
        }

        /* Tasks must not attempt to return from their implementing
        function or otherwise exit.  In newer FreeRTOS port
        attempting to do so will result in an configASSERT() being
        called if it is defined.  If it is necessary for a task to
        exit then have the task call vTaskDelete( NULL ) to ensure
        its exit is clean. */
        vTaskDelete( NULL );
    }


int main(void)
{
	clock_setup();
	gpio_setup();
	setup_systick();
	
	xTaskCreate(prvTaskGreenBlink1, "gblink", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
	
	vTaskStartScheduler();

	while(1) {
	}

	return 0;
}


void vAssertCalled( const char * const pcFileName, unsigned long ulLine )
{
volatile unsigned long ulSetToNonZeroInDebuggerToContinue = 0;

        /* Parameters are not used. */
        ( void ) ulLine;
        ( void ) pcFileName;

        taskENTER_CRITICAL();
        {
                while( ulSetToNonZeroInDebuggerToContinue == 0 )
                {
                        /* Use the debugger to set ulSetToNonZeroInDebuggerToContinue to a
                        non zero value to step out of this function to the point that raised
                        this assert(). */
                        __asm volatile( "NOP" );
                        __asm volatile( "NOP" );
                }
        }
        taskEXIT_CRITICAL();
}
