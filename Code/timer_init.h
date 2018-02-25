/*
Filename:
	timer_init.h
Description:
	Contains function declarations for all timer
	initialization and control operations
Author:
	Brant Geddes
	200350415

*/

#include "stm32f10x.h"

void timer_init(uint16_t prescaler, uint16_t period, uint16_t pulse, uint16_t cycles);
void timer_pulse(uint16_t pulse);
void timer_start(void);

//EOF
