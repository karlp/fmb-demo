/*
 * Implements the required functions for FreeModbus from mbport.h
 * This implements the serial portion
 */
#include <stdlib.h>


#include "ch.h"
#include "hal.h"
#include "mb.h"

#include "syscfg.h"
#include "coresight-trace.h"

static char the_char;

void handle_txend1(UARTDriver *uartp) {
	(void)uartp;
	vMBPortSetISR(TRUE);
	pxMBFrameCBTransmitterEmpty();
	vMBPortSetISR(FALSE);
}

void handle_txend2(UARTDriver *uartp) {
	(void)uartp;
	vMBPortSetISR(TRUE);
	//MB_USART.usart->CR1 &= ~USART_CR1_TCIE;	
#if defined(MB_RS485_DE_PORT)
	palClearPad(MB_RS485_DE_PORT, MB_RS485_DE_PIN);
#endif
	vMBPortSetISR(FALSE);
}

void handle_rxchar(UARTDriver *uartp, uint16_t c) {
	(void)uartp;
	vMBPortSetISR(TRUE);
	the_char = c;
	trace_send_blocking8(TRACE_MODBUS_RX, c);
	pxMBFrameCBByteReceived();
	vMBPortSetISR(FALSE);
}

BOOL
xMBPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity)
{
	/* FIXME - would be sexy to support this somehow, but not required */
	(void) ucPORT;
	/* TODO only support and expect 8 databits here */
	(void) ucDataBits;

	static UARTConfig config = {
		.txend1_cb = handle_txend1,
		.txend2_cb = handle_txend2,
		.rxchar_cb = handle_rxchar,
	};
	config.speed = ulBaudRate;
	
	palSetPadMode(MB_USART_PORT, MB_USART_PIN_TX, PAL_MODE_ALTERNATE(7));
	palSetPadMode(MB_USART_PORT, MB_USART_PIN_RX, PAL_MODE_ALTERNATE(7));

#if defined(MB_RS485_DE_PORT)
	palSetPadMode(MB_RS485_DE_PORT, MB_RS485_DE_PIN,
		PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);)
	palClearPad(MB_RS485_DE_PORT, MB_RS485_DE_PIN);
#endif

	/* Only support 1 stop bit here, even though "none" should use 2 */
	/* Observe that ST has a different idea of counting databits */
	/* 9 bits only if parity is set, or stop bits > 1 */
	config.cr2 |= USART_CR2_STOP_1;
	if (eParity == MB_PAR_NONE) {
		;
	} else {
		config.cr1 |= USART_CR1_M;
	}

	switch (eParity) {
	case MB_PAR_NONE:
		break;
	case MB_PAR_ODD:
		config.cr1 |= (USART_CR1_PCE|USART_CR1_PS);
		break;
	default:
	case MB_PAR_EVEN:
		config.cr1 |= (USART_CR1_PCE);
		break;
	}

	uartStart(&MB_USART, &config);
	return true;
}

#define DO_YOU_WANT_PORT_LEVEL_CLOSE 0
#if DO_YOU_WANT_PORT_LEVEL_CLOSE

/* note that this conflicts with being able to open multiple uarts
 This is not required in freemodbus */
void
vMBPortClose(void)
{
	uartStop(&MB_USARRT;
}

#endif

/**
 * Probably want to do some things here with rs485 too...
 * @param xRxEnable
 * @param xTxEnable
 */
void
vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
	if (xRxEnable) {
		gross_puts(TRACE_CONSOLE, "RXE\n");
		MB_USART.usart->CR1 |= USART_CR1_RXNEIE;
	} else {
		gross_puts(TRACE_CONSOLE, "rxd\n");
		MB_USART.usart->CR1 &= ~USART_CR1_RXNEIE;
	}

	if (xTxEnable) {
		gross_puts(TRACE_CONSOLE, "TXE\n");
#if defined(MB_RS485_DE_PORT)
		palSetPad(MB_RS485_DE_PORT, MB_RS485_DE_PIN);
#endif
		MB_USART.usart->CR1 |= USART_CR1_TXEIE;
		//usart_enable_tx_interrupt(MB_USART);
	} else {
		gross_puts(TRACE_CONSOLE, "txd\n");
		MB_USART.usart->CR1 &= ~USART_CR1_TXEIE;
		//usart_disable_tx_interrupt(MB_USART);
		/* Enable TC so we know when to turn off the rs485 driver */
		//USART_CR1(MB_USART) |= USART_CR1_TCIE;
		MB_USART.usart->CR1 |= USART_CR1_TCIE;
	}
}

BOOL
xMBPortSerialGetByte(CHAR * pucByte)
{
	*pucByte = (CHAR) the_char;
	return TRUE;
}

BOOL
xMBPortSerialPutByte(CHAR ucByte)
{
	gross_puts(TRACE_CONSOLE, "P");
	if (bMBPortIsInISR()) {
		uartStartSendI(&MB_USART, 1, &ucByte);
	} else {
		uartStartSend (&MB_USART, 1, &ucByte);
	}
	return TRUE;
}
