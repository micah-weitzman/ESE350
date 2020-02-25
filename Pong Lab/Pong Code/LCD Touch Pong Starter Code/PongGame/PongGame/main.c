#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include "lcd.h"
#include <time.h>

#define FREQ 16000000
#define BAUD 9600
#define HIGH 1
#define LOW 0
#define BUFFER 1024
#define BLACK 0x000001

uint16_t x;
uint16_t y;


void analogSetUp() {
	ADMUX |= (1 << REFS0); // set reference voltage
	ADCSRA |= (1 << ADEN); // Enable ADC
	ADCSRA |= (1 << ADPS1) | (1<< ADPS2) | (1 << ADPS0); // set to 4 clock division factor 
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
	uint16_t x = fmin(adc_read(0) * 0.156 - 19, 127); 
	PORTC &= ~(1 << PINC1); 
	return x; // * 0.22496;   
	
	if (x > 30) {
	return fmin(fmax(0,(x - 50)), 127);
	}
	else return 0; 
}

uint16_t readTouchY() {
	// Start by readying diff between x values to find y
	DDRC |= (1 << PINC0) | (1 << PINC2); // set Y- and Y+ to output
	DDRC &= ~(1 << PINC1) & ~(1 << PINC3); // set Y- and Y+ to input
	PORTC |= (1 << PINC0);
	PORTC &= ~(1 << PINC2); // & ~(1 << PINC3);  ~(1 << PINC1) 
	_delay_ms(50); 
	uint16_t tmpY = adc_read(3) * 0.1143 - 36;
	if (tmpY >= 63 && tmpY < 1000) {
		return 0;
	} else if (tmpY < 63) {
		return 63 - tmpY; 
	} else {
		return 63; 
	}
	PORTC &= ~(1 << PINC0); 
	return tmpY;// * 0.1707; 
	if (tmpY < 30) {
		return 0; 
	} else if  (tmpY < 160) {
		return fmin((tmpY) *0.7, 63); 
	} else {
		return 0; 
	}
}

// Define SCORES
uint8_t score1 = 0; 
uint8_t score2 = 0; 

//Define paddle parameter variables
uint8_t p1X = 3; 
uint8_t p1Y = 32; 
uint8_t p1W = 2; 
uint8_t p1L = 10; 

uint8_t p2X = 124; 
uint8_t p2Y = 32; 
uint8_t p2W = 2; 
uint8_t p2L = 10; 

//Define ball parameter variables
uint8_t ballX = 56; 
uint8_t ballY = 32; 
uint8_t ballR = 3; 
int ballDx, ballDy; 


char goal = 0; 

//**** Example function to generate different game tones
void beep_tone() {
	
}

//***** Example function checks for Touchscreen inputs and changes the paddle positions  *****//
void checkInput() {
	x = readTouchX();
	//_delay_ms(100); 
	y = readTouchY(); 
}

//***** Example function to update game parameters
void update() {
//	if (goal) {}
	
	ballX += ballDx; 
	ballY += ballDy; 
}

//**** Example draw function to draw all the game sprites/graphics
void draw() {
	lcd_set_brightness(0x18);
	clear_buffer(buff); 
	// draw boundaries
	drawline(buff, 0, 0, 127, 0, 1); 
	drawline(buff, 128, 0, 128, 63, 1); 
	drawline(buff, 0, 63, 127, 63, 1); 
	drawline(buff, 1, 1, 1, 63, 1); 
	
	// draw score
	char s1[1]; 
	char s2[1]; 
	sprintf(s1, "%u", score1); 
	sprintf(s2, "%u", score2); 
	drawstring(buff, 53, 0, s1); 
	drawstring(buff, 69, 0, s2); 
	
	// draw paddles
	fillrect(buff, p1X, p1Y, p1W, p1L, 1);
	fillrect(buff, p2X, p2Y, p2W, p2L, 1); 
	
	// draw ball
	fillcircle(buff, ballX, ballY, ballR, 1); 
	write_buffer(buff); 
}

//**** Example function checks for Horizontal and Vertical collisions of the Ball
void checkCollisions() {
	// check if ball hit p1 paddle 
	if (ballX - ballR == p1X + p1W &&  ballY >= p1Y 
		&& ballY <= p1Y + p1L) {
			ballDx = -1*ballDx; 
		}
	// check if ball hit p2 paddle 
	else if (ballX + ballR == p1X && ballY >= p2Y
		&& ballY <= p2Y + p2L) {
			ballDx = -1*ballDy; 
		}
	
	// check if ball hit upper wall 
	else if (ballY - ballR == 0) {
		ballDy = -1*ballDy; 
	}
	
	// check if ball hit lower wall 
	else if (ballY + ballR == 127) {
		ballDy = -1 * ballDy; 
	}
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
	
	ballDx = rand() % 4 + 1;
	ballDy = rand() % 4 + 1;
	draw(); 
	while (1)
	{
		//update(); 
		draw();
		_delay_ms(50);  
	}
}