
#include <avr/io.h>
#include <avr/interrupt.h>

unsigned int halfperiod = 70; // count to 70 to achieve half 440Hz

void timer0_init() {
	TCCR0B |= (1 << CS02); // prescale 256 
	TCNT0 = 0;
	TCCR0A |= (1 << COM0A0); // compare output mode non PWM toggle OC0A
	TIMSK0 |= (1 << OCIE0A); //set timer counter0 output compare match A enable
}


ISR(TIMER0_COMPA_vect) {
	OCR0A += halfperiod; 
}

int main(void) {
	DDRD |= (1 << 6);
	
	timer0_init();
	sei(); // enable global interrupt

	OCR0A = TCNT0 + halfperiod; 
	
	while(1); 
}
