/*
 * FreeModbus+libopencm3+FreeRTOS demo
 * Consider to be BSD2 Clause, Apache 2.0, MIT, or ISC licensed, at your
 * pleasure.
 * karl Palsson <karlp@tweak.net.au>
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

static void gpio_setup(void)
{
	/* blinken lights on disco board */
	rcc_periph_clock_enable(RCC_GPIOB);
	gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO6);
}

int main(void)
{
	gpio_setup();
	while (1) {
		gpio_toggle(GPIOB, GPIO6);
		for (int i = 0; i < 1000000; i++) {
			__asm__("nop");
		}
	}

	return 0;
}
