#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//red=PD5 green=PD6 blue=PD3

void	init_rgb()
{
	DDRD |= (1 << PD3) | (1 << PD5) | (1 << PD6); //setting DDRB1 to 1 configuring it to output
	PORTD &= ~(1 << PD3) & ~(1 << PD5) & ~(1 << PD6); //setting PORTB1 to 0 configuring it to output LOW

	PRR &= ~(1 << PRTIM2) & ~(1 << PRTIM0); //setting PRTIM2/PRTIM0 to 0 to enable timer/counters2 and 0

	//TIMER0 CONFIG
	//sets the timer as mode 1 (PWN Phase correct) (p131-132 & p140)
	TCCR0A |= (1 << WGM00);
	TCCR0A &= ~(1 << WGM01) & ~(1 << WGM02);

	//sets prescaler divisor of the timer0 at 256(p143)
	TCCR0B &= ~(1 << CS00) & ~(1 << CS01);
	TCCR0B |= (1 << CS00);

	TCCR0A |= (1 << COM0A1) | (1 << COM0B1);
	TCCR0A &= ~(1 << COM0A0) & ~(COM0B0); 

	OCR0A = 0; //setting max value of timer1 to (16000000 / 255) / 244 to get 200Hz(p131-132)
	OCR0B = 0;

	//TIMER2 CONFIG

	TCCR2A |= (1 << WGM20);
	TCCR2A &= ~(1 << WGM21) & ~(1 << WGM22);

	//sets prescaler divisor of the timer2 at 256(p143)
	TCCR2B &= ~(1 << CS20);
	TCCR2B |= (1 << CS21) | (1 << CS22);

	TCCR2A |= (1 << COM2B1);
	TCCR2A &= ~(1 << COM2B0);

	OCR2B = 0; //setting max value of timer1 to (16000000 / 255) / 244 to get 200Hz(p131-132)

}

void	set_rgb(uint8_t r, uint8_t g, uint8_t b)
{
	OCR0A = g;
	OCR0B = r;
	OCR2B = b;
}

void	wheel(uint8_t pos) {
	pos = 255 - pos;
	if (pos < 85) {
		set_rgb(255 - pos * 3, 0, pos * 3);
	} else if (pos < 170) {
		pos = pos - 85;
		set_rgb(0, pos * 3, 255 - pos * 3);
	} else {
	pos = pos - 170;
	set_rgb(pos * 3, 255 - pos * 3, 0);
	}
}

int main()
{
	init_rgb();
	  
	uint8_t	pos = 0;
	int8_t	dir = 1;
	while (1)
	{
		if (pos == 255)
			dir = -1;
		else if (pos == 0)
			dir = 1;
		pos += dir; 
		wheel(pos);
		_delay_ms(5);
	}
	return 0;
}
