/*
 * FreeModbus+ChibiOS demo
 * Consider to be BSD2 Clause, Apache 2.0, MIT, or ISC licensed, at your
 * pleasure.
 * karl Palsson <karlp@tweak.net.au>
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"

#include "mb.h"
#include "syscfg.h"
#include "coresight-trace.h"

#define REG_BASE	0x2000
static USHORT table[10];

/**
 * Blue LED blinking task
 */
static WORKING_AREA(waThreadBlinkBlue, 128);
__attribute__((noreturn))
static msg_t ThreadBlinkBlue(void *arg) {

  (void)arg;
  chRegSetThreadName("blink.blue");
	palSetPadMode(LED_BLUE_PORT, LED_BLUE_PIN,
		PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
	
  while (1) {
	palTogglePad(LED_BLUE_PORT, LED_BLUE_PIN);
	chThdSleepMilliseconds(200);
  }
}

#if defined (LED_GREEN_PORT)
static WORKING_AREA(waThreadBlinkGreen, 128);
__attribute__((noreturn))
static msg_t ThreadBlinkGreen(void *arg) {
  (void)arg;
  chRegSetThreadName("blink.green");
	palSetPadMode(LED_GREEN_PORT, LED_GREEN_PIN,
		PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
	
  while (1) {
	palTogglePad(LED_GREEN_PORT, LED_GREEN_PIN);
	chThdSleepMilliseconds(1000);
  }
}

#endif

#ifdef ALTERNATIVE_TICKER_TASK
static void gpt_blink_green(GPTDriver *gptp) {
	(void)gptp;
	palTogglePad(LED_GREEN_PORT, LED_GREEN_PIN);
}
#endif



static WORKING_AREA(waThreadModbus, 128);
__attribute__((noreturn))
static msg_t ThreadModbus(void *arg) {
	(void)arg;
	eMBErrorCode eStatus;
	eStatus = eMBInit(MB_RTU, 0x0A, 1, 19200, MB_PAR_EVEN);
	chDbgAssert(eStatus == MB_ENOERR, "modbus init failed", "Shouldn't happen");

	const char *report_data = "karlwashere";
	eStatus = eMBSetSlaveID(0x34, TRUE, (UCHAR *) report_data, strlen(report_data));
	chDbgAssert(eStatus == MB_ENOERR, "modbus extra data too big", "fix your mbconf.h");
	assert(eStatus == MB_ENOERR);

	eStatus = eMBEnable();
	chDbgAssert(eStatus == MB_ENOERR, "Double enable called?", "Shouldn't happen");

	while (1) {
		eMBPoll();
		/* TODO - should I yield here? */
	}
}


int main(void)
{
/*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
	halInit();
	chSysInit();

	table[1] = 0xcafe;
	table[9] = 0xdead;

	/*
   * Creates the example thread.
   */
  chThdCreateStatic(waThreadBlinkBlue, sizeof(waThreadBlinkBlue), NORMALPRIO, ThreadBlinkBlue, NULL);
  chThdCreateStatic(waThreadBlinkGreen, sizeof(waThreadBlinkGreen), NORMALPRIO, ThreadBlinkGreen, NULL);
  
#ifdef ALTERNATIVE_TICKER_TASK
	/* Use a gpt timer instead of a thread */
	palSetPadMode(LED_GREEN_PORT, LED_GREEN_PIN,
		PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
  	GPTConfig config;
	config.callback = gpt_blink_green;
	config.frequency = 20000;
	
	gptStart(&GPTD3, &config);
	gptStartContinuous(&GPTD3, 200);  // 1 Hz?
#endif

  chThdCreateStatic(waThreadModbus, sizeof(waThreadModbus), NORMALPRIO, ThreadModbus, NULL);
	
	/* nothing in main idle */
	while (1) {
		chThdSleepMilliseconds(500);
	}

	return 0;
}

eMBErrorCode eMBRegInputCB(UCHAR * pucRegBuffer, USHORT usAddress,
	USHORT usNRegs)
{
	(void)pucRegBuffer;
	(void)usAddress;
	(void)usNRegs;
	return MB_ENOREG;
}

eMBErrorCode eMBRegHoldingCB(UCHAR * pucRegBuffer, USHORT usAddress,
	USHORT usNRegs, eMBRegisterMode eMode)
{
	/*
	 * FreeModbus has a hard++, converting address to reg#, but calling it address
	 * We prefer raw addresses please, 0 based.
	 */
	usAddress--;
	eMBErrorCode status = MB_ENOERR;
	table[0]++;

	if ((usAddress >= REG_BASE)
		&& (usAddress + usNRegs <= REG_BASE + (sizeof(table) / sizeof(table[0])))) {
		int idx = usAddress - REG_BASE;
		if (eMode == MB_REG_READ) {
			gross_puts(TRACE_CONSOLE, "reg_read");
			for (int i = 0; i < usNRegs; i++) {
				*pucRegBuffer++ = (UCHAR) (table[idx+i] >> 8);
				*pucRegBuffer++ = (UCHAR) (table[idx+i] & 0xFF);
			}
		} else if (eMode == MB_REG_WRITE) {
			gross_puts(TRACE_CONSOLE, "reg_write");
			for (int i = 0; i < usNRegs; i++) {
				table[idx+i] = *pucRegBuffer++ << 8;
				table[idx+i] |= *pucRegBuffer++;
			}
		}
	} else {
		status = MB_ENOREG;
	}

	return status;

}

eMBErrorCode eMBRegCoilsCB(UCHAR * pucRegBuffer, USHORT usAddress,
	USHORT usNCoils, eMBRegisterMode eMode)
{
	(void)pucRegBuffer;
	(void)usAddress;
	(void)usNCoils;
	(void)eMode;
	return MB_ENOREG;
}

eMBErrorCode eMBRegDiscreteCB(UCHAR * pucRegBuffer, USHORT usAddress,
	USHORT usNDiscrete)
{
	(void)pucRegBuffer;
	(void)usAddress;
	(void)usNDiscrete;
	return MB_ENOREG;
}
