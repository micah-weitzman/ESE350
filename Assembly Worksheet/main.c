#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#define BUTTON_PIN PB0
#define LED_PIN    PB5

ISR(TIMER1_CAPT_vect) {
    // Handle debouncing first
    /*
    bool pin_state = PINB & (1 << BUTTON_PIN);
    for (volatile int i = 0; i < 10; i++) {
        if (PINB & (1 << BUTTON_PIN) != pin_state) {
            return;
        }
    }
    */

    if (TCCR1B & (1 << ICES1)) {  // we're capturing rising edges (releases)
        PORTB &= ~(1 << LED_PIN);
    } else {                      // we're capturing falling edges (presses)
        PORTB |= (1 << LED_PIN);
    }

    TCCR1B ^= (1 << ICES1);       // toggle edge detection
}

int main(void) {
    DDRB &= ~(1 << BUTTON_PIN); // set button pin (PB0, pin 8) as input
    DDRB |= (1 << LED_PIN);     // set led pin (PB5, pin 13) as output
    PORTB &= ~(1 << LED_PIN);   // turn off led

    TCCR1B &= ~(1 << ICES1);    // capture falling edges on PB0 initially
    TCCR1B |= (1 << ICNC1);     // enable noise canceling on PB0
    TCCR1B |= (1 << CS10) | (1 << CS12);      // start timer 1 with 1024x prescaling
    TIMSK1 |= (1 << ICIE1);     // enable PB0 input capture interrupt
    sei();                      // enable interrupts globally
    while (1) {
    }
}