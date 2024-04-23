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
			state++;
			break;
		}
	
		case 3://YELLOW
		{
			PORTD &= ~(1 << PD3);
			PORTD |= (1 << PD5) | (1 << PD6);
			state++;
			break;
		}
	
		case 4://CYAN
		{
			PORTD &= ~(1 << PD5);
			PORTD |= (1 << PD3) | (1 << PD6);
			state++;
			break;
		}
	
		case 5://MAGENTA
		{
			PORTD &= ~(1 << PD6);
			PORTD |= (1 << PD3) | (1 << PD5);
			state++;
			break;
		}
	
		case 6://WHITE
		{
			PORTD |= (1 << PD3) | (1 << PD5) | (1 << PD6);
			state = 0;
			break;
		}
	
		default:
			break;
	}
}

void	init_rgb()
{
	DDRD |= (1 << PD3) | (1 << PD5) | (1 << PD6); //setting DDRB1 to 1 configuring it to output
	PORTD &= ~(1 << PD3) & ~(1 << PD5) & ~(1 << PD6); //setting PORTB1 to 0 configuring it to output LOW

	//TIMER1 CONFIG
	PRR &= ~(1 << PRTIM1) & ~(1 << PRTIM0); //setting PRTIM1/PRTIM0 to 0 to enable timer/counters1 and 0

	//sets the timer as mode 14 (compare mode FAST PWN) (p131-132 & p140)
	TCCR1A |= (1 << WGM11);
	TCCR1A &= ~(1 << WGM10);
	TCCR1B |= (1 << WGM12) | (1 << WGM13);

	//sets prescaler divisor of the timer1 at 1024(p143)
	TCCR1B |= (1 << CS12) | (1 << CS10);
	TCCR1B &= ~(1 << CS11);

	TCCR1A &= ~(1 << COM1A0) & ~(1 << COM1A1);//when compare match, does nothing except returning to BOTTOM

	ICR1 = 312; //setting max value of timer1 to (16000000 / 1024) / 50 to get 50Hz(p131-132)
	OCR1A = (312 / 2); //setting a step in timer
	
	//TIMER0 CONFIG
	TCCR0A |= (1 << WGM00) | (1 << WGM01); //sets the timer0 to mode 7 (compare mode FAST PWN)
	TCCR0B |= (1 << WGM02); //sets the timer0 to mode 7 (compare mode FAST PWN)

	//sets prescaler divisor of the timer0 at 1024(p143)
	TCCR0B |= (1 << CS02) | (1 << CS00);
	TCCR0B &= ~(1 << CS01);

	TCCR0A &= ~(1 << COM0A0) & ~(1 << COM0A1);//when compare match, does nothing except returning to BOTTOM

	OCR0A = 78;//setting max value of timer to (16000000 / 1024) / 200 to get 200Hz

	TIMSK1 |= (1 << OCIE1A);//enabling timer1 match interrupts
	TIMSK0 |= (1 << OCIE0A) | (1 << OCIE0B);//enabling OCR1A match interrupt
	sei(); //enabling interrupts
}

void	set_rgb(uint8_t r, uint8_t g, uint8_t b)
{

}

int main()
{
	init_rgb();
	  

	while (1)
	{
	}
	return 0;
}
