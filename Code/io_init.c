/*
Filename:
	io_init.c
Description:
	Contains function definitions for all IO
	initialization and read/write operations
Author:
	Brant Geddes
	200350415

*/

#include "io_init.h"
#include "stm32f10x.h"


void io_init(void){
	
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN;
	
	//Enable PA9, PA10, PA11, and PA12 as general purpose output push-pull, max speed 50MHz
	GPIOA->CRH |= GPIO_CRH_MODE9 | GPIO_CRH_MODE10 | GPIO_CRH_MODE11 | GPIO_CRH_MODE12;
	GPIOA->CRH &= ~GPIO_CRH_CNF9 & ~GPIO_CRH_CNF10 & ~GPIO_CRH_CNF11 & ~GPIO_CRH_CNF12;
	
	GPIOC->CRH |= GPIO_CRH_MODE8 | GPIO_CRH_MODE9;
	GPIOC->CRH &= ~GPIO_CRH_CNF8 & ~GPIO_CRH_CNF9;
	
	//Enable PA6, PA7, PC10, PC11 as input with pull up/down
	GPIOA->CRL &= ~GPIO_CRL_MODE5 & ~GPIO_CRL_MODE6 & ~GPIO_CRL_MODE7;
	GPIOA->CRL &= ~GPIO_CRL_CNF5_0 & ~GPIO_CRL_CNF6_0 & ~GPIO_CRL_CNF7_0;
	GPIOA->CRL |= GPIO_CRL_CNF5_1 | GPIO_CRL_CNF6_1 | GPIO_CRL_CNF7_1;
	GPIOB->CRH &= ~GPIO_CRH_MODE8 & ~GPIO_CRH_MODE9;
	GPIOB->CRH &= ~GPIO_CRH_CNF8_0 & ~GPIO_CRH_CNF9_0;
	GPIOB->CRH |= GPIO_CRH_CNF8_1 | GPIO_CRH_CNF9_1;
	GPIOC->CRH &= ~GPIO_CRH_MODE10 & ~GPIO_CRH_MODE11 & ~GPIO_CRH_MODE12;
	GPIOC->CRH &= ~GPIO_CRH_CNF10_0 & ~GPIO_CRH_CNF11_0 & ~GPIO_CRH_CNF12_0;
	GPIOC->CRH |= GPIO_CRH_CNF10_1 | GPIO_CRH_CNF11_1 | GPIO_CRH_CNF12_1;
	
	GPIOA->ODR |= 0x1E00;
	
}

void led_set(int led){
	
	if ((led < 0) | (led > 3)) {
		return;
	} else {
		GPIOA->BSRR |= GPIO_BSRR_BR9<<led;
	}
	
}

void led_rset(int led) {
	
	if ((led < 0) | (led > 3)) {
		return;
	} else {
		GPIOA->BSRR |= GPIO_BSRR_BS9<<led;
	}
	
}

int sw_read(int sw) {
	
	return !!((1<<sw) & sw_read_all());
	
}

int sw_read_all(void){
	
	return 	0xF & ~(((GPIOA->IDR & GPIO_IDR_IDR6) | 
					(GPIOA->IDR & GPIO_IDR_IDR7))>>6 | 
					((GPIOC->IDR & GPIO_IDR_IDR10) | 
					(GPIOC->IDR & GPIO_IDR_IDR11))>>8);
	
}

int btn_read(int btn) {
	
	return !!((1<<btn) & btn_read_all());
	
}

int btn_read_all(void) {
	
	return 0xF & 
					~((((GPIOB->IDR & GPIO_IDR_IDR9) |
					(GPIOB->IDR & GPIO_IDR_IDR8))>>8) | 
					((GPIOC->IDR & GPIO_IDR_IDR12)>>10) | 
					((GPIOA->IDR & GPIO_IDR_IDR5)>>2));
	
}

//EOF
