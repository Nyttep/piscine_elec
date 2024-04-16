#include <avr/io.h>

void	delay_500()
{
	uint32_t	i = 0;

	while (i < 120000)
	{
		i++;
	}
}

int main()
{
	DDRB |= (1 << PB1); //setting DDRB1 to 1 configuring it to output
	PORTB &= ~(1 << PB1); //setting PORTB1 to 0 configuring it to output LOW

	while (1)
	{
		PORTB ^= (1 << PB1); //switching state of PORTB1 so it blinks
		delay_500();
		delay_500();
	}
	return 0;
}
