/*
 * Extra methods shared by the port segments, related to critical sections
 * and interrupt hhndling
 */

#include "mb.h"
#include <libopencm3/cm3/cortex.h>

/* These are empty for now, but would presumably need work with an RTOS */
void vMBPortEnterCritical(void)
{
	cm_disable_interrupts();
}

void vMBPortExitCritical(void)
{
	cm_enable_interrupts();
}
