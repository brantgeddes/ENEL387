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

void interrupt_init(void);
void disable_zfc(void);
void enable_zfc(void);
void SysTick_init(uint32_t counts);

//EOF
