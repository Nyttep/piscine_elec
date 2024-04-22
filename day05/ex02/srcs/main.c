#include <avr/io.h>
#include <stdbool.h>
#include <avr/eeprom.h>

#define COUNTERS_ADDR (uint8_t *)4
#define MAGIC_NUM	(uint16_t)12345

bool	safe_eeprom_write(void *buffer, size_t offset, size_t length)
{
	if (offset + 2 + length > 1023)
		return (1);
	if (eeprom_read_word(offset) != MAGIC_NUM)
		eeprom_write_word(offset, MAGIC_NUM);
	eeprom_update_block(buffer, offset + 2, length);
	return (0);
}

bool	safe_eeprom_read(void *buffer, size_t offset, size_t length)
{
	if (offset + 2 + length > 1023)
		return (1);
	if (eeprom_read_word(offset) != MAGIC_NUM)
		return (1);
	eeprom_read_block(buffer, offset + 2, length);
	return (0);
}

int main()
{
	while (1)
	{
	}
	return 0;
}
