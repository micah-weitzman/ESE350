/*
 * uart_test.c
 *
 * Created: 1/2/2019 9:53:33 AM
 * Author : Kim Luong
 */ 



#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>


#define F_CPU 16000000UL
#define BAUDRATE 9600
#define BAUD_PRESCALLER (((F_CPU / (BAUDRATE * 16UL))) - 1)

void USART_init(void);
void USART_send( unsigned char data);
void USART_putstring(char* StringPtr);

char String[10] = "";


void USART_init(void){
	
	/*Set baud rate */
	UBRR0H = (unsigned char)(BAUD_PRESCALLER>>8);
	UBRR0L = (unsigned char)BAUD_PRESCALLER;
	//Enable receiver and transmitter
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8data, 2stop bit */
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

void USART_send( unsigned char data)
{
	
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
	
}

void USART_putstring(char* StringPtr){
	
	while(*StringPtr != 0x00){
		USART_send(*StringPtr);
	StringPtr++;}
	
}

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>

unsigned int DelayHi = 1500; // high time of the pulses to be created
unsigned int DelayLo = 20; // low time of the pulses to be created
int pcnt; // pulse count
char HiorLo; // flag to choose

unsigned int riseTime, fallTime, diff, overflows;
long pulse_width;

ISR (TIMER1_COMPA_vect){
	if(HiorLo){
		OCR1A += DelayHi;
		HiorLo = 0;
		
	} else {
		OCR1A += DelayLo;
		HiorLo = 1;
	}
	
}

ISR(TIMER1_OVF_vect) {
	overflows++; 
}


int main(void)
{
	USART_init();
	DDRB |= 0x02; // set Port B pin 1 to output
	TCCR1A = 0x40; // enable output compare on OC1A to toggle on compare
	TCCR1B = 0x02; // set prescalar to 8 for channel 0 (2MHz)
	OCR1A = TCNT1 + 16; // pull PB1 pin high quickly
	OCR1A += DelayHi; // start the second OC1 operation
	HiorLo = 0; // next time use DelayLo as delay count of OC0 operation
	TIMSK1 |= 0x02; // enable interrupt for OC1A
	TIMSK1 |= (1 << ICIE1);
	sei(); // enable interrupts
	while (1) {
	}
}
