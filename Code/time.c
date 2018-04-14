/*
Filename:
	time.c
Description:
	Contains function definitions for all time
	related functions
Author:
	Brant Geddes
	200350415
	
*/

#include "time.h"
#include "stm32f10x.h"

void time_decode(time_type *time){
	
	static const uint16_t days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	
	if (mseconds_tick >= 1000) {
		mseconds_tick = 0;
		time->seconds++;
	}
	
	if (time->seconds >= 60) {
		time->seconds = 0;
		time->minutes++;
	}
	
	if (time->minutes >= 60) {
		time->minutes = 0;
		time->hours++;
	}
	
	if (time->hours >= 24) {
		time->hours = 0;
		time->day++;
	}
	
	if (time->day >= days_in_month[(time->month) - 1]) {
		time->day = 0;
		time->month++;
	}
	
	if (time->month >= 12) {
		time->month = 1;
		time->year++;
	}
	
}

//EOF
