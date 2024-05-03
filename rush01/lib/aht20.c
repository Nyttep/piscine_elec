#include <util/delay.h>
#include <util/twi.h>
#include <avr/io.h>
#include <aht20.h>
#include <uart.h>
#include <stdlib.h>

#define AHT20_ADDR 0x38

// AHT20 commands

#define AHT20_STATUS_CMD 0x71
#define AHT20_STATUS_CALBIT 3

#define AHT20_INIT_CMD 0xBE
#define AHT20_INIT_PARAM 0x08
#define AHT20_INIT_PARAM2 0x00

#define AHT20_MESURE_CMD 0xAC
#define AHT20_MESURE_PARAM 0x33
#define AHT20_MESURE_PARAM2 0x00

#define AHT20_BUSY 7
void aht20_init(void)
{
	// Wait for AHT20 to wake up
	_delay_ms(50);
	// 5.4.1
	i2c_start(AHT20_ADDR, I2C_WRITE);
	i2c_write(AHT20_STATUS_CMD);
	i2c_stop();
	i2c_start(AHT20_ADDR, I2C_READ);
	i2c_read(NACK);
	i2c_stop();
	const uint8_t status = TWDR;

	if (!(status & _BV(AHT20_STATUS_CALBIT)))
	{
		uart_printstr("\n\rCalibration bit not set\n\r");
		// 5.4.2
		i2c_start(AHT20_ADDR, I2C_WRITE);
		i2c_write(AHT20_INIT_CMD);
		i2c_write(AHT20_INIT_PARAM);
		i2c_write(AHT20_INIT_PARAM2);
		_delay_ms(10);
	}
	// Wait for AHT20 to wake up
	i2c_stop();
	_delay_ms(50);
}

aht20_data aht20_mesure(void)
{
	aht20_data data = {0};
	i2c_start(AHT20_ADDR, I2C_WRITE);
	i2c_write(AHT20_MESURE_CMD);
	i2c_write(AHT20_MESURE_PARAM);
	i2c_write(AHT20_MESURE_PARAM2);
	i2c_stop();
	i2c_start(AHT20_ADDR, I2C_READ);
	i2c_read(ACK);
	while (!(TWDR & _BV(AHT20_BUSY)))
	{
	}
	i2c_read(ACK);
	data.humidity = (uint32_t)TWDR << 12;
	i2c_read(ACK);
	data.humidity |= (uint32_t)TWDR << 4;
	i2c_read(ACK);
	data.humidity |= (uint32_t)(TWDR & 0xF0) >> 4;
	data.temp = (uint32_t)(TWDR & 0x0f) << 16;
	i2c_read(ACK);
	data.temp |= (uint32_t)TWDR << 8;
	i2c_read(ACK);
	data.temp |= (uint32_t)TWDR;
	i2c_read(NACK);

	i2c_stop();
	return data;
}

void print_temp(uint32_t temp)
{
	float converted_temp = temp;
	converted_temp /= (float)(1L << 20);
	converted_temp *= 200.0;
	converted_temp -= 50.0;
	char float_str[8];
	uart_printstr("Temperature: ");
	dtostrf(converted_temp, 4, 2, float_str);
	uart_printstr(float_str);
	uart_printstr(".C");
}

void print_humi(uint32_t humi)
{
	float converted_humi = humi;
	converted_humi /= (float)(1L << 20);
	converted_humi *= 100.0;
	char float_str[8];
	uart_printstr("Humidity: ");
	dtostrf(converted_humi, 3, 1, float_str);
	uart_printstr(float_str);
	uart_printstr("%");
}

float calc_temp(uint32_t temp)
{
	float converted_temp = temp;
	converted_temp /= (float)(1L << 20);
	converted_temp *= 200.0;
	converted_temp -= 50.0;
	return (converted_temp);
}

float calc_humi(uint32_t humi)
{
	float converted_humi = humi;
	converted_humi /= (float)(1L << 20);
	converted_humi *= 100.0;
	return (converted_humi);
}