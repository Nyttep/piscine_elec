#include <avr/io.h>
#include <stdbool.h>
#include <avr/eeprom.h>

#define COUNTERS_ADDR (uint8_t *)4
#define MAGIC_NUM	(uint16_t)12345

void	uart_tx(char c)
{
	while (!(UCSR0A & (1 << UDRE0)));//wait for empty transmit buffer
	UDR0 = c;//send character
}

void	uart_printstr(const char* str)
{
	for (uint16_t i = 0; str[i]; i++)
	{
		while (!(UCSR0A & (1 << UDRE0)));//wait for empty transmit buffer
		UDR0 = str[i];//send character
	}
}

void	uart_init()
{
	PRR &= ~(1 << PRUSART0);//making sure the USART0 is awake (p54)

	UCSR0C &= ~(1 << UMSEL00) & ~(1 << UMSEL01);// setting aynchronous mode
	UCSR0A |= (1 << U2X0);//setting clock as double speed
	//setting UBRR0 to have a baudrate of 115200
	UBRR0L = (uint8_t)(16 & 0xFF);
	UBRR0H = (uint8_t)(16 >> 8);

	//setting parity mode to no parity
	UCSR0C &= ~(1 << UPM00) & ~(1 << UPM01);

	//setting character size as 8
	UCSR0B &= ~(1 << UCSZ02);
	UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);

	UCSR0C &= ~(1 << USBS0);//setting 1 ending bit

	UCSR0B |= (1 << TXEN0);//enabling transmission
}

void print_hex_value(uint8_t data)
{
	char hex[4];
	hex[0] = "0123456789ABCDEF"[data >> 4];
	hex[1] = "0123456789ABCDEF"[data & 0x0F];
	hex[2] = ' ';
	// hex[2] = '\r';
	// hex[3] = '\n';
	hex[3] = '\0';
	uart_printstr(hex);
}

bool	eepromalloc_write(uint16_t id, void* buffer, uint16_t length)
{
	uint16_t	old_id = 0;
	if (id == 0)
		return (1);
	if (6 + length >= 1023)
		return (1);
	for (uint16_t i = 0; i < 1023;)
	{
		if (eeprom_read_word((uint16_t*)i) != MAGIC_NUM)
		{
			if (i + 6 + length >= 1023)
				return (1);
			eeprom_write_word((uint16_t*)i, MAGIC_NUM);
			eeprom_update_word((uint16_t*)(i + 2), id);
			eeprom_update_word((uint16_t*)(i + 4), length);
			eeprom_update_block(buffer, (uint16_t*)(i + 6), length);
			if (old_id != 0)
				eeprom_write_word((uint16_t*)i, (uint16_t)0);
			return (0);
		}
		i += 2;
		if (eeprom_read_word((uint16_t*)i) == id || eeprom_read_word((uint16_t*)i) == 0)
		{
			i += 2;
			if (eeprom_read_word((uint16_t*)i) <= length)
			{
				if (i + 2 + length >= 1023)
					return (1);
				eeprom_update_word((uint16_t*)i, length);
				eeprom_update_block(buffer, (uint16_t*)(i + 2), length);
				if (old_id != 0)
					eeprom_write_word((uint16_t*)i, (uint16_t)0);
				return (0);
			}
			if (eeprom_read_word((uint16_t*)i) != 0)
				old_id = i - 2;
		}
		else
			i += 2;
		i += eeprom_read_word((uint16_t*)i) + 2;
	}
	return (1);
}

bool	eepromalloc_read(uint16_t id, void* buffer, uint16_t length)
{
	for (uint16_t i = 0; i < 1023;)
	{
		if (eeprom_read_word((uint16_t*)i) == MAGIC_NUM)
		{
			i += 2;
			if (eeprom_read_word((uint16_t*)i) == id)
			{
				eeprom_read_block(buffer, (uint16_t*)(i + 4), length);
				return (0);
			}
			i += eeprom_read_word((uint16_t*)(i + 2)) + 2;
		}
		else
			i += 2;
	}
	return (1);
}

bool	eepromalloc_free(uint16_t id)
{
	for (uint16_t i = 0; i < 1023;)
	{
		if (eeprom_read_word((uint16_t*)i) == MAGIC_NUM)
		{
			i += 2;
			if (eeprom_read_word((uint16_t*)i) == id)
			{
				eeprom_write_word((uint16_t*)(i - 2), (uint16_t)0);
				return (0);
			}
			i += eeprom_read_word((uint16_t*)(i + 2)) + 2;
		}
		else
			i += 2;
	}
	return (1);
}

int main()
{
	uart_init();

	size_t length = 1;
	size_t offset = 123;
	uint16_t	data = 0x20;
	uint16_t	ret = 0x0;
	
	uart_printstr("DATA: init : ");
	print_hex_value(data);
	uart_printstr("\r\n");
	if (eepromalloc_write(3, (void*)&data, 2))
		uart_printstr("write error\r\n");
	uart_printstr("DATA: after write : ");
	print_hex_value(data);
	uart_printstr("\r\n");
	if (eepromalloc_read(3, (void*)&ret, 2))
		uart_printstr("read error\r\n");
	uart_printstr("RET: after read 1 : ");
	print_hex_value(ret);
	uart_printstr("\r\n");
	if (eepromalloc_read(2, (void*)&ret, length))
		uart_printstr("read error\r\n");
	uart_printstr("RET: after read 2 : ");
	print_hex_value(ret);
	uart_printstr("\r\n");
	if (eepromalloc_free(1))
		uart_printstr("free error\r\n");
	if (eepromalloc_read(1, (void*)&ret, length))
		uart_printstr("read error\r\n");
	uart_printstr("RET: after read/free : ");
	print_hex_value(ret);
	uart_printstr("\r\n");
	while (1)
	{
	}
	return 0;
}
