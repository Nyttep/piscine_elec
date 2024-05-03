#include <avr/io.h>
#include <util/delay.h>
#include <i2c.h>
#include <uart.h>
#include <adc.h>
#include <7segment.h>
#include <timer.h>
#include <aht20.h>
#include <rtc.h>

#include "led_spi.h"
#include "modes.h"

// D9 P15
// D10 P16
// D11 P17

#define D9 (1 << 3)
#define D10 (1 << 2)
#define D11 (1 << 1)
#define MAX_MODE 11

#define SW1 (1 << 2)
#define SW2 (1 << 4)
#define SW3 0

volatile uint16_t displed_value = 0;
volatile bool display_point = false;

volatile uint8_t current_mode = 0;

volatile aht20_data aht20 = {0};
volatile rtc_data time_data = {0};

volatile uint8_t	states_expander = 0;
volatile uint8_t	seconds_passed = 0;

bool read_expander0(uint8_t port);
void set_expander0(uint8_t port);
void clear_expander0(uint8_t port);

void	led_buttons()
{
	uint8_t	button_state1 = 0;
	uint8_t	button_state2 = 0;
	uint8_t	button_state3 = 0;

	if (button_state1 == 0)
	{
		if (!(PIND & SW1))
		{
			button_state1 = 1;
			set_expander0(D9);
		}
	}
	if (PIND & SW1)
	{
		button_state1 = 0;
		clear_expander0(D9);
	}

	if (button_state2 == 0)
	{
		if (!(PIND & SW2))
		{
			button_state2 = 1;
			set_expander0(D10);
		}
	}
	if (PIND & SW2)
	{
		button_state2 = 0;
		clear_expander0(D10);
	}

	if (button_state3 == 0)
	{
		if (!read_expander0(SW3))
		{
			button_state3 = 1;
			set_expander0(D11);
		}
	}
	if (read_expander0(SW3))
	{
		button_state3 = 0;
		clear_expander0(D11);
	}
}



void firmware_bootup()
{
	// Clear RGB LEDs
	clear_leds_spi();
	// Turn on D1, D2, D3, D4 for 3 sec
	DDRB |= _BV(PIN4);
	PORTB = (_BV(PIN0) | _BV(PIN1) | _BV(PIN2) | _BV(PIN4));
	seg7_turnall(true);
	timer1_init(1000);
	timer1_OVF();
	seconds_passed = 0;
	while (seconds_passed <= 3)
	{
		led_buttons();
	}
	PORTB = 0;
	seg7_turnall(false);
	seconds_passed = 0;
	while (seconds_passed < 1)
	{
		led_buttons();
	}
}

ISR(TIMER1_OVF_vect)
{
	seconds_passed++;
	if (current_mode == 4)
	{
		static uint8_t color = 0;

		if (color == 0)
		{
			PORTD &= ~(1 << PD3) & ~(1 << PD6); // turn off D5 BLUE and GREEN
			PORTD |= (1 << PD5); // turn on D5 RED
			set_leds_spi((uint8_t[3][3]){{100, 0, 0}, {100, 0, 0}, {100, 0, 0}}); // set all spi leds as RED
		}
		else if (color == 1)
		{
			PORTD &= ~(1 << PD3) & ~(1 << PD5); // turn off D5 RED and BLUE
			PORTD |= (1 << PD6); // turn on D5 GREEN
			set_leds_spi((uint8_t[3][3]){{0, 100, 0}, {0, 100, 0}, {0, 100, 0}}); // set all spi leds as GREEN
		}
		else
		{
			PORTD &= ~(1 << PD5) & ~(1 << PD6); // turn off D5 RED and GREEN
			PORTD |= (1 << PD3); // turn on D5 BLUE
			set_leds_spi((uint8_t[3][3]){{0, 0, 100}, {0, 0, 100}, {0, 0, 100}}); // set all spi leds as BLUE
		}
		color = (color + 1) % 3;
	}
	else if (current_mode == 5 || current_mode == 6 || current_mode == 7)
	{
		aht20 = aht20_mesure();
	}
	else if (current_mode == 8 || current_mode == 9 || current_mode == 10 || current_mode == 11)
	{
		time_data = rtc_get_data();
		// uart_print_rtc(time_data);
	}
}

ISR(TIMER0_COMPA_vect)
{
	switch (current_mode)
	{
	case 0:
	{
		mode_0();
		break;
	}

	case 1:
	{
		mode_1();
		break;
	}

	case 2:
	{
		mode_2();
		break;
	}

	case 3:
	{
		mode_3();
		break;
	}

	case 4:
	{
		mode_4();
		break;
	}

	case 5:
	{
		mode_6();
		break;
	}

	case 6:
	{
		mode_7();
		break;
	}

	case 7:
	{
		mode_8();
		break;
	}
	// Time and date
	case 8:
	{
		displed_value = time_data.hour * 100 + time_data.min;
		display_point = true;
		break;
	}
	case 9:
	{
		displed_value = time_data.day * 100 + time_data.month;
		break;
	}
	case 10:
	{
		displed_value = (time_data.century ? 2000 : 1900) + time_data.year;
		break;
	}

	default:
	{
		break;
	}
	}

	if (current_mode != 4)
	{
		seg7_display_number(displed_value, display_point);
	}
}

bool	read_expander0(uint8_t pin)
{
	cli();
	i2c_start(0b00100000, I2C_WRITE);
	i2c_write(0x00);
	i2c_start(0b00100000, I2C_READ);
	i2c_read(NACK);
	uint8_t data = TWDR;
	i2c_stop();
	sei();
	return (data & (1 << pin));
}

void	set_expander0(uint8_t port)
{
	cli();
	i2c_start(0b00100000, I2C_WRITE);
	i2c_write(2);
	states_expander &= ~port;
	i2c_write(states_expander);
	i2c_stop();
	sei();
}

void	clear_expander0(uint8_t port)
{
	cli();
	i2c_start(0b00100000, I2C_WRITE);
	i2c_write(2);
	states_expander |= port;
	i2c_write(states_expander);
	i2c_stop();
	sei();
}

void current_mode_display()
{
	PORTB = (current_mode & 0b111) | ((current_mode & 0b1000) << 1);
}

int main()
{
	uint8_t button_state1 = 0;
	uint8_t button_state2 = 0;
	uint8_t button_state3 = 0;

	// Set D1, D2, D3, D4 as output
	// DDRB = _BV(PIN0) | _BV(PIN1) | _BV(PIN2) | _BV(PIN4);

	//setting LED D5 as output
	DDRD |= (1 << PD3) | (1 << PD5) | (1 << PD6);
	DDRB |= _BV(PIN0) | _BV(PIN1) | _BV(PIN2) | _BV(PIN4);

	i2c_init();
	seg7_init();

	uart_init(UART_ALL);
	firmware_bootup();
	adc_init(ADC_NORMAL);
	aht20_init();

	timer0_init(21);
	timer0_COMP();

	current_mode = 0;
	current_mode_display();

	/*
		// Set the date and time for debug purposes
		const rtc_data new_date = {
			.sec = 8,
			.min = 56,
			.hour = 21,
			.day = 7,
			.month = 10,
			.year = 20,
			.century = true,
		};

		rtc_set_data(new_date);
	*/
	while (1)
	{
		if (button_state1 == 0)
		{
			if (!(PIND & SW1)) // checking if button 1 is pressed
			{
				button_state1 = 1;
				set_expander0(D9);
				cli();
				if (current_mode == MAX_MODE - 1)
					current_mode = 0;
				else
					current_mode++;
				PORTD &= ~(1 << PD3) & ~(1 << PD5) & ~(1 << PD6); // turn off D5
				current_mode_display();
				clear_leds_spi();
				sei();
				display_point = false;
			}
		}

		if (button_state2 == 0)
		{
			if (!(PIND & SW2)) // checking if button 2 is pressed
			{
				button_state2 = 1;
				set_expander0(D10);
				if (current_mode == 0)
					current_mode = MAX_MODE - 1;
				else
					current_mode--;
				current_mode_display();
				cli();
				PORTD &= ~(1 << PD3) & ~(1 << PD5) & ~(1 << PD6); // turn off D5
				clear_leds_spi();
				sei();
				display_point = false;
			}
		}

		if (button_state3 == 0)
		{
			if (read_expander0(SW3) == 0) // checking if button 3 is pressed
			{
				button_state3 = 1;
				set_expander0(D11);
			}
		}

		if (PIND & SW1) // checking if button 1 is not pressed
		{
			clear_expander0(D9);
			button_state1 = 0;
		}
		if (PIND & SW2) // checking if button 2 is not pressed
		{
			clear_expander0(D10);
			button_state2 = 0;
		}
		if (read_expander0(SW3)) // checking if button 3 is not pressed
		{
			clear_expander0(D11);
			button_state3 = 0;
		}
	}
}
