#include <avr/io.h>
#include <avr/interrupt.h>

void timer1_init() { // sets up timer 	
	TCCR1B |= (1 << ICES1); // input capture 
	TCCR1B |= (1 << CS10); // clock on rising edge  
	TIMSK1 |= (1 << ICIE1); // knows when goes from 0 to 1 
}


// TODO when interrupt occurs 
ISR(TIMER1_CAPT_vect) { 
	
	PORTB ^= (1 << PINB5);
	TCCR1B ^= (1 << ICES1); 
	
}

int main(void) {
	DDRB |= (1 << PINB5); 
	PORTB |= (1 << PINB5); 
	
	timer1_init();
	sei();
	while (1) {
		
	}
}
