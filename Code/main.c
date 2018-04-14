
#include "clock_init.h"
#include "io_init.h"
#include "lcd_init.h"
#include "adc_init.h"
#include "timer_init.h"
#include "interrupt_init.h"

const uint32_t LCD_REFRESH = 250; 			//LCD Refresh time (ms)
const uint32_t LED_REFRESH = 1000;			//LED Flash time (ms)
const uint16_t ZFC_PERIOD = 833;				//ZFC period (60Hz * 2) (ms) 

typedef enum {
	no_fault,
	EIC_00,	//Internal Software Fault
	EIC_01,	//Startup Sequence Fault
	EIC_10,	//User Input Fault
	EIC_11,	//Setpoint Out of Bounds Fault
	EIC_20,	//Reserved
	EIC_30,	//Reserved
	EIC_40,	//Reserved
	EIC_50,	//Reserved
	EOC_00,	//Faulted Drive
	EOC_01,	//ZFC Input not Detected
	EOC_10,	//Overcurrent
	EOC_20,	//Reserved
	EOC_30,	//Reserved
	EOC_40,	//Reserved
	EOC_50	//Reserved
} fault_code_type;

const char fault_strings[9][6] = 	{"      ",
																	 "EIC_00",
																	 "EIC_01",
																	 "EIC_10",
																	 "EIC_11",
																	 "EOC_00",
																	 "EOC_01",
																	 "EOC_10",
																	 "EOC_11"};												


typedef struct {
	fault_code_type fault_code;
} fault_type;			

typedef enum {
	init,
	idle_1,
	run_1,
	run_2,
	faulted
} menu_state_type;
menu_state_type menu_state = init;

typedef enum {
	change,
	set	
} menu_set_type;
menu_set_type menu_set = set;

//ADC variables
volatile uint32_t adc_channel = 2;
volatile uint32_t adc_eoc = 1;
volatile uint32_t raw_setpoint = 0;

//Timing functions and variables
volatile uint32_t tick = 0;

//Menu functions and variables
menu_set_type state_change(menu_state_type *menu_state, menu_state_type state);
uint32_t lcd_last_refresh = 0;
uint32_t led_last_refresh = 0;
char lcd_line1[17];
char lcd_line2[17];

//String formatting functions
void string_format(char* lcd_line, char* string);
void setpoint_format(char* lcd_line, uint32_t raw_setpoint);
void voltage_format(char* lcd_line, uint32_t raw_setpoint);
void power_format(char* lcd_line, uint32_t raw_setpoint);
void current_format(char* lcd_line, uint32_t raw_setpoint);
void fault_format(char* lcd_line, fault_type fault);
void empty_format(char* lcd_line);

//Scaling functions
uint16_t setpoint_scale(uint32_t raw_setpoint);

//Fault functions and variables
void set_fault(fault_type* fault, fault_code_type fault_code);
fault_type fault;

int main(void) {
	
	clock_init();
	io_init();
	lcd_init();
	timer_init(240, ZFC_PERIOD, 1, 1);
	interrupt_init();
	SysTick_init(24000000/1000); //Ticks every ms
	adc_init();
	lcd_clear();
	
	fault.fault_code = no_fault;
	
	while(1) {
		
		switch (menu_state) {	//Inputs
			
			case init:
				state_change(&menu_state, idle_1);
				break;
			
			case idle_1:
			
				if (btn_read(2)) {
						menu_set = state_change(&menu_state, run_1);
				} else {
					if (btn_read_all()) menu_set = change; else menu_set = set;
				}
				
				break;
				
			case run_1:
				
				if (menu_set) {
					if (btn_read(0)) {
						menu_set = state_change(&menu_state, run_2);
					} else if (btn_read(1)) {
						menu_set = state_change(&menu_state, run_2);
					} else if (btn_read(3)) {
						menu_set = state_change(&menu_state, idle_1);
					}
				} else {
					if (btn_read_all()) menu_set = change; else menu_set = set;
				}
				break;
				
			case run_2:
			
				if (menu_set) {
					if (btn_read(0)) {
						menu_set = state_change(&menu_state, run_1);
					} else if (btn_read(1)) {
						menu_set = state_change(&menu_state, run_1);
					} else if (btn_read(3)) {
						menu_set = state_change(&menu_state, idle_1);
					}
				} else {
					if (btn_read_all()) menu_set = change; else menu_set = set;
				}
				break;
				
			case faulted:
				
				if (fault.fault_code == no_fault) set_fault(&fault, EIC_00);
				menu_set = set;
				break;
				
			default:
				menu_set = state_change(&menu_state, faulted);
				break;
			
		}
	
		if (sw_read(0) && !(menu_state == faulted)) menu_set = state_change(&menu_state, faulted);	//Fault Test
		if (sw_read(1) && !(menu_state == faulted)) raw_setpoint = 0xFFFFF;
		
		if (raw_setpoint > 0xFFF) {
			set_fault(&fault, EIC_11);
			menu_set = state_change(&menu_state, faulted);
		}

		if (tick >= (lcd_last_refresh + LCD_REFRESH)) { //Menu logic
			
			switch (menu_state){
				
				case init:
					break;
				case idle_1:
					string_format(lcd_line1, "Idle            ;");
					setpoint_format(lcd_line2, raw_setpoint);
					break;
				case run_1:
					current_format(lcd_line1, raw_setpoint);
					setpoint_format(lcd_line2, raw_setpoint);
					break;
				case run_2:
					voltage_format(lcd_line1, raw_setpoint);
					power_format(lcd_line2, raw_setpoint);
					break;
				case faulted:
					fault_format(lcd_line1, fault);
					empty_format(lcd_line2);
					break;
				default:
					menu_set = state_change(&menu_state, faulted);
					break;
				
			}
			
			if (menu_set == change) lcd_clear();
			
			lcd_display(lcd_line1, 0, 0);
			lcd_display(lcd_line2, 1, 0);
			lcd_last_refresh = tick;
			
			if (tick >= 1000000) {
				tick = 0;
				led_last_refresh = 0;
				lcd_last_refresh = 0;
			}
			adc_eoc = 1;
		}
		
		switch (menu_state) {	//Outputs
			case init:
				disable_zfc();
				timer_disable();
				break;
			case idle_1:
				disable_zfc();
				timer_disable();
				adc_start(2);
				adc_channel = 2;
				GPIOA->BSRR |= GPIO_BSRR_BS9;
				break;
			case run_1:
				timer_pulse(setpoint_scale(raw_setpoint));	//Changes timer pulse length based on potentiometer
				enable_zfc();
				timer_enable();
				GPIOA->BSRR |= GPIO_BSRR_BR9;
				break;
			case run_2:
				timer_pulse(setpoint_scale(raw_setpoint));	//Changes timer pulse length based on potentiometer
				enable_zfc();
				timer_enable();
				GPIOA->BSRR |= GPIO_BSRR_BR9;
				break;
			case faulted:
				if (tick >= (led_last_refresh + LED_REFRESH)) {
					GPIOA->ODR ^= GPIO_ODR_ODR9;
					led_last_refresh = tick;
				}
				disable_zfc();
				timer_disable();
				break;
			default:
				menu_set = state_change(&menu_state, faulted);
				break;
			
		}
		
		if (adc_eoc == 1) {	//Starts ADC Conversion
			raw_setpoint = adc_get();
			adc_start(2);
			adc_eoc = 0;
		}
		
	}
}
//Change menu type, returns menu change
menu_set_type state_change(menu_state_type *menu_state, menu_state_type state){
	
	(*menu_state) = state;
	return change;
	
}


void set_fault(fault_type* fault, fault_code_type fault_code){
	fault->fault_code = fault_code;
}
	
void string_format(char* lcd_line, char* string){
	
	int i = 0;
	for (; i<17; i++) lcd_line[i] = string[i];
	
}

void setpoint_format(char* lcd_line, uint32_t raw_setpoint){
	
	uint32_t setpoint = 10000 - ((raw_setpoint * 10000) / 4095);
	char value[3];
	char string[17] = "Setpoint:  XX.X%;";
	
	uint32_t i = 0, rem = setpoint;
	for (; i < 4; i++) { 
		value[3 - i] = (char)((rem % 10) + '0');
		rem /= 10;		
	}
	if ((value[0] == '0') && (value[1] == '0') && setpoint > 100) string[10] = '1';
	string[11] = value[0];
	string[12] = value[1];
	string[14] = value[2];
	for (i = 0; i<17; i++) lcd_line[i] = string[i];
	
	
}

void voltage_format(char* lcd_line, uint32_t raw_setpoint){
	
	uint32_t voltage = (((0xFFF - raw_setpoint) * 1200) / 4095);
	char value[3];
	char string[17] = "Voltage:    XXXV;";
	
	uint32_t i = 0, rem = voltage;
	for (; i < 4; i++) { 
		value[3 - i] = (char)((rem % 10) + '0');
		rem /= 10;		
	}
	
	string[12] = ((value[0] == '0') ? ' ' : value[0]);
	string[13] = (((value[1] == '0') && (value[0] == '0')) ? ' ' : value[1]);
	string[14] = value[2];
	for (i = 0; i<17; i++) lcd_line[i] = string[i];
	
}

void power_format(char* lcd_line, uint32_t raw_setpoint){
	
	uint32_t power = (((0xFFF - raw_setpoint) * 400) / 4095);
	char value[3];
	char string[17] = "Power:      XXXW;";
	
	uint32_t i = 0, rem = power;
	for (; i < 4; i++) { 
		value[3 - i] = (char)((rem % 10) + '0');
		rem /= 10;		
	}
	
	string[12] = ((value[0] == '0') ? ' ' : value[0]);
	string[13] = (((value[1] == '0') && (value[0] == '0')) ? ' ' : value[1]);
	string[14] = value[2];
	for (i = 0; i<17; i++) lcd_line[i] = string[i];
	
}
void current_format(char* lcd_line, uint32_t raw_setpoint){
	
	char value[3];
	char string[17] = "Current:   X.XXA;";
	uint32_t current = (((0xFFF - raw_setpoint) * 360) / 4095);
	
	uint32_t i = 0;
	uint32_t rem = current;
	for (; i < 4; i++) { 
		value[3 - i] = (char)((rem % 10) + '0');
		rem /= 10;		
	}
	
	string[11] = value[0];
	string[13] = value[1];
	string[14] = value[2];
	
	for (i = 0; i<17; i++) lcd_line[i] = string[i];

}

void fault_format(char* lcd_line, fault_type fault){
	
	uint32_t i = 0;
	char string[17] = "Faulted:  XXX_XX;";
	for (; i<6; i++) string[i + 10] = fault_strings[fault.fault_code][i];
	for (i = 0; i<17; i++) lcd_line[i] = string[i];
	
}

void empty_format(char* lcd_line){
	uint32_t i = 0;
	for (i = 0; i<17; i++) lcd_line[i] = ' ';
}
	
uint16_t setpoint_scale(uint32_t raw_setpoint){
	
	return (raw_setpoint * ZFC_PERIOD) / 4095;
	
}

void EXTI0_IRQHandler(void){
	EXTI->PR |= EXTI_PR_PR0;
	TIM1->CNT = (uint16_t)0x0;
	TIM1->CR1 |= TIM_CR1_CEN;
}

void ADC1_IRQHandler(void){
	ADC1->SR &= ~ADC_SR_EOC;
	adc_eoc = 1;
}

void SysTick_Handler(void)  {                               
	tick++; 
}

//EOF
