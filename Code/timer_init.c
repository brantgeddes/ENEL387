/*
Filename:
	timer_init.c
Description:
	Contains function definitions for all timer
	initialization and control operations
Author:
	Brant Geddes
	200350415

*/

#include "timer_init.h"
#include "stm32f10x.h"

void timer_init(uint16_t prescaler, uint16_t period, uint16_t pulse, uint16_t cycles){
	
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; //Start TIM1 clock
	
	GPIOA->CRH |= GPIO_CRH_CNF9_1; //Configure GPIO as alternate function
																//Maps timer output to pin PA9
	
	TIM1->CR1 &= ~(TIM_CR1_DIR | TIM_CR1_CMS); //Select up counter mode
	
	TIM1->CR1 &= ~TIM_CR1_CKD;
	
	TIM1->ARR = period; //Set the Autoreload value (period)
	TIM1->CCR2 = pulse; //Set the Pulse value 
	TIM1->PSC = prescaler; //Set the Prescaler value
	TIM1->RCR = cycles - 1; //Pulse repeat
	TIM1->EGR = TIM_EGR_UG; //Generate an update event to reload the Prescaler
	
	TIM1->SMCR = 0x0; //Configure slave mode register (clock input)
	
	TIM1->CR1 |= TIM_CR1_OPM; //Select one-shot pulse mode
	TIM1->CCMR1 &= (uint16_t)~TIM_CCMR1_OC2M;
	TIM1->CCMR1 &= (uint16_t)~TIM_CCMR1_CC2S;
	//TIM1->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1; //PWM1
	TIM1->CCMR1 |= TIM_CCMR1_OC2M; //PWM2
	
	TIM1->CCER &= (uint16_t)~TIM_CCER_CC2P; //Select Channel 2 Output Compare
	
	TIM1->CCER = TIM_CCER_CC2E; //Enable the Compare output channel 2 
	TIM1->BDTR |= TIM_BDTR_MOE; //Enable the Timer main Output 
	
}

void timer_pulse(uint16_t pulse){
	
	TIM1->CCR2 = pulse;
	
}

void timer_start(void) {
	if (!(TIM1->CR1 & TIM_CR1_CEN)) TIM1->CR1 |= TIM_CR1_CEN;
}

//EOF
