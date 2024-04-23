#include <avr/io.h>
#include <avr/interrupt.h>

uint8_t	state = 0;

ISR(TIMER1_COMPA_vect) //interrupt function when OCR0A compare match in timer0
{
	switch (state)
	{
		case 0://RED
		{
			PORTD &= ~(1 << PD3) & ~(1 << PD6);
			PORTD |= (1 << PD5);
			state++;
			break;
		}

		case 1://GREEN
		{
			PORTD &= ~(1 << PD3) & ~(1 << PD5);
			PORTD |= (1 << PD6);
			state++;
			break;
		}

		case 2://BLUE
		{
			PORTD &= ~(1 << PD5) & ~(1 << PD6);
			PORTD |= (1 << PD3);
			state = 0;
			break;
		}
	
		default:
			break;
	}
}

int main()
{
	DDRD |= (1 << PD3) | (1 << PD5) | (1 << PD6); //setting DDRB1 to 1 configuring it to output
	PORTD &= ~(1 << PD3) & ~(1 << PD5) & ~(1 << PD6); //setting PORTB1 to 0 configuring it to output LOW

	PRR &= ~(1 << PRTIM1); //setting PRTIM1 to 0 to enable timer/counters1

	//TIMER1 CONFIG
	//sets the timer as mode 14 (compare mode FAST PWN) (p131-132 & p140)
	TCCR1A |= (1 << WGM11) | (1 << WGM10);
	TCCR1B |= (1 << WGM12) | (1 << WGM13);

	//sets prescaler divisor of the timer1 at 1024(p143)
	TCCR1B |= (1 << CS12) | (1 << CS10);
	TCCR1B &= ~(1 << CS11);

	TCCR1A &= ~(1 << COM1A0) & ~(1 << COM1A1);//when compare match, does nothing except returning to BOTTOM

	OCR1A = 15625; //setting max value of timer1 to (16000000 / 1024) / 1 to get 1Hz(p131-132)
	
	TIMSK1 |= (1 << OCIE1A);//enabling timer1 match interrupt
	sei(); //enabling interrupts
	  

	while (1)
	{
	}
	return 0;
}
