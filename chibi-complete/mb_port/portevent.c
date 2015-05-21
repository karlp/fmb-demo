/*
 * Implements the required functions for ChibiOS from mbport.h
 */
#include <stdbool.h>
#include <stdint.h>
#include "ch.h"
#include "mb.h"

static msg_t bufferQueue;
static Mailbox queue;

BOOL xMBPortEventInit(void)
{
	/* Make a ChibiOS Mailbox queue, only needs a single entry */
	chMBInit(&queue, &bufferQueue, 1);
	return TRUE;
}

BOOL xMBPortEventPost(eMBEventType eEvent)
{
	if (bMBPortIsInISR()) {
		chSysLockFromIsr();
		chMBPostI(&queue, (msg_t)eEvent);
		chSysUnlockFromIsr();
	} else {
		chMBPost(&queue, (msg_t)eEvent, 10);
	}
	return TRUE;
}

BOOL xMBPortEventGet(eMBEventType * eEvent)
{
	/* FIXME - this value feels too long... */
	BOOL had_event = FALSE;
	 // FIXME - what unit is ticks?
	if (chMBFetch(&queue, (msg_t *)eEvent, 50) == RDY_OK) {
		had_event = TRUE;
	}
	return had_event;
}