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

unsigned int risSig;
unsigned int dist;
unsigned int halfperiod = 7; // half of count to 16 w/ prescaler


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

void timer1_init() {
	TCCR1B |= (1 << ICES1); // input capture for ICP1 (PB0)
	TCCR1B |= (1 << CS12); // set 256 prescaler
	TCNT1 = 0;
}

ISR(TIMER1_COMPA_vect) {
	OCR0A += halfperiod;
	PORTB &= (0 << PINB1);
	
	DDRB |= (0 << PINB1);
	TCCR1B |= (1 << ICES1);
}

ISR(TIMER1_CAPT_vect) {
	if (6 >> TCCR1B) {
		risSig = ICR1;
	} else {
		dist = ICR1 - risSig;
	}
	
	TCCR1B ^= (1 << ICES1);
}

int main(void)
{
	DDRB |= (1 << 1); // set PB1 to output
	PORTB |= (1 << PINB1);
	
	timer1_init();


	OCR0A = TCNT1 + halfperiod;

	USART_init();

	sei();
    while (1)
	{

		sprintf(String,"%d \n", dist); // Print to terminal (converts a number into a string)
		USART_putstring(String);

	}
}



	
	

