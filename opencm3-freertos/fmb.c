/*
 * FreeModbus+libopencm3+FreeRTOS demo
 * Consider to be BSD2 Clause, Apache 2.0, MIT, or ISC licensed, at your
 * pleasure.
 * karl Palsson <karlp@tweak.net.au>
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

#include "syscfg.h"

#include <libopencm3/cm3/itm.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>


#define CONSOLE_USART USART3
#define CONSOLE_USART_RCC RCC_USART3

// the "console" isn't really setup properly for running on real hardware, we prefer trace there
#ifdef REAL_HARDWARE
#define raw_putc(x)	trace_send_blocking8(0, x)
#else
#define raw_putc(x)	usart_send_blocking(CONSOLE_USART, x)
#endif

void trace_send_blocking8(int stimulus_port, char c)
{
	if (!(ITM_TER[0] & (1<<stimulus_port))) {
		return;
	}
	while (!(ITM_STIM8(stimulus_port) & ITM_STIM_FIFOREADY));
	ITM_STIM8(stimulus_port) = c;
}


int _write(int file, char *ptr, int len)
{
        int i;

        if (file == STDOUT_FILENO || file == STDERR_FILENO) {
                for (i = 0; i < len; i++) {
                        if (ptr[i] == '\n') {
                                raw_putc('\r');
                        }
			raw_putc(ptr[i]);
                }
                return i;
        }
        errno = EIO;
        return -1;
}

static void hack_console_setup(void)
{
	rcc_periph_clock_enable(CONSOLE_USART_RCC);

	usart_set_baudrate(CONSOLE_USART, 115200);
//	usart_set_databits(USART_CONSOLE, 8);
//	usart_set_stopbits(USART_CONSOLE, USART_STOPBITS_1);
//	usart_set_mode(USART_CONSOLE, USART_MODE_TX);
//	usart_set_parity(USART_CONSOLE, USART_PARITY_NONE);
//	usart_set_flow_control(USART_CONSOLE, USART_FLOWCONTROL_NONE);

	/* Finally enable the USART. */
	usart_enable(CONSOLE_USART);
}


void test_uart_setup(unsigned ulBaudRate)
{

	/* port pins and peripherals */
	rcc_periph_clock_enable(MB_USART_RCC);
	rcc_periph_clock_enable(MB_USART_RCC_PORT);

	gpio_mode_setup(MB_USART_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, MB_USART_PINS);
	gpio_set_af(MB_USART_PORT, GPIO_AF7, MB_USART_PINS);

#if defined(MB_RS485_DE_PORT)
	/* fixme - assume port already enabled in rcc */
	gpio_mode_setup(MB_RS485_DE_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, MB_RS485_DE_PIN);
	gpio_clear(MB_RS485_DE_PORT, MB_RS485_DE_PIN);
#endif

	/* Setup UART parameters. */
	usart_set_baudrate(MB_USART, ulBaudRate);
	usart_set_flow_control(MB_USART, USART_FLOWCONTROL_NONE);
	usart_set_mode(MB_USART, USART_MODE_TX_RX);

	usart_set_parity(MB_USART, USART_PARITY_NONE);

	usart_disable_rx_interrupt(MB_USART);
	usart_disable_tx_interrupt(MB_USART);
	nvic_enable_irq(MB_USART_NVIC);
	usart_enable(MB_USART);
}


static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_clock_config[RCC_CLOCK_VRANGE1_HSI_PLL_32MHZ]);
}

static void gpio_setup(void)
{
	/* blinken lights */
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOA);
	gpio_mode_setup(LED_BLUE_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_BLUE_PIN);
#if defined(LED_GREEN_PORT)
	gpio_mode_setup(LED_GREEN_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_GREEN_PIN);
#endif
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


static QueueHandle_t rxq, txq;

void MB_USART_ISR(void)
{
	BaseType_t xHigherPriorityTaskWoken_rx = pdFALSE;
	BaseType_t xHigherPriorityTaskWoken_tx = pdFALSE;

	if (usart_get_flag(MB_USART, USART_SR_TXE)) {
		// get from txq if available, and write
		uint8_t c;
		if (xQueueReceiveFromISR(txq, &c, &xHigherPriorityTaskWoken_tx)) {
			usart_send(MB_USART, c);
		} else {
			usart_disable_tx_interrupt(MB_USART);
		}
	}
	if (usart_get_flag(MB_USART, USART_SR_RXNE)) {
		uint8_t c = usart_recv(MB_USART); /* 9bit wat? */
		xQueueSendToBackFromISR(rxq, &c, &xHigherPriorityTaskWoken_rx);
	}
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken_rx || xHigherPriorityTaskWoken_tx);
}

static void prvTaskRenodeEcho(void *pvParameters)
{
	(void)pvParameters;

	rxq = xQueueCreate(16, 1);
	txq = xQueueCreate(16, 1);

	// reuse the standard modbus port init
	test_uart_setup(115200);
	usart_enable_rx_interrupt(MB_USART);

	while (1) {
		uint8_t c;
		if (xQueueReceive(rxq, &c, portMAX_DELAY) == pdPASS) {
			c++;
			printf("pushing rx to tx: %x\n", c);

			if (xQueueSendToBack(txq, &c, 50)) {
				printf("enable txe\n");
				usart_enable_tx_interrupt(MB_USART);
			} else {
				printf("txqfull ?!\n");
			}
		} else {
			printf("Wot? blocking read failed?!\n");
		}
	}
}

int main(void)
{
	clock_setup();
	gpio_setup();
	hack_console_setup();
	printf("starting up\n");

	// Make sure we're above the freertos split
#define IRQ2NVIC_PRIOR(x)	((x)<<4)
        nvic_set_priority(MB_USART_NVIC, IRQ2NVIC_PRIOR(6));
        nvic_set_priority(MB_TIMER_NVIC, IRQ2NVIC_PRIOR(7));

#if defined (LED_GREEN_PORT)
	xTaskCreate(prvTaskGreenBlink1, "green.blink", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
#endif
	xTaskCreate(prvTaskRenodeEcho, "renode", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
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
