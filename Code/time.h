/*
Filename:
	time.h
Description:
	Contains function declarations for all time
	related functions
Author:
	Brant Geddes
	200350415
	
*/

#include "stm32f10x.h"

typedef struct {
	uint16_t seconds;
	uint16_t minutes;
	uint16_t hours;
	uint16_t day;
	uint16_t month;
	uint16_t year;
} time_type;


void time_decode(time_type *time);

//EOF
