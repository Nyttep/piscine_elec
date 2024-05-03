BIN = target/main.bin

SRCS = ./main.c \
./lib/i2c.c \
./lib/uart.c \
./lib/7segment.c \
./lib/timer.c \
./lib/adc.c \
./lib/mode_4.c \
./lib/led_spi.c \
./lib/rtc.c \
./mode_adc.c \
./lib/mode_aht20.c \
./set_date.c \
./lib/aht20.c

CC = avr-gcc
OBJCPY = avr-objcopy
CFLAGS = -mmcu=atmega328p -D F_CPU=16000000 -Wall -Wextra -Os -I./lib -MMD -MP

PORT = /dev/ttyUSB0
BAUD_RATE = 115200

OBJS = ${SRCS:%.c=%.o}
DEPS = ${OBJS:.o=.d}

all: hex flash

hex: ${BIN}.hex

%.o: %.c
	${CC} ${CFLAGS} -o $@ -c $<

${BIN}: ${OBJS}
	mkdir -p $(@D)
	${CC} ${CFLAGS} -o $@ $^

${BIN}.hex: ${BIN}
	${OBJCPY} -O ihex $< ${BIN}.hex

flash: ${BIN}.hex
	avrdude -c arduino -p m328p -P ${PORT} -b ${BAUD_RATE} -U flash:w:${BIN}.hex:i

clean:
	rm -rf ./target $(OBJS) $(DEPS)


screen: all
	screen ${PORT} ${BAUD_RATE}

-include $(DEPS)

.PHONY: all hex flash clean screen
