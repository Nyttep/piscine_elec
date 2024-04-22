
CC = avr-gcc
CFLAGS = -mmcu=atmega328p -Os -DF_CPU=${F_CPU}

OBJCPY = avr-objcopy
OFLAGS = -O ihex

DUDE = avrdude
DFLAGS = -c arduino -b 115200 -p ATmega328P -U flash:w:${HEX_NAME} -P /dev/ttyUSB0

NAME = main.bin

HEX_NAME = main.hex

RM = rm -rf

SRCS_NAME =	main.c

SRCS_PATH =	srcs
SRCS = $(addprefix $(SRCS_PATH)/, $(SRCS_NAME))

INCS_NAME =	

INCS_PATH =	includes
INCS = $(addprefix $(INCS_PATH)/, $(INCS_NAME))

F_CPU=16000000UL

all: hex flash

hex: ${HEX_NAME}

%.bin: ${SRCS_PATH}/%.c
	${CC} ${CFLAGS} ${SRCS} -o ${NAME}

${HEX_NAME}: ${NAME}
	avr-objcopy ${OFLAGS} $< $@

flash: ${HEX}
	${DUDE} ${DFLAGS}

screen: all
	screen /dev/ttyUSB0 115200

clean:
	${RM} ${NAME} && ${RM} ${HEX_NAME}

re: clean all

.PHONY : all clean re hex flash
