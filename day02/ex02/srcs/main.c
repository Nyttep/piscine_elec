#include <avr/io.h>
#include <avr/interrupt.h>

char 	counter = 0;
char	pins[4] = {PB0, PB1, PB2, PB4};

ISR(INT0_vect) //interrupt function when OCR0A compare match in timer0
{
	if (counter < 15)
		counter++;
	
	for (char i = 3; i >= 0; i--)
	{
		if (counter & (1 << i))
			PORTB |= (1 << pins[i]);
		else
			PORTB &= ~(1 << pins[i]);
	}
}

ISR(PCINT2_vect) //interrupt function when OCR0A compare match in timer0
{
	if (counter > 0)
		counter--;
	
	for (char i = 3; i >= 0; i--)
	{
		if (counter & (1 << i))
			PORTB |= (1 << pins[i]);
		else
			PORTB &= ~(1 << pins[i]);
	}
}

int main()
{
	DDRB |= 1 | (1 << PB1) | (1 << PB2) | (1 << PB4); //setting DDRB0 DDRB1 DDRB2 DDRB4 to 1 configuring it to output
	PORTB &= ~1 & ~(1 << PB1) & ~(1 << PB2) & ~(1 << PB4); //setting PORTB0 PORTB1 PORTB2 PORTB4 to 0 configuring it to output LOW

	DDRD = DDRD & ~(1 << PD2) & ~(1 << PD4); //setting DDRD2 DDR4 to 0 configuring it to input
	PORTD = PORTD | (1 << PD2) | (1 << PD4); //setting PORTD2 DDR4 to 1 activating the pull-up resistor

	EIMSK |= (1 << INT0); //enabling external pin change interrupt 0 (switch 1)
	//setting up when the interruption will generate 
	EICRA |= (1 << ISC01);
	EICRA &= ~(1 << ISC00);

	PCMSK2 |= (1 << PCINT20); //enable PCINT20 to trigger interrupt
	PCICR |= (1 << PCIE2); //enable external pin change interrupt 2 (PCINT23:16)
	
	sei(); //enabling interrupts

	while (1)
	{
	}
	return 0;
}
