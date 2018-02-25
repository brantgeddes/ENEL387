
#include "util.h"
#include "clock_init.h"
#include "io_init.h"
#include "lcd_init.h"
#include "adc_init.h"
#include "timer_init.h"
#include "interrupt_init.h"

const uint32_t LCD_REFRESH = 350; //0.35s
const uint16_t ZFC_PERIOD = 833;
const uint32_t CT_TURNS = 1;
const uint32_t BURDEN_RESISTOR = 1000;
const uint32_t CT_SPAN = 140;
const uint32_t CT_ZERO = 300;
const uint32_t CT_OVERLOAD = 10000;

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
	uint16_t seconds;
	uint16_t minutes;
	uint16_t hours;
	uint16_t day;
	uint16_t month;
	uint16_t year;
} time_type;

typedef struct {
	fault_code_type fault_code;
	time_type timestamp;
} fault_type;			

typedef enum {
	init,
	idle_1,
	idle_2,
	run_1,
	run_2,
	run_3,
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
volatile uint32_t raw_current = 0;
volatile uint32_t raw_setpoint = 0;

//Timing functions and variables
void time_decode(time_type *time);
volatile uint32_t tick = 0;
volatile uint32_t mseconds_tick = 0;
time_type time;

//Menu functions and variables
menu_set_type state_change(menu_state_type *menu_state, menu_state_type state);
uint32_t lcd_last_refresh = 0;
char lcd_line1[17];
char lcd_line2[17];

//String formatting functions
void string_format(char* lcd_line, char* string);
void setpoint_format(char* lcd_line, uint32_t raw_setpoint);
void voltage_format(char* lcd_line, uint32_t raw_setpoint);
void power_format(char* lcd_line, uint32_t raw_setpoint);
void current_format(char* lcd_line, uint32_t raw_current);
void fault_format(char* lcd_line, fault_type fault);
void timestamp_format(char* lcd_line, time_type* fault_time);

//Scaling functions
uint16_t setpoint_scale(uint32_t raw_setpoint);

//Fault functions and variables
void set_fault(fault_type* fault, time_type time, fault_code_type fault_code);
fault_type fault;

int main(void) {
	
	clock_init();
	io_init();
	lcd_init();
	adc_init();
	timer_init(240, ZFC_PERIOD, 1, 1);
	interrupt_init();
	SysTick_init(24000000/1000); //Ticks every 100ms
	lcd_clear();
	
	fault.fault_code = no_fault;
	
	time.seconds = 0;
	time.minutes = 0;
	time.hours = 0;
	time.day = 22;
	time.month = 2;
	time.year = 2018;
	
	while(1) {
		
		switch (menu_state) {
			
			case init:
				state_change(&menu_state, idle_1);
			break;
			
			case idle_1:
				
				string_format(lcd_line1, "Idle            ;");
				setpoint_format(lcd_line2, raw_setpoint);
			
				if (btn_read(0)) {
						menu_set = state_change(&menu_state, idle_2);
					} else if (btn_read(1)) {
						menu_set = state_change(&menu_state, idle_2);
					} else if (btn_read(2)) {
						menu_set = state_change(&menu_state, run_1);
				} else {
					if (btn_read_all()) menu_set = change; else menu_set = set;
				}
				
			break;
				
			case idle_2:
				
				string_format(lcd_line1, "Time:           ;");
				timestamp_format(lcd_line2, &time);
			
				if (btn_read(0)) {
						menu_set = state_change(&menu_state, idle_1);
					} else if (btn_read(1)) {
						menu_set = state_change(&menu_state, idle_1);
					} else if (btn_read(2)) {
						menu_set = state_change(&menu_state, run_1);
				} else {
					if (btn_read_all()) menu_set = change; else menu_set = set;
				}
				
			break;
				
			case run_1:
				
				current_format(lcd_line1, raw_current);
				setpoint_format(lcd_line2, raw_setpoint);
				
				if (menu_set) {
					if (btn_read(0)) {
						menu_set = state_change(&menu_state, run_3);
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
				
				voltage_format(lcd_line1, raw_setpoint);
				power_format(lcd_line2, raw_setpoint);
			
				if (menu_set) {
					if (btn_read(0)) {
						menu_set = state_change(&menu_state, run_1);
					} else if (btn_read(1)) {
						menu_set = state_change(&menu_state, run_3);
					} else if (btn_read(3)) {
						menu_set = state_change(&menu_state, idle_1);
					}
				} else {
					if (btn_read_all()) menu_set = change; else menu_set = set;
				}
			break;
				
			case run_3:
				
				string_format(lcd_line1, "Time:           ;");
				timestamp_format(lcd_line2, &time);
			
				if (menu_set) {
					if (btn_read(0)) {
						menu_set = state_change(&menu_state, run_2);
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
				
				if (fault.fault_code == no_fault) set_fault(&fault, time, EIC_00);
			
				fault_format(lcd_line1, fault);
				timestamp_format(lcd_line2, &time);
			
				if (!sw_read(0)) {
					set_fault(&fault, time, no_fault);
					menu_set = state_change(&menu_state, idle_1);
				}
			break;
				
			default:
				menu_set = state_change(&menu_state, faulted);
			break;
			
		}
		
		if (sw_read(0) && !(menu_state == faulted)) state_change(&menu_state, faulted);	//Fault Test
		
		if (tick >= (lcd_last_refresh + LCD_REFRESH)) {
			if (menu_set == change) lcd_clear();
			lcd_display(lcd_line1, 0, 0);
			lcd_display(lcd_line2, 1, 0);
			lcd_last_refresh = tick;
		}
		
		timer_pulse(setpoint_scale(raw_setpoint));
		time_decode(&time);
		
	}
}

menu_set_type state_change(menu_state_type *menu_state, menu_state_type state){
	
	(*menu_state) = state;
	return change;
	
}

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

void set_fault(fault_type* fault, time_type time, fault_code_type fault_code){
	fault->fault_code = fault_code;
	fault->timestamp = time;
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
	if ((value[0] == '0') && (value[1] == '0') && setpoint != 0) string[10] = '1';
	string[11] = value[0];
	string[12] = value[1];
	string[14] = value[2];
	for (i = 0; i<17; i++) lcd_line[i] = string[i];
	
	
}

void voltage_format(char* lcd_line, uint32_t raw_setpoint){
	
	uint32_t voltage = ((raw_setpoint * 1200) / 4095);
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
	
	uint32_t power = ((raw_setpoint * 1000) / 4095);
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
void current_format(char* lcd_line, uint32_t raw_current){
	
	uint32_t i = 0, current = ((raw_current * 3300) / 4095); //Scale in voltage
	current *= (CT_TURNS * 2000) / BURDEN_RESISTOR;		//Translate to primary current
	current =  ((current * CT_SPAN) / 100) - CT_ZERO;	//Calibration
	
	char value[3];
	char string[17] = "Current:   X.XXA;";
	
	if (current >= CT_OVERLOAD) {
		
		string[11] = ' ';
		string[12] = 'O';
		string[13] = '/';
		string[14] = 'L';
		string[15] = ' ';
		
	} else {
		
		uint32_t rem = current;
		for (; i < 4; i++) { 
			value[3 - i] = (char)((rem % 10) + '0');
			rem /= 10;		
		}
		
		string[11] = value[0];
		string[13] = value[1];
		string[14] = value[2];
	}
	
	for (i = 0; i<17; i++) lcd_line[i] = string[i];
}

void fault_format(char* lcd_line, fault_type fault){
	
	uint32_t i = 0;
	char string[17] = "Faulted:  XXX_XX;";
	for (; i<6; i++) string[i + 10] = fault_strings[fault.fault_code][i];
	for (i = 0; i<17; i++) lcd_line[i] = string[i];
	
}

void timestamp_format(char* lcd_line, time_type* fault_time) {
	
	uint32_t i = 0;
	uint16_t minutes = fault_time->minutes;
	uint16_t hours = fault_time->hours;
	uint16_t days = fault_time->day;
	uint16_t months = fault_time->month;
	uint16_t years = fault_time->year;
	
	char string[17] = "MM/DD/YY   HH:MM;";
	
	string[12] = (char)((hours % 10) + '0');
	hours /= 10;
	string[11] = (char)((hours % 10) + '0');
	
	string[15] = (char)((minutes % 10) + '0');
	minutes /= 10;
	string[14] = (char)((minutes % 10) + '0');
	
	string[4] = (char)((days % 10) + '0');
	days /= 10;
	string[3] = (char)((days % 10) + '0');
	
	string[1] = (char)((months % 10) + '0');
	months /= 10;
	string[0] = (char)((months % 10) + '0');
	
	string[7] = (char)((years % 10) + '0');
	years /= 10;
	string[6] = (char)((years % 10) + '0');
	
	
	for (; i<17; i++) lcd_line[i] = string[i];
	
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
	if (adc_channel == 2) {
		raw_setpoint = adc_get();
		adc_start(3);
		adc_channel = 3;
	} else if (adc_channel == 3) {
		raw_current = adc_get();
		adc_start(2);
		adc_channel = 2;
	} else {
		adc_channel = 2;
	}
}

//EOF
