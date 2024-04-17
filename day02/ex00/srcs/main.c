#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

/*
1
*/

ISR(INT0_vect) //interrupt funciont when button is pressed
{
	PORTB ^= (1 << PB1);
}

int main()
{
	DDRB |= (1 << PB1); //setting DDRB1 to 1 configuring it to output
	PORTB &= ~(1 << PB1); //setting PORTB1 to 0 configuring it to output LOW

	DDRD = DDRD & ~(1 << PD2); //setting DDRD2 to 0 configuring it to input
	PORTD = PORTD | (1 << PD2); //setting PORTD2 to 1 activating the pull-up resistor

	sei(); //enabling interrupts
	EIMSK |= (1 << INT0); //enabling external pin interrupt
	//setting up when the interruption will generate 
	EICRA |= (1 << ISC01);
	EICRA &= ~(1 << ISC00);

	while (1)
	{
	}
	return 0;
}
