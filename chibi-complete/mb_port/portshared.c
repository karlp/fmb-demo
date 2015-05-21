/*
 * Extra methods shared by the port segments, related to critical sections
 * and interrupt hhndling
 */


#include "ch.h"
#include "mb.h"

static BOOL inside_isr = FALSE;
static BOOL inside_critical = FALSE;

void vMBPortSetISR(BOOL inside) {
	inside_isr = inside;
}

BOOL bMBPortIsInISR(void) {
	return inside_isr;
}

void vMBPortEnterCritical(void)
{
	/* No-one cares though.  
	 * freemodbus enters critical broadly before entire port init
	 * which then uses methods in chibios hal that try to lock.
	 * which consequently fail.
	 */
	inside_critical = true;
	// chSysLock();
}

void vMBPortExitCritical(void)
{
	inside_critical = false;
	// chSysUnlock();
}
