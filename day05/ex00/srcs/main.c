#include <avr/io.h>
#include <stdlib.h>
#include <avr/eeprom.h>

void render_num()
{
	char pins[4] = {PB0, PB1, PB2, PB4};
	
	int i = 3;
	while (i >= 0)
	{
		if (eeprom_read_byte(0) & (1 << i))
			PORTB |= (1 << pins[i--]);
		else
			PORTB &= ~(1 << pins[i--]);
	}
}

int main()
{
	DDRB |= 1 | (1 << PB1) | (1 << PB2) | (1 << PB4); //setting DDRB0 DDRB1 DDRB2 DDRB4 to 1 configuring it to output
	PORTB &= ~1 & ~(1 << PB1) & ~(1 << PB2) & ~(1 << PB4); //setting PORTB0 PORTB1 PORTB2 PORTB4 to 0 configuring it to output LOW

	DDRD = DDRD & ~(1 << PD2); //setting DDRD2 DDR4 to 0 configuring it to input
	PORTD = PORTD | (1 << PD2); //setting PORTD2 DDR4 to 1 activating the pull-up resistor

	char	button_state1 = 0;
	uint8_t	counter = 0;

	while (1)
	{
		render_num();
		if (button_state1 == 0)
		{
			if ((PIND & (1 << 2)) == 0) //checking if PIND2 is set to 0 (button is pressed)
			{
				button_state1 = 1;
				if (eeprom_read_byte(0) < 15)
					eeprom_update_byte(0, eeprom_read_byte(0) + 1);
				else
					eeprom_update_byte(0, 0);
			}
		}
		if (PIND & (1 << 2)) //checking if PIND2 is set to 1 (button is not pressed)
			button_state1 = 0;
	}
	return 0;
}
