/*
Filename:
	util.c
Description:
	Contains function definitions for any utility
	functions
Author:
	Brant Geddes
	200350415

*/

#include "util.h"
#include "stm32f10x.h"

void debounce(uint32_t btn, uint32_t time){
	
	
	
}

void hex_string(char* buffer, uint32_t analog){
	
	uint32_t i = 0, rem = 0;
	
	for (i = 0; i < 4; i++) { 
		rem = 0xF & analog>>(i*4);
		if (rem < 10) buffer[4 - i] = 48 + rem; else buffer[4 - i] = 55 + rem;
	}	
	buffer[0] = '0'; buffer[1] = 'x';	buffer[5] = ';';
	
}

void analog_voltage(char* buffer, uint32_t analog){
	
	char value[4];
	dec_string(value, ((analog * 330) / 4095));
	
	int i = 0;
	
	for (; i < 4; i++) {
		buffer[i + 11] = value[i];
	}
	
	buffer[11] = buffer[12];
	buffer[12] = '.';
	buffer[15] = 'v';
	
}

void analog_temp(char* buffer, uint32_t analog){
		
	char value[4];
	dec_string(value, ((analog * 8) / 10));
	
	int i = 0;
	
	for (; i < 4; i++) {
		buffer[i + 9] = value[i];
	}
	
	buffer[9] = ' ';
	buffer[13] = buffer[12];
	buffer[12] = '.';
	buffer[14] = 0xDF;
	buffer[15] = 'C';
	
	
	
}

void dec_string(char* buffer, uint32_t analog) {
	
	uint32_t i = 0, rem = analog;
	for (i = 0; i < 4; i++) { 
		buffer[3 - i] = (char)((rem % 10) + '0');
		rem /= 10;		
	}
	
}


uint32_t lowpass_filter(uint32_t* samples, uint32_t n) {
	uint32_t average = 0, filtered_val = 0, good_samples = 0, i = 0;
	for (; i < n; i++) {
		average += samples[i];
	}
	average /= n;
	for (i = 0; i < n; i++) {
		if (samples[i] <= average) { filtered_val += samples[i]; good_samples++; } 
	}
	return filtered_val/good_samples;
}


//EOF
