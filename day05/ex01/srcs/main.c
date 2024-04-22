#include <avr/io.h>
#include <stdlib.h>
#include <avr/eeprom.h>

#define COUNTERS_ADDR (uint8_t *)4

void render_num(uint8_t *counter)
{
	char pins[4] = {PB0, PB1, PB2, PB4};
	
	int i = 3;
	while (i >= 0)
	{
		if (eeprom_read_byte(counter) & (1 << i))
			PORTB |= (1 << pins[i--]);
		else
			PORTB &= ~(1 << pins[i--]);
	}
}

int main()
{
	DDRB |= 1 | (1 << PB1) | (1 << PB2) | (1 << PB4); //setting DDRB0 DDRB1 DDRB2 DDRB4 to 1 configuring it to output
	PORTB &= ~1 & ~(1 << PB1) & ~(1 << PB2) & ~(1 << PB4); //setting PORTB0 PORTB1 PORTB2 PORTB4 to 0 configuring it to output LOW

	DDRD = DDRD & ~(1 << PD2) & ~(1 << PD4); //setting DDRD2 DDR4 to 0 configuring it to input
	PORTD = PORTD | (1 << PD2) | (1 << PD4); //setting PORTD2 DDR4 to 1 activating the pull-up resistor

	char	button_state1 = 0;
	char	button_state2 = 0;
	uint_fast16_t counter = 0;

	while (1)
	{
		render_num(counter);
		if (button_state1 == 0)
		{
			if ((PIND & (1 << 2)) == 0) //checking if PIND2 is set to 0 (button is pressed)
			{
				button_state1 = 1;
				if (eeprom_read_byte(counter) < 15)
					eeprom_update_byte(counter, eeprom_read_byte(counter) + 1);
				else
					eeprom_update_byte(counter, 0);
			}
		if (button_state2 == 0)
		{
			if ((PIND & (1 << 4)) == 0) //checking if PIND4 is set to 0 (button is pressed)
			{
				button_state2 = 1;
				if (counter == 3)
					counter = 0;
				else
					counter++;
				eeprom_update_byte(COUNTERS_ADDR, counter + 1);
			}
		}
		}
		if (PIND & (1 << 2)) //checking if PIND2 is set to 1 (button is not pressed)
			button_state1 = 0;
		if (PIND & (1 << 4)) //checking if PIND2 is set to 1 (button is not pressed)
			button_state2 = 0;
	}
	return 0;
}
