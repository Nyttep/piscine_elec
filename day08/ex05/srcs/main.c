#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

uint8_t	RGB[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

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

void	read_adc(uint8_t pin)
{
	PRR &= ~(1 << PRADC);//disabling power reduction

	ADMUX = pin;//select which pin to read, here the ADC0

	//setting voltage reference as internal 1.1V
	ADMUX |= (1 << REFS0);
	ADMUX &= ~(1 << REFS1);

	ADMUX |= (1 << ADLAR);//left adjust the result (push it to the left)	

	ADCSRA |= (1 << ADPS0) | (1 << ADPS1)| (1 << ADPS2);//sets the prescaler at 128

	ADCSRA |= (1 << ADEN);//turning ON the ADC

	ADCSRA |= (1 << ADSC);//start conversion
	
	while (ADCSRA & (1 << ADSC));//wait for the value
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
	spi_send(0b11100100);
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

void	pot_gauge()
{
	uint8_t pot = ADCH;
	if (pot == 255)
		set_leds((uint8_t [3][3]){{255, 0, 0}, {0, 255, 0}, {0, 0, 255}});
	else if (pot >= (255 / 3) * 2)
		set_leds((uint8_t [3][3]){{255, 0, 0}, {0, 255, 0}, {0, 0, 0}});
	else if (pot >= (255 / 3))
		set_leds((uint8_t [3][3]){{255, 0, 0}, {0, 0, 0}, {0, 0, 0}});
	else
		set_leds((uint8_t [3][3]){{0, 0, 0}, {0, 0, 0}, {0, 0, 0}});
}

ISR(TIMER1_COMPA_vect)
{
	set_leds((uint8_t [3][3]){{RGB[0][0], RGB[0][1], RGB[0][2]}, {RGB[1][0], RGB[1][1], RGB[1][2]}, {RGB[2][0], RGB[2][1], RGB[2][2]}});
}

int main()
{
	DDRD = DDRD & ~(1 << PD2) & ~(1 << PD4); //setting DDRD2 DDR4 to 0 configuring it to input
	PORTD = PORTD | (1 << PD2) | (1 << PD4); //setting PORTD2 DDR4 to 1 activating the pull-up resistor

	uart_init();
	spi_init();
	init_timer();

	char	button_state1 = 0;
	char	button_state2 = 0;
	uint8_t	color_select = 0;
	uint8_t	led_select = 0;
	while (1)
	{
		if (button_state1 == 0)
		{
			if ((PIND & (1 << 2)) == 0) //checking if PIND2 is set to 0 (button is pressed)
			{
				button_state1 = 1;
				if (color_select == 2)
					color_select = 0;
				else
					color_select++;
				_delay_ms(300);
			}
		}
		if (button_state2 == 0)
		{
			if ((PIND & (1 << 4)) == 0) //checking if PIND4 is set to 0 (button is pressed)
			{
				button_state2 = 1;
				if (led_select == 2)
					led_select = 0;
				else
					led_select++;
				_delay_ms(300);
			}
		}
		if (PIND & (1 << 2)) //checking if PIND2 is set to 1 (button is not pressed)
			button_state1 = 0;
		if (PIND & (1 << 4)) //checking if PIND4 is set to 1 (button is not pressed)
			button_state2 = 0;
		read_adc(0b00000000);
		RGB[led_select][color_select] = ADCH;
	}
	return 0;
}
