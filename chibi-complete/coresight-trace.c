/*
 * Karl Palsson, <karlp@tweak.net.au> May 2015
 * Considered to be released under your choice of BSD2 clause, Apache2, MIT,
 * X11, or ISC licenses.
 * 
 * Provides raw coresight ITM trace output routines without depending on any
 * external operating systems, HAL layers or libraries or such forth.
 * 
 * Does not, as is right and true, attempt to configure any of the output
 * mechanisms.  That's the job of your debugger tools.  Tracepoints are meant to
 * be non-intrusive remember!
 */

#include "coresight-trace.h"

/* Those defined only on ARMv7 and above */
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

#define IO32(kolkrabbi)		(*(volatile uint32_t *)(kolkrabbi))
#define IO16(bleikja)		(*(volatile uint16_t *)(bleikja))
#define IO8(ysa)		(*(volatile uint8_t *)(ysa))

#define ITM_BASE		0xE0000000
#define ITM_STIM8(ze_byte)	(IO8(ITM_BASE + ((ze_byte)*4)))
#define ITM_STIM16(ze_short)	(IO16(ITM_BASE + ((ze_short)*4)))
#define ITM_STIM32(ze_hole)	(IO32(ITM_BASE + ((ze_hole)*4)))
#define ITM_STIM_FIFOREADY	1

#define ITM_TER			(&IO32(ITM_BASE + 0xE00))

void trace_send_blocking8(int stimulus_port, char c) {
	if (!(ITM_TER[0] & (1<<stimulus_port))) {
		return;
	}
	while (!(ITM_STIM8(stimulus_port) & ITM_STIM_FIFOREADY))
		;
	ITM_STIM8(stimulus_port) = c;
}

void trace_send8(int stimulus_port, char val) {
	if (!(ITM_TER[0] & (1<<stimulus_port))) {
		return;
	}
	ITM_STIM8(stimulus_port) = val;
}

void trace_send_blocking16(int stimulus_port, uint16_t val) {
	if (!(ITM_TER[0] & (1<<stimulus_port))) {
		return;
	}
	while (!(ITM_STIM16(stimulus_port) & ITM_STIM_FIFOREADY))
		;
	ITM_STIM16(stimulus_port) = val;
}

void trace_send16(int stimulus_port, uint16_t val) {
	if (!(ITM_TER[0] & (1<<stimulus_port))) {
		return;
	}
	ITM_STIM16(stimulus_port) = val;
}


void trace_send_blocking32(int stimulus_port, uint32_t val) {
	if (!(ITM_TER[0] & (1<<stimulus_port))) {
		return;
	}
	while (!(ITM_STIM32(stimulus_port) & ITM_STIM_FIFOREADY))
		;
	ITM_STIM32(stimulus_port) = val;
}

void trace_send32(int stimulus_port, uint32_t val) {
	if (!(ITM_TER[0] & (1<<stimulus_port))) {
		return;
	}
	ITM_STIM32(stimulus_port) = val;
}

void gross_puts(int stimulus_port, const char *s) {
	while (*s != '\0') {
		if (*s == '\n') {
			trace_send_blocking8(stimulus_port, '\r');
		}
		trace_send_blocking8(stimulus_port, *s++);
	}
}

#else
#error "Instrumentation Trace Macrocell only available on armv7(e)m"
#endif