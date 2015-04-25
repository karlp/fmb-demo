/*
 * Implements the required functions for FreeModbus from mbport.h
 * This implements the event portion, and is based on the BARE demo
 * but is unfortunately not included verbatim due to licensing
 * incompatibilities between the library core and the demo sample.
 */
#include <stdbool.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "mb.h"
#include "queue.h"
#include "timers.h"

static QueueHandle_t queue;

BOOL xMBPortEventInit(void)
{
	/* Make a FreeRTOS queue, only needs a single entry */
	queue = xQueueCreate( 1, sizeof( eMBEventType ) );
	if (queue) {
		return TRUE;
	} else {
		return FALSE;
	}
}

BOOL xMBPortEventPost(eMBEventType eEvent)
{
	if (bMBPortIsInISR()) {
		xQueueSendFromISR(queue, &eEvent, pdFALSE);
	} else {
		xQueueSend(queue, &eEvent, pdFALSE);
	}
	return TRUE;
}

BOOL xMBPortEventGet(eMBEventType * eEvent)
{
	/* FIXME - this value feels too long... */
	BOOL had_event = FALSE;
	if (pdTRUE == xQueueReceive(queue, eEvent, portTICK_PERIOD_MS * 50)) {
		had_event = TRUE;
	}
	return had_event;
}