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
	DDRC &= ~(1 << PINC1); 
	DDRC |= (1 << PINC0); 
	PORTC |= (1 << PINC0); 
}


uint16_t  adc_read(uint8_t ch) {
	ch &= 0b00000111; 
	ADMUX = (ADMUX & 0xF8)|ch;
	
	ADCSRA |= (1<<ADSC); 

	return(ADC);
	
}

uint16_t readTouchX() {
	// Start by readying diff between y values to find x 
	DDRC |= (1 << PINC1) | (1 << PINC3); // set X- and X+ to output
	DDRC &= ~(1 << PINC0); 
	DDRC &= ~(1 << PINC2); // set Y- and Y+ to input 
	PORTC |= (1 << PINC3); 
	PORTC &= ~(1 << PINC1) & ~(1 << PINC0) & ~(1 << PINC2); 
	uint16_t x = adc_read(0); 
	PORTC &= ~(1 << PINC3); 
	return x; 
}

uint16_t readTouchY() {
	// Start by readying diff between x values to find y
	DDRC |= (1 << PINC0) | (1 << PINC2); // set Y- and Y+ to output
	DDRC &= ~(1 << PINC1) & ~(1 << PINC3); // set Y- and Y+ to input
	PORTC |= (1 << PINC0);
	PORTC &= ~(1 << PINC1) & ~(1 << PINC3) & ~(1 << PINC2); 
	return adc_read(3);
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
	
	//lcd initialisation
	lcd_init();
	lcd_command(CMD_DISPLAY_ON);
	lcd_set_brightness(0x18);
	write_buffer(buff);
	_delay_ms(1000);
	clear_buffer(buff);
	uint8_t st[8] = "pong\0";
	
	int x = 30;
	int y = 30;
	int dx = 0;
	int dy = 0;
	int rad = 5;
	while (1)
	{
		analogSetUp();
		clear_buffer(buff);
		lcd_set_brightness(0x18);
		drawstring(buff,0,0,st);
		
		setpixel(buff, 60, 5, 1); 
		
		drawline(buff, 0, 0, 100, 40, 1); 
		
		drawrect(buff, 12, 12, 100, 20, 1);
		fillrect(buff, 10, 50, 30, 10, 1);
		
		drawcircle(buff, 80, 30, 20, 1);
		fillcircle(buff, x, y, rad, 1);
		write_buffer(buff);
		_delay_ms(100);
		
		if (x + rad < 120 && x - rad > 5) {
			x += dx;
			} else {
			dx = -dx;
			x += dx;
		}
		if (y + rad < 60 && y - rad > 5) {
			y += dy;
			} else if (y - rad < 0) {
			dy = -2 * dy;
			} else {
			if (dy > 0) {
				dy = -dy;
				} else {
				dy = abs(dy);
			}
			y += dy;
		}
	}
}