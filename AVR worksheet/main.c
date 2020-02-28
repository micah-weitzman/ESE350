#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "iocompat.h"          

enum { UP, DOWN };

ISR (TIMER1_OVF_vect)           
{
    static uint16_t pwm;        // The PWM is being used in 10-bit mode, so we need a 16-bit variable to remember the current value.
    static uint8_t direction;

    switch (direction)         // This section determines the new value of the PWM
    {
        case 1:
            if (++pwm == TIMER1_TOP)
                direction = 0;
            break;

        case 0:
            if (--pwm == 0)
                direction = 1;
            break;
    }

    OCR = pwm;                  // Newly computed value is loaded into the PWM register
}

void
ioinit (void)                   // Called after a reset to initialize PWM and enable interrupts
{
    /* Timer 1 is 10-bit PWM (8-bit PWM on some ATtinys). */
    TCCR1A = TIMER1_PWM_INIT;
    /*
     * Start timer 1.
     *
     * NB: TCCR1A and TCCR1B could actually be the same register, so
     * take care to not clobber it.
     */
    TCCR1B |= TIMER1_CLOCKSOURCE;
    /*
     * Run any device-dependent timer 1 setup hook if present.
     */
#if defined(TIMER1_SETUP_HOOK)
    TIMER1_SETUP_HOOK();
#endif

    /* Set PWM value to 0. */
    OCR = 0;

    /* Enable OC1 as output. */
    DDROC = _BV (OC1);

    /* Enable timer 1 overflow interrupt. */
    TIMSK = _BV (TOIE1);
    sei ();
}

int main (void)
{

    ioinit ();

    /* loop forever, the interrupts are doing the rest */

    for (;;)                    // All work is done in interrupt routine
        sleep_mode();

    return (0);
}