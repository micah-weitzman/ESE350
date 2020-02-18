/*/*
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

char String[15] = "";


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

unsigned int DelayHi = 37000; // high time of the pulses to be created
unsigned int DelayLo = 20; // low time of the pulses to be created
char HiorLo; // flag to choose

volatile unsigned int riseTime, fallTime, diff;
volatile char i = 0; // controls if looking for high or low edge  

void timer0_set() {
	TCCR0A |= 0x40;
	TCCR0A |= 0x02;
	DDRD |= 0x40;
	TCCR0B |= 0x03;
}

void analog_init() {
	ADMUX |= (1 << REFS0); 
	ADMUX &= ~(1 << REFS1); 
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); 
}

uint16_t adc_read(uint8_t ch) {
	ch &= 0b00000111; 
	ADMUX = (ADMUX & 0xF8) | ch;
	
	ADCSRA |= (1 << ADSC); 
	//while(ADCSRA & (1 << ADSC)); 
	return  (ADC); 
}

void contFreq() {
	if (diff) {
		OCR0A = (int) (0.011 * (float)(diff + 1025.14));
	}
}

void setToglBtn() {
	PCICR |= (1 << PCIE2); // enable interput on reg. 2
	PCMSK2 |= (1 << PCINT23); // enable interput on pin 23
}

void discFreq() {
	if (diff > 1600) {
		OCR0A = 121;
	} else if (diff > 1400) {
		OCR0A = 106;
	} else if (diff > 1200) {
		OCR0A = 97;
	} else if (diff > 1000) {
		OCR0A = 92;
	} else if (diff > 800) {
		OCR0A = 81;
	} else if (diff > 600) {
		OCR0A = 72;
	} else if (diff > 400) {
		OCR0A = 64;
	} else {
		OCR0A = 61;
	}

}

int main(void)
{
	USART_init();
	
	DDRB |= (1 << PINB4) | (1 << PINB3) | (1 << PINB2); 
	DDRB |= 0x02; // set Port B pin 1 to output
	DDRD |= (1 << PIND6); // set pin D6 to out
	TCCR1A = 0x40; // enable output compare on OC1A to toggle on compare
	TCCR1B = 0x02; // set prescalar to 8 for channel 0 (2MHz)
	OCR1A = TCNT1 + 16; // pull PB1 pin high quickly
	OCR1A += DelayHi; // start the second OC1 operation
	HiorLo = 0; // next time use DelayLo as delay count of OC0 operation
	TIMSK1 |= 0x02; // enable interrupt for OC1A
	TIMSK1 |= (1 << ICIE1) | (1 << TOIE1);
	timer0_set(); 
	analog_init(); 
	
	volatile int cvsf = 1; // controls cont vs discrete
	//volatile int pressed = 0; 
	volatile int pressed = 1;

	int tmp = cvsf; 
	uint16_t adc_res; 
	
	sei(); // enable interrupts
	while (1) {
		
		//sprintf(String, "%i\n", tmp); 
		//USART_putstring(String); 
		
		if (PIND & (1 << PIND7)) {
			if (pressed == 1) {
				tmp = pressed;
				pressed = 0;
			} else {
				tmp = pressed;
				pressed = 1;
			} 
		}  
		
		if (tmp) {
			if (diff) {contFreq();}
		} else {
			if (diff) {discFreq();}
		}
			
		adc_res = adc_read(0);
		
		PORTB |= (1 << PINB2);
		
		
		if (adc_res > 880) {
			PORTB |= (1 << PINB4) | (1 << PINB3) | (1 << PINB2); 
		} else if (adc_res > 867) {
			PORTB |= (1 << PINB4) | (1 << PINB3); 
			PORTB &= ~(1 << PINB2); 
		} else if (adc_res > 815) {
			PORTB |= (1 << PINB4); 
			PORTB &= ~(1 << PINB3);
			PORTB &= ~(1 << PINB2);  
		} else if (adc_res > 762) {
			PORTB &= ~(1 << PINB3);
			PORTB |= (1 << PINB4) | (1 << PINB2); 
		} else if (adc_res > 710) {
			PORTB |= (1 << PINB2) | (1 << PINB3);
			PORTB &= ~(1 << PINB4); 
		} else if (adc_res > 657) {
			PORTB |= (1 << PINB3);
			PORTB &= ~(1 << PINB4); 
			PORTB &= ~(1 << PINB2); 
		} else if (adc_res > 604) {
			PORTB |= (1 << PINB2); 
			PORTB &= ~(1 << PINB4); 
			PORTB &= ~(1 << PINB3); 
		} else {
			PORTB &= ~(1 << PINB2); 
			PORTB &= ~(1 << PINB4); 
			PORTB &= ~(1 << PINB3); 
		}
	} 
	
}



ISR (TIMER1_COMPA_vect){
	if(HiorLo){
		OCR1A += DelayHi;
		HiorLo = 0;
		TCCR1B |= (1 << ICES1);
		TIMSK1 |= (1 << ICIE1);
		
		} else {
		OCR1A += DelayLo;
		HiorLo = 1;	
				
	}

}

ISR(TIMER1_CAPT_vect) {
	if (i) {
		fallTime = ICR1;
		
		diff = (fallTime - riseTime) / 2.0;

		i = 0;
	} else {
		riseTime = ICR1;
		i = 1;
	}
		
	TCCR1B ^= (1 << ICES1); // toggle to look for opposite edge
}
