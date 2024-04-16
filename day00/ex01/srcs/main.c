#define DDRB 0x24 //register DDRB's adress (p624 ATmega328 doc), used for configuring the pins as input(0) or output(1) (p85 ATmega328 doc)
#define PORTB 0x25 //register PORTB's adress (p624 ATmega328 doc), used for configuring the pins as HIGH(1) or LOW(0) when the pin is configured as output by DDRB (p85 ATmega doc)

int main()
{
	char*	ddrb = (char*)DDRB;
	char*	portb = (char*)PORTB;
	
	*ddrb = *ddrb ^ 1; //setting DDRB1 to 1 configuring it to output
	*portb = *portb ^ 1; //setting PORTB1 to 1 configuring it to output HIGH

	return 0;
}
