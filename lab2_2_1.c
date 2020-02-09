#include <avr/io.h>
#include <avr/interrupt.h>

unsigned int halfperiod = 70; // count to 70 to achieve half 440Hz

void timer0_init() {
	TCCR0B |= (1 << CS02); // prescale 256 
	TCCR0A |= (1 << WGM01) | (1 << COM0A0); // sets CTC mode 
	TCNT0 = 0; 
}

int main(void) {
	DDRD |= (1 << 6);
	
	timer0_init();
	sei(); 

	OCR0A = halfperiod; 
	
	while(1); 
}
