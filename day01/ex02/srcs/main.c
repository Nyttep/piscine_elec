#include <avr/io.h>

int main()
{
	DDRB |= (1 << PB1); //setting DDRB1 to 1 configuring it to output
	PORTB &= ~(1 << PB1); //setting PORTB1 to 0 configuring it to output LOW

	PRR &= ~(1 << PRTIM1); //setting PRTIM1 to 0 to enable timer/counters1

	//sets the timer as mode 14 (compare mode) (p131-132 & p140)
	TCCR1A |= (1 << WGM11);
	TCCR1A &= ~(1 << WGM10);
	TCCR1B |= (1 << WGM12) | (1 << WGM13);

	//sets prescaler divisor of the clock at 1024(p143)
	TCCR1B |= (1 << CS12) | (1 << CS10);
	TCCR1B &= ~(1 << CS11);

	//stes led when timer == 0 and turn it off when timer == OCR1A (p140)
	TCCR1A &= ~(1 << COM1A0);
	TCCR1A |= (1 << COM1A1);

	ICR1 = 16000000 / 1024; //setting max value of timer to 16000000 / 1024(p131-132)
	OCR1A = ICR1 / 10; 
	
	while (1)
	{
	}
	return 0;
}
