#include <util/delay.h>
#include <util/twi.h>
#include <avr/io.h>
#include <rtc.h>
#include <uart.h>
#include <stdlib.h>
#include <stdbool.h>

#define RTC_ADDR 0b1010001

// RTC commands

#define Control_status_1_CMD 0x00 // Data order: TEST1 N STOP N TESTC N N N
#define Control_status_2_CMD 0x01 // Data order: N N N TI_TP AF TF AIE TIE

//  Time and date registers
#define VL_seconds_CMD 0x02		// Data order: VL SECONDS(0 to 59)
#define Minutes_CMD 0x03		// Data order: x MINUTES(0 to 59)
#define Hours_CMD 0x04			// Data order: x x HOURS(0 to 23)
#define Days_CMD 0x05			// Data order: x x DAYS(1 to 31)
#define Weekdays_CMD 0x06		// Data order: x x x x x WEEKDAYS(0 to 6)
#define Century_months_CMD 0x07 // Data order: C x x MONTHS(1 to 12)
#define Years_CMD 0x08			// Data order: YEARS(0 to 99)

static inline uint8_t bcdToDec(uint8_t val)
{
	// uint8_t dec = ((val & 0xF0) >> 4) * 10 + (val & 0x0F);
	// return dec;
	return ((val / 16 * 10) + (val % 16));
}

static uint8_t decToBcd(uint8_t val)
{
	// uint8_t bcd = ((val / 10) << 4) | (val % 10);
	// return bcd;
	return ((val / 10 * 16) + (val % 10));
}

void rtc_set_time(uint8_t sec, uint8_t min, uint8_t hour, uint8_t day, uint8_t month, bool century, uint8_t year)
{
	i2c_start(RTC_ADDR, I2C_WRITE);
	i2c_write(VL_seconds_CMD);
	// // Control_status_1
	// i2c_write(0x00);
	// // Control_status_2
	// i2c_write(0x00);
	// Time and date registers
	i2c_write(decToBcd(sec));
	i2c_write(decToBcd(min));
	i2c_write(decToBcd(hour));
	i2c_write(decToBcd(day));
	// Weekdays
	i2c_write(0x00);
	i2c_write((century << 7) | decToBcd(month));
	i2c_write(decToBcd(year));

	// Disable Timer Minutes
	i2c_write(0x80);
	// Disable Timer Hours
	i2c_write(0x80);
	// Disable Timer Days
	i2c_write(0x80);
	// Disable Timer Weekdays
	i2c_write(0x80);
	// Set Frequency to 32768 Hz e.g. for tuning
	i2c_write(0x80);
	// timer switched off
	i2c_write(0x0);

	i2c_stop();
}

void rtc_set_data(rtc_data data)
{
	rtc_set_time(data.sec, data.min, data.hour, data.day, data.month, data.century, data.year);
}

rtc_data rtc_get_data(void)
{
	rtc_data data = {0};
	i2c_start(RTC_ADDR, I2C_WRITE);
	i2c_write(VL_seconds_CMD);
	i2c_stop();
	i2c_start(RTC_ADDR, I2C_READ);
	i2c_read(ACK);
	data.sec = bcdToDec(TWDR & 0x7F); // Remove VL bit
	// data.sec = (((data.sec & 0b1110000) >> 4) * 10) + (data.sec & 0b1111); // Convert BCD to decimal
	i2c_read(ACK);
	data.min = bcdToDec(TWDR & 0x7F);
	// data.min = (((data.min & 0b1110000) >> 4) * 10) + (data.min & 0b1111); // Convert BCD to decimal
	i2c_read(ACK);
	data.hour = bcdToDec(TWDR & 0x3F);
	// data.hour = (((data.hour & 0b110000) >> 4) * 10) + (data.hour & 0b1111); // Convert BCD to decimal
	i2c_read(ACK);
	data.day = bcdToDec(TWDR & 0x3F);
	// data.day = (((data.day & 0b110000) >> 4) * 10) + (data.day & 0b1111); // Convert BCD to decimal
	// Skipping weekdays
	i2c_read(ACK);
	i2c_read(ACK);
	data.month = bcdToDec(TWDR & 0x1F);
	data.century = (TWDR & 0b10000000) >> 7;
	// data.month = (((data.month & 0b10000) >> 4) * 10) + (data.month & 0b1111); // Convert BCD to decimal
	i2c_read(NACK);
	data.year = bcdToDec(TWDR);
	// data.year = (((data.year & 0b11110000) >> 4) * 10) + (data.year & 0b1111); // Convert BCD to decimal
	i2c_stop();
	return data;
}

void uart_print_rtc(rtc_data data)
{
	uart_printdec(data.day);
	uart_tx('/');
	uart_printdec(data.month);
	uart_tx('/');
	uart_printdec(data.century ? 20 : 19);
	uart_printdec(data.year);
	uart_tx(' ');
	uart_printdec(data.hour);
	uart_tx(':');
	uart_printdec(data.min);
	uart_tx(':');
	uart_printdec(data.sec);
	uart_printnl();
}
