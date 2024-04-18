#include <avr/io.h>
#include <avr/interrupt.h>

char	state = 1;
int 	cycle = 0;

ISR(TIMER0_COMPA_vect) //interrupt function when OCR0A compare match in timer0
{
	if (cycle >= 100)
		state = 0;
	else if (cycle <= 0)
		state = 1;
	if (state == 1)
		cycle += 1;
	else if (state == 0)
		cycle -= 1;
	OCR1A = (312 / 100) * cycle; //setting a step in timer
}

int main()
{
	DDRB |= (1 << PB1); //setting DDRB1 to 1 configuring it to output
	PORTB &= ~(1 << PB1); //setting PORTB1 to 0 configuring it to output LOW

	PRR &= ~(1 << PRTIM1); //setting PRTIM1 to 0 to enable timer/counters1

	//TIMER1 CONFIG
	//sets the timer as mode 14 (compare mode FAST PWN) (p131-132 & p140)
	TCCR1A |= (1 << WGM11);
	TCCR1A &= ~(1 << WGM10);
	TCCR1B |= (1 << WGM12) | (1 << WGM13);

	//sets prescaler divisor of the timer1 at 1024(p143)
	TCCR1B |= (1 << CS12) | (1 << CS10);
	TCCR1B &= ~(1 << CS11);

	//sets led when timer == 0 and turn it off when timer == OCR1A (p140)
	TCCR1A &= ~(1 << COM1A0);
	TCCR1A |= (1 << COM1A1);

	ICR1 = 312; //setting max value of timer1 to (16000000 / 1024) / 50 to get 50Hz(p131-132)
	
	//TIMER0 CONFIG
	TCCR0A |= (1 << WGM00) | (1 << WGM01); //sets the timer0 to mode 7 (compare mode FAST PWN)
	TCCR0B |= (1 << WGM02); //sets the timer0 to mode 7 (compare mode FAST PWN)

	//sets prescaler divisor of the timer0 at 1024(p143)
	TCCR0B |= (1 << CS02) | (1 << CS00);
	TCCR0B &= ~(1 << CS01);

	TCCR0A &= ~(1 << COM0A0) & ~(1 << COM0A1);//when compare match, does nothing except returning to BOTTOM

	OCR0A = 78;//setting max value of timer to (16000000 / 1024) / 200 to get 200Hz

	TIMSK0 |= (1 << OCIE0A);//enabling OCR1A match interrupt
	sei(); //enabling interrupts

	while (1)
	{
	}
	return 0;
}
