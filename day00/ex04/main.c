#include <util/delay.h>

#define DDRB 0x24 //register DDRB's adress (p624 ATmega328 doc), used for configuring the pins as input(0) or output(1) (p85 ATmega328 doc)
#define PORTB 0x25 //register PORTB's adress (p624 ATmega328 doc), used for configuring the pins as HIGH(1) or LOW(0) when the pin is configured as output by DDRB (p85 ATmega doc)
#define DDRD 0x2A //register DDRD's adress (p624 ATmega328 doc), used for configuring the pins as input(0) or output(1) (p85 ATmega328 doc)
#define PORTD 0x2B //register PORTD's adress (p624 ATmega328 doc), used for activating the pull-up resistor(1)	or deactivating it(0) when the pin is configured as input by DDRB (p85 ATmega doc)
#define PIND 0x29 //register PIND's adress (p624 ATmega328 doc), used for reading the pin state
 
char*	ddrb = (char*)DDRB;
char*	portb = (char*)PORTB;

char*	ddrd = (char*)DDRD;
char*	portd = (char*)PORTD;
char*	pind = (char*)PIND;
		
void render_num(int counter)
{
	int bin[4] = {0, 0, 0, 0};

	for (int i = 0; counter > 0; i++)
	{
		bin[i] = counter % 2;
		counter /= 2;
	}

	for (int i = 3; i >= 0; i--)
	{
		if (bin[i] == 1)
		{
			if (i != 3)
				*portb |= (1 << i);
			else
				*portb |= (i << 4);
		}
		else
		{
			if (i != 3)
				*portb &= ~(1 << i);
			else
				*portb &= ~(i << 4);
		}
	}
}

int main()
{
	*ddrb = *ddrb | 1 | (1 << 1) | (1 << 2) | (1 << 4); //setting DDRB0 DDRB1 DDRB2 DDRB4 to 1 configuring it to output
	*portb = *portb & ~1 & ~(1 << 1) & ~(1 << 2) & ~(1 << 4); //setting PORTB0 PORTB1 PORTB2 PORTB4 to 0 configuring it to output LOW

	*ddrd = *ddrd & ~(1 << 2) & ~(1 << 4); //setting DDRD2 DDR4 to 0 configuring it to input
	*portd = *portd | (1 << 2) | (1 << 4); //setting PORTD2 DDR4 to 1 activating the pull-up resistor

	char	button_state1 = 0;
	char	button_state2 = 0;
	unsigned char	counter = 0;

	while (1)
	{
		if (button_state1 == 0)
		{
			if ((*pind & (1 << 2)) == 0) //checking if PIND2 is set to 0 (button is pressed)
			{
				button_state1 = 1;
				if (counter < 15)
					counter++;
				render_num(counter);
				_delay_ms(300);
			}
		}
		if (button_state2 == 0)
		{
			if ((*pind & (1 << 4)) == 0) //checking if PIND4 is set to 0 (button is pressed)
			{
				button_state2 = 1;
				if (counter > 0)
					counter--;
				render_num(counter);
				_delay_ms(300);
			}
		}
		if (*pind & (1 << 2)) //checking if PIND2 is set to 1 (button is not pressed)
			button_state1 = 0;
		if (*pind & (1 << 4)) //checking if PIND4 is set to 1 (button is not pressed)
			button_state2 = 0;
	}
	return 0;
}
