/*
 * Implements the required functions for FreeModbus from mbport.h
 * This implements the event portion, and is based on the BARE demo
 * but is unfortunately not included verbatim due to licensing
 * incompatibilities between the library core and the demo sample.
 */
#include <stdbool.h>
#include <stdint.h>
#include <libopencm3/cm3/cortex.h>
#include "mb.h"

static bool has_entry;
static eMBEventType entry;

BOOL xMBPortEventInit(void)
{
	has_entry = false;
	return TRUE;
}

BOOL xMBPortEventPost(eMBEventType eEvent)
{
	if (has_entry) {
		return FALSE;
	}
	entry = eEvent;
	has_entry = true;
	return TRUE;
}

BOOL xMBPortEventGet(eMBEventType * eEvent)
{
	BOOL had_event = FALSE;
	if (has_entry) {
		*eEvent = entry;
		has_entry = false;
		had_event = TRUE;
	}
	return had_event;
}

/* These are empty for now, but would presumably need work with an RTOS */
void vMBPortEnterCritical(void)
{
	cm_disable_interrupts();
}

void vMBPortExitCritical(void)
{
	cm_enable_interrupts();
}
