/*
 * Extra methods shared by the port segments, related to critical sections
 * and interrupt hhndling
 */

#include "FreeRTOS.h"
#include "task.h"

#include "mb.h"

static BOOL inside_isr = FALSE;

void vMBPortEnterCritical(void)
{
	taskENTER_CRITICAL();
}

void vMBPortExitCritical(void)
{
	taskEXIT_CRITICAL();
}

void vMBPortSetISR(BOOL inside) {
	inside_isr = inside;
}

BOOL bMBPortIsInISR(void) {
	return inside_isr;
}

