#pragma once
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

void timer0_init(uint8_t ms);
void timer0_COMP();

void timer1_init(uint16_t ms);
void timer1_OVF();
