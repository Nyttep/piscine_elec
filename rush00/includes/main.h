#ifndef MAIN_H
#define MAIN_H

#define __AVR_ATmega328P__ 1

#ifndef F_CPU
#define F_CPU 16000000
#endif

#ifndef UART_BAUDRATE
#define UART_BAUDRATE 115200
#endif

#define CMD_LIGHT_D3_ON 0x01 // D4 on
#define CMD_LIGHT_D3_OFF 0x02 // D4 off
#define CMD_LIGHT_D2_ON 0x03 // D3 on
#define CMD_LIGHT_D2_OFF 0x04 // D3 off
#define CMD_LIGHT_D1_ON 0x05 // D2 off
#define CMD_LIGHT_D1_OFF 0x06 // D2 off
#define CMD_LIGHT_ALL_ON 0x07 // D3:1 off
#define CMD_LIGHT_ALL_OFF 0x08 // D3:1 off
#define CMD_START_BUTTON_RUSH 0x09 // start button rush
#define CMD_STOP_BUTTON_RUSH 0x0a // stop button rush and save button state
#define CMD_GET_ACK_WINNER 0x0b // send ACK if button has been pressed
#define CMD_RESET 0x0c // reset the game

#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include <stdbool.h>
#include <stdlib.h>

#define TOGGLE(x,y) (x) ^= (1 << (y))
#define SET(x,y) (x) |= (1 << (y))
#define RESET(x,y) (x) &= ~(1 << (y))

void	uart_tx(char c);
void	uart_init();
void	uart_printstr(const char* str);
void	print_hex_value(uint8_t data);

void	send_status();
bool	lost_arbitration(uint8_t status);
bool	i2c_response(uint8_t status_check);
void	i2c_init();
void	i2c_start(bool isWrite, uint8_t address);
void i2c_write(unsigned char data);
uint8_t	i2c_read(uint8_t last);
void	i2c_stop();

void	im_a_slave();
void	im_your_master();
void	manhunt();

uint8_t	read_command();
void	start_button_rush(bool is_looser);
void	start_button_rush_as_loser();

void	begin_game();
bool	exec_command(uint8_t command, bool is_looser);

#endif