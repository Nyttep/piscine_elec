#include <avr/io.h>

void	delay_500()
{
	uint32_t	i = 0;

	while (i < 350000)
	{
		i++;
	}
}

int main()
{
	DDRB |= (1 << PB1); //setting DDRB1 to 1 configuring it to output
	PORTB &= ~(1 << PB1); //setting PORTB1 to 0 configuring it to output LOW

	PRR &= ~(1 << PRTIM1); //setting PRTIM1 to 0 to enable timer/counters1

	//sets the timer as mode 15 (compare mode) (p131-132 & p140)
	TCCR1A |= (1 << WGM11) | (1 << WGM10);
	TCCR1B |= (1 << WGM12) | (1 << WGM13);

	//sets prescaler divisor of the clock at 1024(p143)
	TCCR1B |= (1 << CS12) | (1 << CS10);
	TCCR1B &= ~(1 << CS11);

	//toggles the OC1A pin, here a led (p140)
	TCCR1A |= (1 << COM1A0) | (1 << COM1B0);
	TCCR1A &= ~(1 << COM1A1) | ~(1 << COM1B1);

	OCR1A = F_CPU / 2048; //setting max value of timer (p131-132)
	
	while (1)
	{
	}
	return 0;
}
