/*
Filename:
	interrupt_init.h
Description:
	Contains function declarations for all interupt
	initialization and control operations
Author:
	Brant Geddes
	200350415

*/

#include "stm32f10x.h"

extern volatile uint32_t tick;
extern volatile uint32_t mseconds_tick;
void interrupt_init(void);
void SysTick_init(uint32_t counts);

//EOF
