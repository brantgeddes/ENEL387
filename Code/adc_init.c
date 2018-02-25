/*
Filename:
	adc_init.c
Description:
	Contains function definitions for all ADC
	initialization and access functions
Author:
	Brant Geddes
	200350415

*/

#include "adc_init.h"
#include "clock_init.h"
#include "stm32f10x.h"

void adc_init(void) {
	
	//Enable clocks for PORT A and ADC1
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN | RCC_APB2ENR_IOPAEN;
	
	//Power ADC1 on
	ADC1->CR2 |= ADC_CR2_ADON;
	delay(10);
	ADC1->CR2 |= ADC_CR2_CAL;
	while((ADC1->CR2 & ADC_CR2_CAL) == ADC_CR2_CAL);
	
	//Configure PORT A Pin 1, 2, 3 as analog inputs
	GPIOA->CRL &= ~(GPIO_CRL_CNF1) & ~(GPIO_CRL_CNF2) & ~(GPIO_CRL_CNF3) &
								~(GPIO_CRL_MODE1) & ~(GPIO_CRL_MODE2) & ~(GPIO_CRL_MODE3);
	
	//Enable interrupt
	ADC1->CR1 |= ADC_CR1_EOCIE;
	
	//Select channels to convert
	ADC1->SQR1 &= ~ADC_SQR1_L;
	ADC1->SQR3 |= ADC_SQR3_SQ1_1;
	
	//Select channel sample time
	ADC1->SMPR2 |= ADC_SMPR2_SMP0;
	
	//Continuous conversion
	//ADC1->CR2 |= ADC_CR2_CONT;
	
	//Start ADC
	ADC1->CR2 |= ADC_CR2_ADON;
	
}

void adc_start(uint32_t channel) {
	
	ADC1->SQR3 = 0x0;
	ADC1->SQR3 |= ((channel == 2) ? 2 : 3);
	
	ADC1->CR2 |= ADC_CR2_ADON;
	
}

uint32_t adc_get(void) {
	
	return ADC1->DR;
	
}

//EOF
