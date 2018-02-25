/*
Filename:
	interrupt_init.c
Description:
	Contains function definitions for all interupt
	initialization and control operations
Author:
	Brant Geddes
	200350415

*/

#include "interrupt_init.h"
#include "stm32f10x.h"

void interrupt_init(void) {
	
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPAEN;
	GPIOA->CRL &= ~GPIO_CRL_MODE0;
	GPIOA->CRL &= ~GPIO_CRL_CNF0_1;
	GPIOA->CRL |= GPIO_CRL_CNF0_0;
	AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI0_PA;
	EXTI->IMR |= EXTI_IMR_MR0;
	EXTI->FTSR |= EXTI_FTSR_TR0;
	EXTI->RTSR |= EXTI_RTSR_TR0;
	NVIC->ISER[0] |= NVIC_ISER_SETENA_6; //EXTI0 on PA0
	
	NVIC->ISER[0] |= NVIC_ISER_SETENA_18; //ADC1 EOC flag
	
}

void SysTick_init(uint32_t counts) {
	
	SysTick->CTRL = 0x0;
	SysTick->VAL = 0x0;
	SysTick->LOAD = counts;
	SysTick->CTRL |= SysTick_CTRL_ENABLE | SysTick_CTRL_CLKSOURCE | SysTick_CTRL_TICKINT;
	
}

void SysTick_Handler(void)  {                               
	tick++; 
	mseconds_tick++;
}

//EOF
