#include <avr/io.h>
#include <util/delay.h>

int main()
{
	float cycle = 1;
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

	ICR1 = 7812; //setting max value of timer to 16000000 / 2048(p131-132)
	OCR1A = 781 * cycle; //setting a step in timer
	
	DDRD = DDRD & ~(1 << PD2) & ~(1 << PD4); //setting DDRD2 DDR4 to 0 configuring it to input
	PORTD = PORTD | (1 << PD2) | (1 << PD4); //setting PORTD2 DDR4 to 1 activating the pull-up resistor

	while (1)
	{
		OCR1A = 781 * cycle; //setting a step in timer
		if (((PIND & (1 << PD2)) == 0) && cycle < 10)
		{
			cycle += 1;
			_delay_ms(500);
		}
		if (((PIND & (1 << PD4)) == 0) && cycle > 2)
		{
			cycle -= 1;
			_delay_ms(500);
		}
	}
	return 0;
}
