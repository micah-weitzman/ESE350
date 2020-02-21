#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include "lcd.h"

#define FREQ 16000000
#define BAUD 9600
#define HIGH 1
#define LOW 0
#define BUFFER 1024
#define BLACK 0x000001


void analogSetUp() {
	ADMUX |= (1 << REFS0); // set reference voltage
	ADCSRA |= (1 << ADEN); // Enable ADC
	ADCSRA |= (1 << ADPS1); //| (1<< ADPS2) | (1 << ADPS0); // set to 4 clock division factor 
	//ADCSRA |= (1 << ADIE); // enabe interrupts 
}


uint16_t  adc_read(uint8_t ch) {
	ch &= 0b00000111; 
	ADMUX = (ADMUX & 0xF8)|ch;
	
	ADCSRA |= (1<<ADSC); 
	while(ADCSRA & (1<<ADSC));
	return(ADC);
	
}

uint16_t readTouchX() {
	// Start by readying diff between y values to find x 
	DDRC |= (1 << PINC1)| (1 << PINC3); // set X- and X+ to output
	DDRC &= ~(1 << PINC0) & ~(1 << PINC2); // set Y- and Y+ to input 
	PORTC |= (1 << PINC1); 
	PORTC &= ~(1 << PINC3); // & ~(1 << PINC0); //& ~(1 << PINC2); 
	_delay_ms(50); 
	uint16_t x = adc_read(2) * 0.25; 
	PORTC &= ~(1 << PINC1); 
	if (x == 0) {
		return 0; 
	} else if (x - 37 > 128) {
		return 128;
	} else if (x > 30) {
		return x - 37; 
	} else {
		return 0; 
	}
}

uint16_t readTouchY() {
	// Start by readying diff between x values to find y
	DDRC |= (1 << PINC0) | (1 << PINC2); // set Y- and Y+ to output
	DDRC &= ~(1 << PINC1) & ~(1 << PINC3); // set Y- and Y+ to input
	PORTC |= (1 << PINC0);
	PORTC &= ~(1 << PINC2); // & ~(1 << PINC3);  ~(1 << PINC1) 
	_delay_ms(50); 
	uint16_t tmpY = adc_read(3) * 0.1875;
	PORTC &= ~(1 << PINC0); 
	if (tmpY > 120|| tmpY < 4) {
		return 0; 
	} else if  (tmpY < 130) {
		return (130 - tmpY) * 0.89;
	} else {
		return 0; 
	}
}

void detectInput() {
	//
}


int main(void)
{
	//setting up the gpio for backlight
	DDRD |= 0x80;
	PORTD &= ~0x80;
	PORTD |= 0x00;
	
	DDRB |= 0x05;
	PORTB &= ~0x05;
	PORTB |= 0x00;
	
	analogSetUp(); 
	
	//lcd initialisation
	lcd_init();
	lcd_command(CMD_DISPLAY_ON);
	lcd_set_brightness(0x18);
	write_buffer(buff);
	_delay_ms(1000);
	clear_buffer(buff);
	
	int x; 
	int y; 
	while (1)
	{
		char String[20];
		//clear_buffer(buff);
		lcd_set_brightness(0x18);
		
		x = readTouchX(); 
		y = readTouchY();

		sprintf(String, "X=%u, Y=%u", x, y); 
		//drawstring(buff, 0, 0, String); 
		
		setpixel(buff, x, y, 1); 
		
		write_buffer(buff);
		//_delay_ms(10);
	}
}