/*
Filename:
	b.h
Description:
	Contains function declarations for all IO
	initialization and read/write operations
Author:
	Brant Geddes
	200350415

*/

#include "stm32f10x.h"

void io_init(void);

void led_set(int led);

void led_rset(int led);

int sw_read(int sw);
//
//sw_read_all
//
//	description:
//			Returns an int with the first four bits set to 
//			the status of the four dip-switches
//	return type:
//			int
//
int sw_read_all(void);

int btn_read(int btn);

int btn_read_all(void);

//EOF
