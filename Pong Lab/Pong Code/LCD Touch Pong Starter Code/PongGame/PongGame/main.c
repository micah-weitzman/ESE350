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

// Timer values
unsigned int DelayHi = 37000; // high time of the pulses to be created
unsigned int DelayLo = 20; // low time of the pulses to be created
volatile char HiorLo; // flag to choose
unsigned int halfperiod = 70; // count to 70 to achieve half 440Hz

// Define SCORES
volatile char score2 = 0; 
int score1 = 0;  
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
uint8_t ballX; 
uint8_t ballY; 
uint8_t ballR = 3; 
int ballDx, ballDy; 

// gameplay mode
//	0 : 1 vs 1
//	1 : 1 vs CMP
//	2 : Accl vs CMP

uint16_t mode; 

char goal = 0; 

void reset_ball() {
	ballX = 63; 
	ballY = 32; 
	
	ballDx = rand() % 4;
	ballDy = rand() % 4 ;
}

void reset_game() {
	score1 = 0;
	score2 = 0;
	reset_ball();
}

//**** Example function to generate different game tones
void beep_tone() {
	PORTB |= (1 << PINB1); 
	TCNT1 = 0; 
	sei(); 
	//_delay_ms(100);
	//cli(); 
}

//***** Example function checks for Touchscreen inputs and changes the paddle positions  *****//
void checkInput() {
	x = readTouchX();
	y = readTouchY(); 
	// 1 vs 1 
	if (mode == 0) {
		if (x < 63 && y < 32) {
			if (!(p1Y <= 2)) {
				p1Y -= 3;
			}
		} else if (x < 63 && y > 32) {
			if (!(p1Y + p1L >= 63)) {
				p1Y += 3;
			}
		} else if (y < 32) {
			if (!(p2Y <= 2)) {
				p2Y -= 3; 
			}
		} else if (x > 63 && y > 32 && y < 62) {
			if(!(p2Y + p2L >= 63)) {
				p2Y += 3; 
			}
		}
	} else if (mode == 1) {
		if (x < 63 && y < 32) {
			if (!(p1Y <= 2)) {
				p1Y -= 3;
			}
			} else if (x < 63 && y > 32) {
			if (!(p1Y + p1L >= 63)) {
				p1Y += 3;
			}
		}
		p2Y = fmax(1, ballY - p2L / 2); 
	} else if (mode == 2) {
		uint16_t a = adc_read(4); 
		if (a > 330 && !(p1Y <= 2)) {
			p1Y -= 3;
		} else if (a < 300 && !(p1Y + p1L >= 63)) {
			p1Y += 3;
		}
		p2Y = fmax(1, ballY - p2L / 2); 
	}
}

// Check to see if goal is made
void update_score() {
	if (ballX - ballR <= 0) {
		beep_tone();
		score2++;
		PORTB |= (1 << PINB2);
		//PORTB &= ~(1 << PINB0) & ~(1 << PINB1);
		_delay_ms(1000);
		PORTB &= ~(1 << PINB2);
		reset_ball();
	} else if (ballX + ballR >= 126) {
		beep_tone();
		score1++;
		PORTB |= (1 << PINB0); 
		//PORTB &= ~(1 << PINB0) & ~(1 << PINB1); 
		_delay_ms(1000);
		PORTB &= ~(1 << PINB0); 
		reset_ball();
	}
}

//***** Example function to update game parameters
void update() {
//	if (goal) {}
	update_score();
	ballX += ballDx; 
	ballY += ballDy;
	if (score2 == 5 || score1 == 5) {
		_delay_ms(700); 
		reset_game(); 
	}
}

//**** Example draw function to draw all the game sprites/graphics
void draw() {
	lcd_set_brightness(0x18); //0x18
	clear_buffer(buff); 
	// draw boundaries
	drawline(buff, 0, 0, 127, 0, 1); 
	drawline(buff, 128, 0, 128, 63, 1); 
	drawline(buff, 0, 63, 127, 63, 1); 
	drawline(buff, 1, 1, 1, 63, 1); 
	
	// draw score
	char String1[1]; 
	sprintf(String1, "%i", score1); 
	drawstring(buff, 50, 0, String1); 
	char s2[1]; 
	sprintf(s2, "%i", score2); 
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
	if (ballX - ballR <= p1X + p1W +1 &&  ballY >= p1Y 
		&& ballY <= p1Y + p1L) {
			ballDx = -1*ballDx; 
			beep_tone(); 
		}
	// check if ball hit p2 paddle 
	else if (ballX + ballR >= p2X - 1 && ballY >= p2Y
		&& ballY <= p2Y + p2L) {
			ballDx = -1*ballDx; 
			beep_tone();
		}
	
	// check if ball hit upper wall 
	else if (ballY - ballR == 0) {
		ballDy = -1*ballDy;
		beep_tone(); 
	}
	
	// check if ball hit lower wall 
	if (ballY + ballR >= 63) {
		ballDy = -1*ballDy;
		beep_tone(); 
	}
}



int main(void)
{
	//setting up the gpio for backlight
	DDRD |= 0x80;
	PORTD &= ~0x80;
	PORTD |= 0x00;
	DDRD |= (1 << PIND6); // set pin D6 to out
	DDRB |= (1 << PINB0) | (1 << PINB2); 
	
	DDRB |= 0x05;
	PORTB &= ~0x05;
	PORTB |= 0x00;
	DDRD |= (1 << PIND7); 
	
	DDRB |= 0x02; // set Port B pin 1 to output
	TCCR1B = 0x02; // set prescalar to 8 for channel 0 (2MHz)
	TIMSK1 |= (1 << TOIE1); // enable overflow interrupt

 
	analogSetUp(); 
	
	//lcd initialisation
	lcd_init();
	lcd_command(CMD_DISPLAY_ON);
	lcd_set_brightness(0x18);
	write_buffer(buff);
	_delay_ms(1000);
	clear_buffer(buff);
	
	reset_game(); 
	mode = 2; 
	

	while (1)
	{
		draw();
		checkInput();
		checkCollisions();
		update();
		_delay_ms(50);
	}
}

ISR(TIMER1_OVF_vect) {
	PORTB &= ~(1 << PINB1);
	cli();  
}