#include <avr/io.h>
#include <stdbool.h>
#include <avr/eeprom.h>

#define MAGIC_NUM	(uint16_t)12345

#define MAX_LEN_KEY 254
#define MAX_LEN_VALUE 1017
#define MAX_LEN_CMD 6

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

char	uart_rx()
{
	while (!(UCSR0A & (1 << RXC0)));
	return (UDR0);
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
	UCSR0B |= (1 << RXEN0);//enabling reception
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

uint32_t	hash(unsigned char *str)
{
    uint32_t hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

uint16_t strlen(char *str)
{
	uint16_t	i = 0;
	while (str[i])
		i++;
	return (i);
}

bool	is_my_id(char *id, uint16_t i)
{
	uint8_t	c;
	uint16_t	j = 0;
	c = eeprom_read_byte((uint8_t*)i);
	while (c && id[j])
	{
		if (c != id[j])
			return (0);
		j++;
		c = eeprom_read_byte((uint8_t*)i + j);
	}
	if (id[j] == 0 && c == 0)
		return (1);
	return (0);
}

void	write_id(char *id, uint16_t i)
{
	uint8_t	j = 0;
	for (; id[j]; j++)
	{
		eeprom_update_byte((uint8_t*)(i + j), id[j]);
	}
	eeprom_update_byte((uint8_t*)(i + j), id[j]);
}

uint16_t	after_next_zero(uint16_t i)
{
	i++;
	while (eeprom_read_byte((uint8_t*)i))
	{
		i++;
	}
	i++;
	return (i);
}

// bool	eepromalloc_write(char *id, void *buffer, uint16_t length)
// {
// 	uint16_t	old_id = 0;
// 	if (id[0] == 0)
// 		return (1);
// 	if (6 + length >= 1023)
// 		return (1);
// 	for (uint16_t i = 0; i < 1023;)
// 	{
// 		if (eeprom_read_word((uint16_t*)i) != MAGIC_NUM)
// 		{
// 			if (i + 2 + strlen(id) + 1 + length >= 1023)
// 				return (1);
// 			eeprom_write_word((uint16_t*)i, MAGIC_NUM);
// 			write_id(id, i + 2);
// 			eeprom_update_word((uint16_t*)(i + 2 + strlen(id) + 1), length);
// 			eeprom_update_block(buffer, (uint16_t*)(i + 4 + strlen(id) + 1), length);
// 			if (old_id != 0)
// 				eeprom_write_byte((uint8_t*)old_id, (uint8_t)0);
// 			uart_printstr("\r\nNEW KEY\r\n");
// 			return (0);
// 		}
// 		i += 2;
// 		if (is_my_id(id, i) || eeprom_read_byte((uint8_t*)i) == 0)
// 		{
// 			if (eeprom_read_byte((uint8_t*)i) == 0)
// 			{
// 				while(eeprom_read_byte((uint8_t*)i++) == 0);
// 			}
// 			else
// 			{
// 				old_id = i;
// 				i += strlen(id);
// 			}
// 			i += 1;
// 			if (eeprom_read_word((uint16_t*)i) <= length)
// 			{
// 				if (i + 2 + length >= 1023)
// 					return (1);
// 				eeprom_update_word((uint16_t*)i, length);
// 				eeprom_update_block(buffer, (uint16_t*)(i + 2), length);
// 				uart_printstr("\r\nOLD KEY\r\n");
// 				return (0);
// 			}
// 		}
// 		else
// 			i  = after_next_zero(i);
// 		i += eeprom_read_word((uint16_t*)i) + 2;
// 	}
// 	return (1);
// }

// bool	eepromalloc_read(char *id, void *buffer)
// {
// 	for (uint16_t i = 0; i < 1023;)
// 	{
// 		if (eeprom_read_word((uint16_t*)i) == MAGIC_NUM)
// 		{
// 			i += 2;
// 			if (is_my_id(id, i))
// 			{
// 				eeprom_read_block(buffer, (uint16_t*)(i + 2 + strlen(id) + 1), eeprom_read_word((uint16_t*)i + strlen(id) + 1));
// 				return (0);
// 			}
// 			i += eeprom_read_word((uint16_t*)(i + 2)) + 2;
// 		}
// 		else
// 			i = after_next_zero(i);
// 	}
// 	return (1);
// }

// bool	eepromalloc_free(char *id)
// {
// 	for (uint16_t i = 0; i < 1023;)
// 	{
// 		if (eeprom_read_word((uint16_t*)i) == MAGIC_NUM)
// 		{
// 			i += 2;
// 			if (is_my_id(id, i))
// 			{
// 				eeprom_write_byte((uint8_t*)(i), (uint16_t)0);
// 				return (0);
// 			}
// 			i += eeprom_read_word((uint16_t*)(i + strlen(id) + 1)) + 2;
// 		}
// 		else
// 			i = after_next_zero(i);
// 	}
// 	return (1);
// }

bool	eepromalloc_write(uint32_t id, void* buffer, uint16_t length)
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
			if (i + 8 + length >= 1023)
				return (1);
			eeprom_write_word((uint16_t*)i, MAGIC_NUM);
			eeprom_write_dword((uint32_t*)(i + 2), id);
			eeprom_update_word((uint16_t*)(i + 6), length);
			eeprom_update_block(buffer, (uint16_t*)(i + 8), length);
			if (old_id != 0)
				eeprom_write_dword((uint32_t*)old_id, (uint32_t)0);
			return (0);
		}
		i += 2;
		if (eeprom_read_dword((uint32_t*)i) == id || eeprom_read_dword((uint32_t*)i) == 0)
		{
			eeprom_update_dword((uint32_t*)i, id);
			i += 4;
			if (eeprom_read_word((uint16_t*)i) >= length)
			{
				if (i + 2 + length >= 1023)
					return (1);
				eeprom_update_word((uint16_t*)i, length);
				eeprom_update_block(buffer, (uint16_t*)(i + 2), length);
				if (old_id != 0)
					eeprom_write_dword((uint32_t*)old_id, (uint32_t)0);
				return (0);
			}
			if (eeprom_read_dword((uint32_t*)(i - 4)) != 0)
				old_id = i - 4;
		}
		else
			i += 4;
		i += eeprom_read_word((uint16_t*)i) + 2;
	}
	return (1);
}

bool	eepromalloc_read(uint32_t id, void* buffer)
{
	for (uint16_t i = 0; i < 1023;)
	{
		if (eeprom_read_word((uint16_t*)i) == MAGIC_NUM)
		{
			uart_printstr("\r\nfound MN");
			i += 2;
			if (eeprom_read_dword((uint32_t*)i) == id)
			{
				eeprom_read_block(buffer, (uint16_t*)(i + 6), eeprom_read_word((uint16_t*)(i + 4)));
				return (0);
			}
			i += eeprom_read_word((uint16_t*)(i + 4)) + 4;
		}
		else
			i += 2;
	}
	return (1);
}

bool	eepromalloc_free(uint32_t id)
{
	for (uint16_t i = 0; i < 1023;)
	{
		if (eeprom_read_word((uint16_t*)i) == MAGIC_NUM)
		{
			i += 2;
			if (eeprom_read_dword((uint32_t*)i) == id)
			{
				eeprom_write_dword((uint32_t*)(i), (uint32_t)0);
				return (0);
			}
			i += eeprom_read_word((uint16_t*)(i + 4)) + 2;
		}
		else
			i += 2;
	}
	return (1);
}

bool	get_args(char* buff, uint16_t max_len)
{
	uint32_t	i = 0;
	char	c = 0;
	bool	in_quotes = 0;

	c = uart_rx();
	while (i < UINT32_MAX)
	{
		if (c == 127)
		{
			if (i > 0)
			{
				i--;
				uart_printstr("\b \b");
			}
		}
		else if (i == 0 && c == '"')
		{
			uart_tx(c);
			in_quotes = 1;
		}
		else if (in_quotes && c == '"')
		{
			uart_tx(c);
			uart_tx(' ');
			break ;
		}
		else if (i == 0 && !in_quotes && c != '"')
		{
			uart_tx(c);
			uart_printstr("\r\nERROR: KEY and PAIR must start with '\"'\r\n");
			return (1);
		}
		else if (c == ' ' && i == 0)
		{
			uart_tx(c);
		}
		else if (c == '\r')
		{
			uart_printstr("\r\nERROR: dont press enter please its spaghetti code");
			return (1);
		}
		else
		{
			if (i < max_len)
				buff[i] = c;
			i++;
			uart_tx(c);
		}
		c = uart_rx();
	}
	if (i >= max_len)
	{
		uart_printstr("\r\nERROR: input too long (MAX_KEY = 254) (MAX_VALUE = 1018)");
		return (1);
	}
	buff[i] = 0;
	return (0);
}

bool	get_cmd(char* buff)
{
	uint32_t	i = 0;
	char	c = 0;

	c = uart_rx();
	while (i < UINT32_MAX)
	{
		if (c == 127)
		{
			if (i > 0)
			{
				i--;
				uart_printstr("\b \b");
			}
		}
		else if (c == ' ')
		{
			uart_tx(c);
			break ;
		}
		else if (c == '\r')
		{
			uart_printstr("\r\nERROR: dont press enter please its spaghetti code");
			return (1);
		}
		else
		{
			if (i < MAX_LEN_CMD)
				buff[i] = c;
			i++;
			uart_tx(c);
		}
		c = uart_rx();
	}
	if (i > MAX_LEN_CMD)
	{
		uart_printstr("\r\nERROR: incorrect CMD");
		return (1);
	}
	buff[i] = 0;
	return (0);
}

uint8_t compstr(char* str, char* checker)
{
	for (int i = 0; str[i]; i++)
	{
		if (str[i] != checker[i])
			return (0);
	}
	return (1);
}

int main()
{
	uart_init();

	uint8_t	key_buff[MAX_LEN_KEY + 1];
	uint8_t	value_buff[MAX_LEN_VALUE + 1];
	uint8_t	cmd_buff[MAX_LEN_CMD + 1];

 	// for(int i = 0; i <= 1023; i++) 
	// {
	// 	eeprom_update_byte((uint8_t*)i , (uint8_t)0);
	// 	uart_printstr("i");
	// }
	// return (0);

	while (1)
	{
		uart_printstr("\r\nEEPROMalloc$ ");
		if (get_cmd(cmd_buff))
			continue;
		if (compstr(cmd_buff, "PRINT"))
		{
			uart_printstr("\r\n");
	 		for(int i = 0; i <= 200; i++) 
			{
				print_hex_value(eeprom_read_byte((uint8_t*)i));
				uart_printstr(" ");
			}
			uart_printstr("\r\n");
		}
		if (get_args(key_buff, MAX_LEN_KEY))
			continue;
		if (compstr(cmd_buff, "READ"))
		{
			if (eepromalloc_read(hash(key_buff), value_buff))
			{
				uart_printstr("\r\n");
				continue ;
			}
			uart_printstr("\r\n");
			uart_printstr(value_buff);
		}
		else if (compstr(cmd_buff, "WRITE"))
		{
			if (get_args(value_buff, MAX_LEN_VALUE))
				continue;
			if (eepromalloc_write(hash(key_buff), value_buff, strlen(value_buff) + 1))
			{
				uart_printstr("\r\nERROR: no space left on device");
				continue ;
			}
			uart_printstr("\r\nDONE");
		}
		else if (compstr(cmd_buff, "FORGET"))
		{
			if (eepromalloc_free(hash(key_buff)))
			 {
				uart_printstr("\r\nERROR: not found");
				continue ;
			 }
			uart_printstr("\r\nDONE");
		}
		else
			uart_printstr("\r\nERROR: unknown CMD");
	}
	return 0;
}
