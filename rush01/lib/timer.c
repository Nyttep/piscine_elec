#include "timer.h"

#define CLOCK_CYCLE_1MS (F_CPU / 1024 / 1000)

void timer0_init(uint8_t ms)
{
	// Set Timer0 in CTC mode
	TCCR0A |= (1 << WGM01);

	// Set compare match register for 10ms intervals
	OCR0A = ms * (F_CPU / 1024 / 1000);

	TCCR0B |= (0b101 << CS00);
}

void timer0_COMP()
{
	// Enable Timer0 compare match interrupt
	TIMSK0 |= (1 << OCIE0A);
	sei();
}

void timer1_init(uint16_t ms)
{
	// Set Waveform Generation Mode 15, TOP on OCR1A (16-4)
	TCCR1A = (0b11 << WGM10);
	TCCR1B = (0b11 << WGM12);

	// Set Clock scalar per 1024 (16-5)
	TCCR1B |= (0b101 << CS00);

	// Set top depending of number ms
	OCR1A = CLOCK_CYCLE_1MS * ms;
}

void timer1_OVF()
{
	// Set mode to overflow interupt
	TIMSK1 |= (1 << TOIE1);
	sei();
}
