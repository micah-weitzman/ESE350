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

char displayChar = 0;

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
	uint8_t st[8] = "Micah\0";
	
	int x = 30;
	int y = 30;
	int dx = 3;
	int dy = 3;
	int rad = 10;
	while (1)
	{
		clear_buffer(buff);
		lcd_set_brightness(0x18);
		drawstring(buff,0,0,st);
		
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

