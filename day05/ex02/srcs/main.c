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

bool	safe_eeprom_write(const void *buffer, size_t offset, size_t length)
{
	if (offset + 2 + length > 1023)
		return (1);
	if (eeprom_read_word((uint16_t*)(offset)) != MAGIC_NUM)
		eeprom_write_word((uint16_t*)(offset), MAGIC_NUM);
	for (uint16_t i = 0; i < length; i++)
	{
		eeprom_update_byte((uint8_t *) offset + 2, ((uint8_t*)buffer)[i]);
	}
	return (0);
}

bool	safe_eeprom_read(void *buffer, size_t offset, size_t length)
{
	if (offset + 2 + length > 1023)
		return (1);
	if (eeprom_read_word((uint16_t*)(offset)) != MAGIC_NUM)
		return (1);
	for (uint8_t i = 0; i < length; i++)
	{
		((uint8_t*)buffer)[i] = eeprom_read_byte(((uint8_t*)offset) + 2 + i);
	}
	return (0);
}

int main()
{
	uart_init();

	size_t length = 1;
	size_t offset = 123;
	uint8_t	data = 0x14;
	uint8_t	ret = 0x0;
	
	if (safe_eeprom_write((void*)&data, offset, length))
		uart_printstr("didnt find magic nmber in write");
	print_hex_value(data);
	if (safe_eeprom_read((void*)&ret, offset, length))
		uart_printstr("didnt find magic nmber in read");
	print_hex_value(data);
	print_hex_value(ret);
	while (1)
	{
	}
	return 0;
}
