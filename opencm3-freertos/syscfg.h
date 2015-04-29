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
#define BOARD_HARDWARE BHW_CUSTOM_ALTERNATIVE

#if (BOARD_HARDWARE == BHW_DISCO)
#define MB_USART		USART2
#define MB_USART_RCC		RCC_USART2
#define MB_USART_RCC_PORT	RCC_GPIOA
#define MB_USART_PORT		GPIOA
#define MB_USART_PINS		(GPIO2 | GPIO3)
#define MB_USART_NVIC		NVIC_USART2_IRQ
#define MB_USART_ISR		usart2_isr
#define LED_BLUE_PORT		GPIOB
#define LED_BLUE_PIN		GPIO6
#define LED_GREEN_PORT		GPIOB
#define LED_GREEN_PIN		GPIO7
#elif (BOARD_HARDWARE == BHW_CUSTOM_ALTERNATIVE)
#define LED_BLUE_PORT		GPIOB
#define LED_BLUE_PIN		GPIO11
#endif

#define MB_TIMER TIM6
#define MB_TIMER_RCC RCC_TIM6
#define MB_TIMER_NVIC NVIC_TIM6_IRQ
#define MB_TIMER_ISR tim6_isr


#ifdef	__cplusplus
}
#endif

#endif	/* SYSCFG_H */

