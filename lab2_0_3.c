#include <avr/io.h>
#include <avr/interrupt.h>


void timer0_init() {
	
	TCCR0B |= (1 << CS02) | (1 << CS00); // prescale 1024
	TCNT0 = 0;
	TIMSK0 |= (1 << TOIE0); // interrupt enable 
}


ISR(TIMER0_OVF_vect) {
	PORTD ^= (1 << 6); 	
}

int main(void) {
		DDRD |= (1 << 6);
		
		sei(); // enable global interrupt
		timer0_init();
		while(1){
			
		}
}
