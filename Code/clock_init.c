/*
Filename:
	clock_init.c
Description:
	Contains function definitions for all clock
	initialization and general purpose functions
Author:
	Brant Geddes
	200350415
	
*/

#include "clock_init.h"
#include "stm32f10x.h"

void clock_init(void) {
	
	RCC->CFGR |= RCC_CFGR_PLLMULL3 | RCC_CFGR_PLLSRC_PREDIV1;
	
	RCC->CR |= RCC_CR_PLLON | RCC_CR_HSEON | RCC_CR_HSION;
	
	while((RCC->CR & RCC_CR_PLLRDY) != RCC_CR_PLLRDY);
	
	
}

void delay(int delay_time){
	
	int i;
	for (i = 0; i < (delay_time * 100); i++);
	
}

//EOF
