#include <util/delay.h>

#define DDRB 0x24 //register DDRB's adress (p624 ATmega328 doc), used for configuring the pins as input(0) or output(1) (p85 ATmega328 doc)
#define PORTB 0x25 //register PORTB's adress (p624 ATmega328 doc), used for configuring the pins as HIGH(1) or LOW(0) when the pin is configured as output by DDRB (p85 ATmega doc)
#define DDRD 0x2A //register DDRD's adress (p624 ATmega328 doc), used for configuring the pins as input(0) or output(1) (p85 ATmega328 doc)
#define PORTD 0x2B //register PORTD's adress (p624 ATmega328 doc), used for activating the pull-up resistor(1)	or deactivating it(0) when the pin is configured as input by DDRB (p85 ATmega doc)
#define PIND 0x29 //register PIND's adress (p624 ATmega328 doc), used for reading the pin state

int main()
{
	char*	ddrb = (char*)DDRB;
	char*	portb = (char*)PORTB;

	char*	ddrd = (char*)DDRD;
	char*	portd = (char*)PORTD;
	char*	pind = (char*)PIND;
	
	*ddrb = *ddrb | 1; //setting DDRB0 to 1 configuring it to output
	*portb = *portb & ~1; //setting PORTB0 to 0 configuring it to output LOW

	*ddrd = *ddrd & ~(1 << 2); //setting DDRD2 to 0 configuring it to input
	*portd = *portd | (1 << 2); //setting PORTD2 to 1 activating the pull-up resistor

	int button_state = 0;
	while (1)
	{
		while (button_state == 0)
		{
			if ((*pind & (1 << 2)) == 0) //checking if PIND2 is set to 0 (button is pressed)
			{
				button_state = 1;
				*portb = *portb ^ 1; //switching state of PORTB0	
				_delay_ms(100);
			}
		}
		if (*pind & (1 << 2)) //checking if PIND2 is set to 1 (button is not pressed)
			button_state = 0;
	}
	return 0;
}
