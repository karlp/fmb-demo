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


#define MB_USART		USART2
#define MB_USART_RCC		RCC_USART2
#define MB_USART_RCC_PORT	RCC_GPIOA
#define MB_USART_PORT		GPIOA
#define MB_USART_PINS		(GPIO2 | GPIO3)
#define MB_USART_NVIC		NVIC_USART2_IRQ
#define MB_USART_ISR		usart2_isr

#define MB_RS485_DE_PORT	GPIOA
#define MB_RS485_DE_PIN		GPIO1
#define LED_BLUE_PORT		GPIOA
#define LED_BLUE_PIN		GPIO0

#define MB_TIMER TIM6
#define MB_TIMER_RCC RCC_TIM6
#define MB_TIMER_NVIC NVIC_TIM6_IRQ
#define MB_TIMER_ISR tim6_isr


#ifdef	__cplusplus
}
#endif

#endif	/* SYSCFG_H */

