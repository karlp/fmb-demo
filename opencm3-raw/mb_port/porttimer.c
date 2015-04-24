/*
 * Implements the required functions for FreeModbus from mbport.h
 * This implements the timers portion
 */
#include <stdlib.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

#include "mb.h"

#include "syscfg.h"

BOOL xMBPortTimersInit(USHORT usTimeOut50us)
{
	/* Pick a timer, any timer, needs to be capable of counting
	 to 20000 if you want ASCII mode.  Thankfully we're not a system
	 with only 8 bit timers ;) */
	rcc_periph_clock_enable(MB_TIMER_RCC);
	nvic_enable_irq(MB_TIMER_NVIC);
	timer_reset(MB_TIMER);
	TIM_CNT(MB_TIMER) = 0;
	/* from docs, needs a 20kHz counter */
	timer_set_prescaler(MB_TIMER, 1599); // 32MHz/20kHz - 1
	timer_set_period(MB_TIMER, usTimeOut50us);
	return TRUE;
}

void xMBPortTimersClose(void)
{
	nvic_disable_irq(MB_TIMER_NVIC);
	rcc_periph_clock_disable(MB_TIMER_RCC);
}

void vMBPortTimersEnable(void)
{
	timer_set_counter(MB_TIMER, 0);
	timer_enable_irq(MB_TIMER, TIM_DIER_UIE);
	timer_enable_counter(MB_TIMER);
}

void vMBPortTimersDisable(void)
{
	timer_disable_irq(MB_TIMER, TIM_DIER_UIE);
	timer_disable_counter(MB_TIMER);
}

/* ------------- end of required functions ----------- */

void MB_TIMER_ISR(void)
{
	if (timer_get_flag(MB_TIMER, TIM_SR_UIF)) {
		timer_clear_flag(MB_TIMER, TIM_SR_UIF);
		if (pxMBPortCBTimerExpired()) {
			/* TODO rtos signal ctx switch here */
		}
	}
}