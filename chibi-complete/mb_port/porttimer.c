/*
 * Implements the required functions for FreeModbus from mbport.h
 * This implements the timers portion
 */
#include <stdlib.h>

#include "ch.h"
#include "hal.h"
#include "mb.h"

#include "syscfg.h"

static VirtualTimer timer;
static int ticks_required;

static void timer_callback(void *param) {
	(void)param;
	vMBPortSetISR(TRUE);
	pxMBPortCBTimerExpired();
	vMBPortSetISR(FALSE);
}

BOOL xMBPortTimersInit(USHORT usTimeOut50us)
{
	/* Timer should be able to work in 50uS steps, ideally... */
	/* Pick a timer, any timer, needs to be capable of counting
	 to 20000 if you want ASCII mode.  Thankfully we're not a system
	 with only 8 bit timers ;) */
	ticks_required = US2ST(usTimeOut50us * 50);
	
	return TRUE;
}

void xMBPortTimersClose(void)
{
}

void vMBPortTimersEnable(void)
{
	/* might need more from isr shit */
	if (bMBPortIsInISR()) {
		chSysLockFromIsr();
		if (chVTIsArmedI(&timer)) {
			chVTResetI(&timer);
		}
		chVTSetI(&timer, ticks_required, timer_callback, NULL);
		chSysUnlockFromIsr();
	} else {
		chVTReset(&timer);
		chVTSet(&timer, ticks_required, timer_callback, NULL);
	}
}

void vMBPortTimersDisable(void)
{
	chSysLockFromIsr();
	if (chVTIsArmedI(&timer)) {
		chVTResetI(&timer);
	}
	chSysUnlockFromIsr();
}
