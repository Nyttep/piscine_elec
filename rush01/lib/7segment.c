#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>
#include <i2c.h>
#include <stdbool.h>
#include <adc.h>

#define SW3 (1 << 0)

static const uint8_t slave_address = (0b0100 << 3) /* Fixed Address*/ | 0b000 /* A2 A1 A0 */;

extern volatile	uint8_t	states_expander;

void seg7_display(uint8_t pos, uint8_t n, bool point)
{
	static const uint8_t seg7[13] = {
		0b00111111, // 0
		0b00000110, // 1
		0b01011011, // 2
		0b01001111, // 3
		0b01100110, // 4
		0b01101101, // 5
		0b01111101, // 6
		0b00000111, // 7
		0b01111111, // 8
		0b01101111, // 9
		0b00000000, // Clear
		0b01111111, // ALL
		0b01000000, // -
	};

	i2c_start(slave_address, I2C_WRITE);
	i2c_write(0x02);
	states_expander |= 0xf0;
	i2c_write(states_expander);
	i2c_write(0);
	// i2c_stop();

	// i2c_start(slave_address, I2C_WRITE);
	// i2c_write(0x02);
	states_expander &= ~(0x80 >> pos);
	i2c_write(states_expander);
	i2c_write(seg7[n] | (point ? 0b10000000 : 0));
	i2c_stop();
}

void seg7_turnall(bool turn)
{
	i2c_start(slave_address, I2C_WRITE);
	i2c_write(0x02);
	i2c_write(0xf);
	i2c_write(turn ? (0xff) : 0);
	i2c_stop();
}

void seg7_display_number(uint16_t number, bool middle_point)
{
	if (number >= 10000)
	{
		seg7_display(0, 10, false);
		seg7_display(1, 10, false);
		seg7_display(2, 10, false);
		seg7_display(3, 10, false);
		return;
	}
	seg7_display(0, number % 10, middle_point);
	seg7_display(1, (number / 10) % 10, false);
	seg7_display(2, (number / 100) % 10, middle_point);
	seg7_display(3, (number / 1000) % 10, false);
	seg7_display(3, 10, false);
}

void seg7_init()
{
	// Read configuration register
	// i2c_start(slave_address, I2C_WRITE);
	// i2c_write(0x06);
	// i2c_stop();
	// i2c_start(slave_address, I2C_READ);
	// i2c_read(NACK);
	// uint8_t config = TWDR;
	// i2c_stop();

	// config = ~((~config) | 0b11110000); // Set as output
	// Set as output
	i2c_start(slave_address, I2C_WRITE);
	i2c_write(0x06);
	// i2c_write(config);	   // Set output digit selector
	i2c_write(0b00000001);	   // Set output digit selector
	i2c_write(0x00);	   // Set output all segments
	i2c_stop();
}
