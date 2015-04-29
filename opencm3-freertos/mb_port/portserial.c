/*
 * Implements the required functions for FreeModbus from mbport.h
 * This implements the serial portion
 */
#include <stdlib.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>

#include "FreeRTOS.h"

#include "mb.h"

#include "syscfg.h"

BOOL
xMBPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity)
{
	/* FIXME - would be sexy to support this somehow, but not required */
	(void) ucPORT;
	/* TODO only support and expect 8 databits here */
	(void) ucDataBits;

	/* port pins and peripherals */
	rcc_periph_clock_enable(MB_USART_RCC);
	rcc_periph_clock_enable(MB_USART_RCC_PORT);

	gpio_mode_setup(MB_USART_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, MB_USART_PINS);
	gpio_set_af(MB_USART_PORT, GPIO_AF7, MB_USART_PINS);

#if defined(MB_RS485_DE_PORT)
	/* fixme - assume port already enabled in rcc */
	gpio_mode_setup(MB_RS485_DE_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, MB_RS485_DE_PIN);
	gpio_clear(MB_RS485_DE_PORT, MB_RS485_DE_PIN);
#endif

	/* Setup UART parameters. */
	usart_set_baudrate(MB_USART, ulBaudRate);
	usart_set_flow_control(MB_USART, USART_FLOWCONTROL_NONE);
	usart_set_mode(MB_USART, USART_MODE_TX_RX);

	/* Only support 1 stop bit here, even though "none" should use 2 */
	/* Observe that ST has a different idea of counting databits */
	/* 9 bits only if parity is set, or stop bits > 1 */
	usart_set_stopbits(MB_USART, USART_STOPBITS_1);
	if (eParity == MB_PAR_NONE) {
		usart_set_databits(MB_USART, 8);
	} else {
		usart_set_databits(MB_USART, 9);
	}

	switch (eParity) {
	case MB_PAR_NONE:
		usart_set_parity(MB_USART, USART_PARITY_NONE);
		break;
	case MB_PAR_ODD:
		usart_set_parity(MB_USART, USART_PARITY_ODD);
		break;
	default:
	case MB_PAR_EVEN:
		usart_set_parity(MB_USART, USART_PARITY_EVEN);
		break;
	}

	usart_disable_rx_interrupt(MB_USART);
	usart_disable_tx_interrupt(MB_USART);
	nvic_enable_irq(MB_USART_NVIC);
	usart_enable(MB_USART);
	return true;
}

#define DO_YOU_WANT_PORT_LEVEL_CLOSE 1
#if DO_YOU_WANT_PORT_LEVEL_CLOSE

/* note that this conflicts with being able to open multiple uarts
 This is not required in freemodbus */
void
vMBPortClose(void)
{
	nvic_disable_irq(MB_USART_NVIC);
	usart_disable(MB_USART);
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
		usart_enable_rx_interrupt(MB_USART);
	} else {
		usart_disable_rx_interrupt(MB_USART);
	}

	if (xTxEnable) {
#if defined(MB_RS485_DE_PORT)
		gpio_set(MB_RS485_DE_PORT, MB_RS485_DE_PIN);
#endif
		usart_enable_tx_interrupt(MB_USART);
	} else {
		usart_disable_tx_interrupt(MB_USART);
		/* Enable TC so we know when to turn off the rs485 driver */
		USART_CR1(MB_USART) |= USART_CR1_TCIE;
	}
}

BOOL
xMBPortSerialGetByte(CHAR * pucByte)
{
	*pucByte = (CHAR) usart_recv(MB_USART);
	return TRUE;
}

BOOL
xMBPortSerialPutByte(CHAR ucByte)
{
	usart_send(MB_USART, ucByte);
	return TRUE;
}

/* ----------- End of required functions ------------ */

/**
 * STM32 uses one irq for both rx and tx
 */
void MB_USART_ISR(void)
{
	vMBPortSetISR(TRUE);
	int tasks_ready = 0;
	if (usart_get_interrupt_source(MB_USART, USART_SR_RXNE)) {
		if (pxMBFrameCBByteReceived()) {
			tasks_ready++;
		}
	}
	if (usart_get_interrupt_source(MB_USART, USART_SR_TXE)) {
		if (pxMBFrameCBTransmitterEmpty()) {
			tasks_ready++;
		}
	}
	if (usart_get_interrupt_source(MB_USART, USART_SR_TC)) {
		/* TC is used for rs485 */
		USART_CR1(MB_USART) &= ~USART_CR1_TCIE;
		USART_SR(MB_USART) &= ~USART_SR_TC;
#if defined(MB_RS485_DE_PORT)
		gpio_clear(MB_RS485_DE_PORT, MB_RS485_DE_PIN);
#endif
	}
	vMBPortSetISR(FALSE);
	portEND_SWITCHING_ISR( tasks_ready ? pdTRUE : pdFALSE );
}

