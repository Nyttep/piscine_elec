#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//red = PD5 green = PD6 blue = PD3

char	str_RGB[7];

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

uint8_t	get_RGB()
{
	int	i = 0;
	char	c = 0;

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
			if (i < 6)
				str_RGB[i] = c;
			i++;
			uart_tx(c);
		}
		c = uart_rx();
	}
	if (i ==  6)
		return (0);
	return (1);
}

void	init_rgb()
{
	DDRD |= (1 << PD3) | (1 << PD5) | (1 << PD6); //setting DDRB1 to 1 configuring it to output
	PORTD &= ~(1 << PD3) & ~(1 << PD5) & ~(1 << PD6); //setting PORTB1 to 0 configuring it to output LOW

	PRR &= ~(1 << PRTIM2) & ~(1 << PRTIM0); //setting PRTIM2/PRTIM0 to 0 to enable timer/counters2 and 0

	//TIMER0 CONFIG
	//sets the timer as mode 1 (PWN Phase correct) (p131-132 & p140)
	TCCR0A |= (1 << WGM00);
	TCCR0A &= ~(1 << WGM01) & ~(1 << WGM02);

	//sets prescaler divisor of the timer0 at 256(p143)
	TCCR0B &= ~(1 << CS00) & ~(1 << CS01);
	TCCR0B |= (1 << CS00);

	TCCR0A |= (1 << COM0A1) | (1 << COM0B1);
	TCCR0A &= ~(1 << COM0A0) & ~(COM0B0); 

	OCR0A = 0; //setting max value of timer1 to (16000000 / 255) / 244 to get 200Hz(p131-132)
	OCR0B = 0;

	//TIMER2 CONFIG

	TCCR2A |= (1 << WGM20);
	TCCR2A &= ~(1 << WGM21) & ~(1 << WGM22);

	//sets prescaler divisor of the timer2 at 256(p143)
	TCCR2B &= ~(1 << CS20);
	TCCR2B |= (1 << CS21) | (1 << CS22);

	TCCR2A |= (1 << COM2B1);
	TCCR2A &= ~(1 << COM2B0);

	OCR2B = 0; //setting max value of timer1 to (16000000 / 255) / 244 to get 200Hz(p131-132)

}

void	set_rgb(uint8_t r, uint8_t g, uint8_t b)
{
	OCR0A = g;
	OCR0B = r;
	OCR2B = b;
}

void	wheel(uint8_t pos) {
	pos = 255 - pos;
	if (pos < 85) {
		set_rgb(255 - pos * 3, 0, pos * 3);
	} else if (pos < 170) {
		pos = pos - 85;
		set_rgb(0, pos * 3, 255 - pos * 3);
	} else {
	pos = pos - 170;
	set_rgb(pos * 3, 255 - pos * 3, 0);
	}
}

uint8_t atohex(unsigned char c[2])
{
	uint8_t result = 0;
	for (int i = 0; i < 2; i++)
	{
		if (c[i] >= '0' && c[i] <= '9')
		{
			result = (result << 4) + (c[i] - '0');
		}
		else if (c[i] >= 'A' && c[i] <= 'F')
		{
			result = (result << 4) + (c[i] - 'A' + 10);
		}
		else if (c[i] >= 'a' && c[i] <= 'f')
		{
			result = (result << 4) + (c[i] - 'a' + 10);
		}
		else
		{
			uart_printstr("\r\nBAD RGB VALUE\r\n");
			return (0);
		}
	}
	return result;
}

int main()
{
	init_rgb();
	uart_init();
	  
	while (1)
	{
		uart_printstr("Enter RGB value: ");
		if (get_RGB())
		{
			uart_printstr("\r\nBAD RGB VAUE\r\n");
			continue;
		}
		set_rgb(atohex(str_RGB), atohex(str_RGB + 2), atohex(str_RGB + 4));
		uart_printstr("\r\n");
	}
	return 0;
}
