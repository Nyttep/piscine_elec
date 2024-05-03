#include "modes.h"
#include "timer.h"
#include "7segment.h"
#include "led_spi.h"

void	mode_4_setup()
{
	// mode_4();
	// set_leds_spi((uint8_t [3][3]){{100, 0, 0}, {100, 0, 0}, {100, 0, 0}});//set all spi leds as RED
}

void	mode_4()
{
	seg7_display(3, 12, 0);
	seg7_display(2, 4, 0);
	seg7_display(1, 2, 0);
	seg7_display(0, 12, 0);
	seg7_display(0, 10, 0);
}
