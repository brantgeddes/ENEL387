/*
Filename:
	util.h
Description:
	Contains function declarations for any utility
	functions
Author:
	Brant Geddes
	200350415

*/

#include "stm32f10x.h"

void debounce(uint32_t btn, uint32_t time);
void hex_string(char*, uint32_t);
void analog_voltage(char*, uint32_t);
void analog_temp(char*, uint32_t);
void dec_string(char*, uint32_t);
uint32_t lowpass_filter(uint32_t*, uint32_t);

//EOF
