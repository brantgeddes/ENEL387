/*
Filename:
	lcd_init.c
Description:
	Contains function definitions for all LCD
	initialization and access functions
Author:
	Brant Geddes
	200350415

*/


#include "lcd_init.h"
#include "clock_init.h"
#include "stm32f10x.h"

//#define LCD_CHECK_BUSY_FLAG	//not working yet

#define LCD_CLEAR_DISPLAY		((uint32_t)0x01)
#define LCD_RETURN_HOME			((uint32_t)0x02)
#define LCD_DISPLAY_SET			((uint32_t)0x0E)
#define LCD_ENTRY_MODE			((uint32_t)0x06)
#define LCD_FUNCTION_SET		((uint32_t)0x38)
#define LCD_DDRAM_SET				((uint32_t)0x80)

#define LCD_ENA_ON					(GPIO_BSRR_BS1)
#define LCD_RS_ON						(GPIO_BSRR_BS0)
#define LCD_RW_ON						(GPIO_BSRR_BS5)
#define LCD_ENA_OFF					(GPIO_BSRR_BR1)
#define LCD_RS_OFF					(GPIO_BSRR_BR0)
#define LCD_RW_OFF					(GPIO_BSRR_BR5)

void lcd_init(void) {
	
	//Initialize clocks for PORT B, C
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN;
	
	lcd_configure_db_write();
	
	//Initialize LCD_RS, _RW, and _ENA as general purpose push-pull outputs, 50 MHz
	GPIOB->CRL |= GPIO_CRL_MODE0 | GPIO_CRL_MODE1 | GPIO_CRL_MODE5;
	GPIOB->CRL &= ~(GPIO_CRL_CNF0) & ~(GPIO_CRL_CNF1) & ~(GPIO_CRL_CNF5);
	
	GPIOB->BSRR |= LCD_ENA_ON | LCD_RS_OFF | LCD_RW_OFF;
	
	lcd_start();
	
}

void lcd_start(void) {
	
	delay(1000);
	GPIOC->BRR |= 0xFF;
	GPIOC->BSRR |= LCD_FUNCTION_SET & 0xFF;
	GPIOB->BSRR |= LCD_ENA_ON; delay(10); GPIOB->BSRR |= LCD_ENA_OFF; delay(10);
	GPIOB->BSRR |= LCD_ENA_ON; delay(10); GPIOB->BSRR |= LCD_ENA_OFF; delay(10); 
	GPIOB->BSRR |= LCD_ENA_ON; delay(10); GPIOB->BSRR |= LCD_ENA_OFF; delay(10);
	
	lcd_instruction_latch(LCD_DISPLAY_SET);
	lcd_instruction_latch(LCD_ENTRY_MODE);
	lcd_instruction_latch(LCD_CLEAR_DISPLAY);
	
}

void lcd_display(char *message, uint32_t row, uint32_t offset) {
	
	int i = 0, j = 0, ddram = 0;
	
	if (offset > 15) return;
	if (row > 1) return;
	
	while (message[i] != ';') {
		i++;
		if (i > 16) return;
	}
	
	if (offset > 16) return;
	if (row > 1) return;
	if (offset + i > 16) return;
	
	ddram = ((row * 0x40) + offset) | LCD_DDRAM_SET;

	lcd_instruction_latch(ddram);
	
	for (j = 0; j < i; j++) lcd_data_latch(message[j]);
	
}

void lcd_clear(void) {
	
	lcd_instruction_latch(LCD_CLEAR_DISPLAY);
	
}

void lcd_latch(void) {
	GPIOB->BSRR |= LCD_ENA_ON; lcd_delay(); GPIOB->BSRR |= LCD_ENA_OFF;
}

void lcd_data_latch(uint32_t data) {
	
	GPIOB->BSRR |= LCD_RS_ON;
	
	GPIOC->BRR |= 0xff;
	GPIOC->BSRR |= (data) & ((uint32_t)0xFF);
	lcd_latch();
	
}
	
void lcd_instruction_latch(uint32_t instruction) {
	
	GPIOB->BSRR |= LCD_RS_OFF;
	
	GPIOC->BRR |= 0xFF;
	GPIOC->BSRR = (instruction) & ((uint32_t)0xFF);
	lcd_latch();
	
}
	
void lcd_configure_db_read(void) { 
	
	//Initialize data bus as floating inputs
	GPIOC->CRL &= ~(GPIO_CRL_MODE0) & ~(GPIO_CRL_MODE1) & ~(GPIO_CRL_MODE2) & ~(GPIO_CRL_MODE3) &
								~(GPIO_CRL_MODE4) & ~(GPIO_CRL_MODE5) & ~(GPIO_CRL_MODE6) & ~(GPIO_CRL_MODE7);
	GPIOC->CRL &= ~(GPIO_CRL_CNF0) & ~(GPIO_CRL_CNF1) & ~(GPIO_CRL_CNF2) & ~(GPIO_CRL_CNF3) & 
								~(GPIO_CRL_CNF4) & ~(GPIO_CRL_CNF5) & ~(GPIO_CRL_CNF6) & ~(GPIO_CRL_CNF7);
	GPIOC->CRL |= GPIO_CRL_CNF0_0 | GPIO_CRL_CNF1_0 | GPIO_CRL_CNF2_0 | GPIO_CRL_CNF3_0 |
								GPIO_CRL_CNF4_0 | GPIO_CRL_CNF5_0 | GPIO_CRL_CNF6_0 | GPIO_CRL_CNF7_0;
	//Set RW to read
	GPIOB->BSRR |= LCD_RW_ON;
	
}

void lcd_configure_db_write(void) {
	
	//Initialize data bus as general purpose push-pull outputs, 50MHz
	GPIOC->CRL |= GPIO_CRL_MODE0 | GPIO_CRL_MODE1 | GPIO_CRL_MODE2 | GPIO_CRL_MODE3 | 
								GPIO_CRL_MODE4 | GPIO_CRL_MODE5 | GPIO_CRL_MODE6 | GPIO_CRL_MODE7;
	GPIOC->CRL &= ~(GPIO_CRL_CNF0) & ~(GPIO_CRL_CNF1) & ~(GPIO_CRL_CNF2) & ~(GPIO_CRL_CNF3) & 
								~(GPIO_CRL_CNF4) & ~(GPIO_CRL_CNF5) & ~(GPIO_CRL_CNF6) & ~(GPIO_CRL_CNF7);
	//Set RW to write
	GPIOB->BSRR |= LCD_RW_OFF;
	
}

#ifdef LCD_CHECK_BUSY_FLAG

void lcd_delay(void) {
	
	lcd_configure_db_read();
	
	GPIOB->BSRR |= LCD_RS_OFF;
	GPIOB->BSRR |= LCD_ENA_OFF;
	while ((GPIOC->IDR & GPIO_IDR_IDR7) == GPIO_IDR_IDR7) { GPIOB->BSRR |= LCD_ENA_OFF; delay(10); GPIOB->BSRR |= LCD_ENA_ON; delay(10); }
	lcd_configure_db_write();
	
}

#else

void lcd_delay(void) {	
	
	delay(100);
	
}

#endif

//EOF
