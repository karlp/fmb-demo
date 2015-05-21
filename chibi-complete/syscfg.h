/* 
 * File:   syscfg.h
 * Author: karlp
 *
 * Created on April 23, 2015, 5:29 PM
 */

#ifndef SYSCFG_H
#define	SYSCFG_H

#ifdef	__cplusplus
extern "C" {
#endif

#define BHW_DISCO 0 /* Stock stm32l discovery board */
#define BHW_CUSTOM_ALTERNATIVE 1 /* just some L1 board with a rs485 chip */
//#define BOARD_HARDWARE BHW_CUSTOM_ALTERNATIVE
#define BOARD_HARDWARE BHW_DISCO

#if (BOARD_HARDWARE == BHW_DISCO)
#define MB_USART		UARTD2
#define MB_USART_PORT		GPIOA
#define MB_USART_PIN_TX		2
#define MB_USART_PIN_RX		3
#define LED_BLUE_PORT		GPIOB
#define LED_BLUE_PIN		6
#define LED_GREEN_PORT		GPIOB
#define LED_GREEN_PIN		7
#elif (BOARD_HARDWARE == BHW_CUSTOM_ALTERNATIVE)
#define MB_USART		UARTD1
#define MB_USART_RCC		RCC_USART1
#define MB_USART_RCC_PORT	RCC_GPIOB
#define MB_USART_PORT		GPIOB
#define MB_USART_PINS		(GPIO6 | GPIO7)
#define MB_USART_NVIC		NVIC_USART1_IRQ
#define MB_USART_ISR		usart1_isr
#define MB_RS485_DE_PORT	GPIOB
#define MB_RS485_DE_PIN		5
#define LED_BLUE_PORT		GPIOB
#define LED_BLUE_PIN		11
#endif
	
	enum etrace_src {
		TRACE_CONSOLE,
		TRACE_MODBUS_RX,
	};

#ifdef	__cplusplus
}
#endif

#endif	/* SYSCFG_H */

