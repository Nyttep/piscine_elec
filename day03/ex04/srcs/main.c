#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define TBD 0
#define YEP 1
#define NOP -1
#define UNAME_LEN 6
#define PSWD_LEN 6


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

	UCSR0C &= ~(1 << UMSEL00) & ~(1 << UMSEL01);// setting asynchronous mode
	UCSR0A |= (1 << U2X0);//setting clock as normal
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

uint8_t check(char* str, char* checker)
{
	for (int i = 0; str[i]; i++)
	{
		if (str[i] != checker[i])
			return (NOP);
	}
	return (YEP);
}

uint8_t	get_pswd(char* uname)
{
	int	i = 0;
	char	c = 0;
	char	checker[PSWD_LEN] = "";

	c = uart_rx();
	while (c != '\r')
	{
		if (c == 127)
		{
			if (i > 0)
			{
				i--;
				uart_printstr("\b \b");
			}
		}
		else
		{
			if (i < PSWD_LEN)
				checker[i] = c;
			i++;
			uart_tx('*');
		}
		c = uart_rx();
	}
	if (i == PSWD_LEN - 1)
		return (check(uname, checker));
	return (NOP);
}


uint8_t	get_uname(char* uname)
{
	int	i = 0;
	char	c = 0;
	char	checker[UNAME_LEN] = "";

	c = uart_rx();
	while (c != '\r')
	{
		if (c == 127)
		{
			if (i > 0)
			{
				i--;
				uart_printstr("\b \b");
			}
		}
		else
		{
			if (i < UNAME_LEN)
				checker[i] = c;
			i++;
			uart_tx(c);
		}
		c = uart_rx();
	}
	if (i == UNAME_LEN - 1)
		return (check(uname, checker));
	return (NOP);
}

int main()
{
	DDRB |= (1 << PB0) | (1 << PB1); //setting DDRB1 to 1 configuring it to output
	PORTB &= ~(1 << PB0) & ~(1 << PB1); //setting PORTB1 to 0 configuring it to output LOW
	uart_init();
	sei();//enabling global interrupts

	char uname[UNAME_LEN] = "admin";
	char pswd[PSWD_LEN] = "admin";
	uint8_t	uname_check = TBD;
	uint8_t	pswd_check = TBD;
	while (1)
	{
		uart_printstr("Enter your login:\r\nusername: ");
		uname_check = get_uname(uname);
		uart_printstr("\r\npassword: ");
		pswd_check = get_pswd(uname);
		if (pswd_check == YEP && uname_check == YEP)
		{
			uart_printstr("\r\nure in !\r\n");
			break ;
		}
		else
		{
			uart_printstr("\r\nimpostor >:(\r\n");
		}
	}

	while (1)
	{
		PORTB ^= (1 << PB0) | (1 << PB1);
		_delay_ms(100);
	}

	return 0;
}
