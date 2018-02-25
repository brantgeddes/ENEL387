/*
Filename:
	adc_init.h
Description:
	Contains function declarations for all ADC
	initialization and read/write operations
Author:
	Brant Geddes
	200350415

*/

#include "stm32f10x.h"

void adc_init(void);
void adc_start(uint32_t channel);
uint32_t adc_get(void);

//EOF
