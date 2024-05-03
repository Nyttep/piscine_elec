#ifndef SEGMENT7_H
#define SEGMENT7_H

#include <avr/io.h>
#include <i2c.h>
#include <stdbool.h>

void seg7_display(uint8_t pos, uint8_t n, bool point);
void seg7_display_number(uint16_t number, bool middle_point);
void seg7_init();

void seg7_turnall(bool turn);

#endif
