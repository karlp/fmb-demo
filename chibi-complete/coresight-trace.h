/*
 * Karl Palsson, <karlp@tweak.net.au> May 2015
 * Considered to be released under your choice of BSD2 clause, Apache2, MIT,
 * X11, or ISC licenses.
 */

#ifndef CORESIGHT_TRACE_H
#define	CORESIGHT_TRACE_H

#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

void trace_send_blocking8(int stimulus_port, char c);
void trace_send8(int stimulus_port, char val);

void trace_send_blocking16(int stimulus_port, uint16_t val);
void trace_send16(int stimulus_port, uint16_t val);

void trace_send_blocking32(int stimulus_port, uint32_t val);
void trace_send32(int stimulus_port, uint32_t val);

void gross_puts(int stimulus_port, const char *s);

#ifdef	__cplusplus
}
#endif

#endif	/* CORESIGHT_TRACE_H */

