#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>

char	str[13] = "FULLRAINBOW";
char	final_str[13] = "FULLRAINBOW";
uint8_t	RGB[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
uint8_t pos = 0;
int8_t	dir = 1;


void	init_timer()
{
	PRR &= ~(1 << PRTIM1); //setting PRTIM1 to 0 to enable timer/counters1

	//TIMER1 CONFIG
	//sets the timer as mode 14 (compare mode FAST PWN) (p131-132 & p140)
	TCCR1A |= (1 << WGM11);
	TCCR1A &= ~(1 << WGM10);
	TCCR1B |= (1 << WGM12) | (1 << WGM13);

	//sets prescaler divisor of the timer1 at 1024(p143)
	TCCR1B |= (1 << CS12) | (1 << CS10);
	TCCR1B &= ~(1 << CS11);

	//do nothing when OCR1A compare (p140)
	TCCR1A &= ~(1 << COM1A0) & ~(1 << COM1A1);

	ICR1 = 312; //setting max value of timer1 to (16000000 / 1024) / 50 to get 50Hz(p131-132)
	OCR1A = ICR1 / 2;

	TIMSK1 |= (1 << OCIE1A);
	sei();
}

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

void	print_dec_value(uint16_t	nbr)
{
	if (nbr >= 0 && nbr <= 9 )
		uart_tx(nbr + '0');
	else
	{
		print_dec_value(nbr / 10);
		print_dec_value(nbr % 10);
	}
}

void print_hex_value(uint8_t data)
{
	char hex[3];
	hex[0] = "0123456789ABCDEF"[data >> 4];
	hex[1] = "0123456789ABCDEF"[data & 0x0F];
	// hex[2] = ' ';
	// hex[2] = '\r';
	// hex[3] = '\n';
	hex[2] = '\0';
	uart_printstr(hex);
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
			if (i < 12)
				str[i] = c;
			i++;
			uart_tx(c);
		}
		c = uart_rx();
	}
	if (i <= 12)
	{
		str[i] = 0;
		return (0);
	}
	uart_printstr("\r\nERROR: bad RGB value");
	return (1);
}

void	read_adc(uint8_t pin)
{
	while (ADCSRA & (1 << ADSC));//wait for the value

	PRR &= ~(1 << PRADC);//disabling power reduction

	ADMUX = pin;//select which pin to read, here the ADC0

	//setting voltage reference as internal 1.1V
	ADMUX |= (1 << REFS0);
	ADMUX &= ~(1 << REFS1);

	ADMUX |= (1 << ADLAR);//left adjust the result (push it to the left)	

	ADCSRA |= (1 << ADPS0) | (1 << ADPS1)| (1 << ADPS2);//sets the prescaler at 128

	ADCSRA |= (1 << ADEN);//turning ON the ADC

	ADCSRA |= (1 << ADSC);//start conversion	
}

void	spi_init()//initializing SPI as master
{
	PRR &= ~(1 << PRSPI);//disabling power reduction

	//configuring SPI pins
	DDRB |= (1 << PB2) | (1 << PB3) | (1 << PB5);//setting SS/MOSI/SCK to output
	DDRB &= ~(1 << PB4);//setting MISO to input

	SPCR |= (1 << SPE) | (1 << MSTR);//enabling SPI as master

	SPCR &= ~(1 << DORD);//sets data order as MSB first

	SPCR &= ~(1 << CPOL) & ~(1 << CPHA);//sets clock as mode 0

	//sets clock frequency as /16
	SPCR |= (1 << SPR0);
	SPCR &= ~(1 << SPR1);
}	

void	spi_send(uint8_t data)
{
	SPDR = data;//load data
	while (!(SPSR & (1 << SPIF)));//wait for data to be sent
}


void	start_frame()
{
	for (uint8_t i = 0; i < 4; i++)
		spi_send(0);
}

void	led_frame(uint8_t r, uint8_t g, uint8_t b)
{
	spi_send(0b11100010);
	spi_send(b);
	spi_send(g);
	spi_send(r);
}

void	end_frame()
{
	for (uint8_t i = 0; i < 4; i++)
		spi_send(255);
}

void	set_leds(uint8_t RGB[3][3])
{
	start_frame();
	
	for (uint8_t i = 0; i < 3; i++)
		led_frame(RGB[i][0], RGB[i][1], RGB[i][2]);
		
	end_frame();
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
			uart_printstr("\r\nBAD RGB VALUE");
			return (0);
		}
	}
	return result;
}

void	*ft_memset(void *s, int c, uint32_t n)
{
	uint32_t	i;
	char	*tmp;

	i = 0;
	tmp = s;
	while (i < n)
	{
		tmp[i] = (char) c;
		i++;
	}
	return (s);
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

void	ft_strlcpy(char *dst, const char *src, uint32_t size)
{
	uint32_t	i;

	i = 0;
	if (size)
	{
		while (i < (size - 1) && src[i])
		{
			dst[i] = src[i];
			i++;
		}
		dst[i] = 0;
	}
	return ;
}

uint32_t	ft_strlen(const char	*str)
{
	uint32_t	i;

	i = 0;
	while (str[i])
		i++;
	return (i);
}

void	set_rgb(uint8_t r, uint8_t g, uint8_t b)
{
	set_leds((uint8_t [3][3]){{r, g, b}, {r, g, b}, {r, g, b}});
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

ISR(TIMER1_COMPA_vect)
{
	if (compstr(final_str, "#FULLRAINBOW"))
	{
		if (pos == 255)
			dir = -1;
		else if (pos == 0)
			dir = 1;
		pos += dir;
		wheel(pos);
	}
	else
	{
		set_leds((uint8_t [3][3]){{RGB[0][0], RGB[0][1], RGB[0][2]}, {RGB[1][0], RGB[1][1], RGB[1][2]}, {RGB[2][0], RGB[2][1], RGB[2][2]}});
	}
}

bool is_hex(char c)
{
	if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))
	{
		return true;
	}
	else
	{
		return false;
	}
}

uint8_t	str_is_valid()
{
	if (str[0] != '#')
	{
		uart_printstr("\r\nERROR: bad RGB valuee");
		return (0);
	}
	else if (ft_strlen(str) != 9)
	{
		uart_printstr("\r\n");
		print_dec_value(ft_strlen(str));
		uart_printstr("\r\nERROR: bad RGB valueee");
		return (0);
	}
	for (uint8_t i = 1; i < 7; i++)
	{
		if (!is_hex(str[i]))
		{
			uart_printstr("\r\nERROR: bad RGB valueeee");
			return (0);
		}
	}
	if (compstr(str + 7, "D6"))
	{
		return (1);
	}
	if (compstr(str + 7, "D7"))
	{
		return (1);
	}
	if (compstr(str + 7, "D8"))
	{
		return (1);
	}
	uart_printstr("\r\nERROR: bad RGB valueeeee");
	return (0);
}

int main()
{
	uart_init();
	spi_init();
	init_timer();

	while (1)
	{
		uart_printstr("\r\nselect RGB: ");
		while (get_RGB())
			uart_printstr("\r\nselect RGB: ");
		uart_printstr("\r\n");
		uart_printstr(str);
		if (compstr(str, "#FULLRAINBOW"))
		{
			RGB[0][0] = 0;
			RGB[0][1] = 0;
			RGB[0][2] = 0;
			RGB[1][0] = 0;
			RGB[1][1] = 0;
			RGB[1][2] = 0;
			RGB[2][0] = 0;
			RGB[2][1] = 0;
			RGB[2][2] = 0;
			ft_strlcpy(final_str, str, 12);
		}
		else if (str_is_valid())
		{
			ft_strlcpy(final_str, str, 9);
			if (compstr(str + 7, "D6"))
			{
				uart_printstr("\r\nchanging D6");
				RGB[0][0] = atohex(final_str + 1);
				RGB[0][1] = atohex(final_str + 3);
				RGB[0][2] = atohex(final_str + 5);
			}
			if (compstr(str + 7, "D7"))
			{
				uart_printstr("\r\nchanging D7");
				RGB[1][0] = atohex(final_str + 1);
				RGB[1][1] = atohex(final_str + 3);
				RGB[1][2] = atohex(final_str + 5);
			}
			if (compstr(str + 7, "D8"))
			{
				uart_printstr("\r\nchanging D8");
				RGB[2][0] = atohex(final_str + 1);
				RGB[2][1] = atohex(final_str + 3);
				RGB[2][2] = atohex(final_str + 5);
			}
		}
	}
	return 0;
}
