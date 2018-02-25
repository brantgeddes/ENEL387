/*
Filename:
	lcd_init.h
Description:
	Contains function declarations for all LCD
	initialization and access functions
Author:
	Brant Geddes
	200350415

*/

#include "stm32f10x.h"

void lcd_init(void);

void lcd_start(void);
//
//lcd_display
//
//Description:
//	Takes a pointer to a character array delimited with ';', 
//	a row, and an offset and writes to the lcd
//	
void lcd_display(char *, uint32_t, uint32_t);

void lcd_clear(void);

void lcd_latch(void);

void lcd_data_latch(uint32_t);

void lcd_instruction_latch(uint32_t);

void lcd_configure_db_read(void);

void lcd_configure_db_write(void);

void lcd_delay(void);

//EOF
