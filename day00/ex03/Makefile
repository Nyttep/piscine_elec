
CC = avr-gcc
CFLAGS = -mmcu=atmega328p -DF_CPU=${F_CPU} -Os

NAME = main.hex

RM = rm -rf

SRCS_NAME =	main.c

SRCS_PATH =	srcs
SRCS = $(addprefix $(SRCS_PATH)/, $(SRCS_NAME))

INCS_NAME =	

INCS_PATH =	includes
INCS = $(addprefix $(INCS_PATH)/, $(INCS_NAME))

F_CPU="default"

all: hex flash

hex:
	${CC} ${CFLAGS} ${SRCS_NAME} -o main.bin && avr-objcopy -O ihex main.bin main.hex

flash:
	avrdude -c arduino -b ${F_CPU} -p ATmega328P -U flash:w:main.hex -P /dev/ttyUSB0

clean:
	${RM} main.bin && ${RM} ${NAME}

re: clean all

.PHONY : all clean fclean re directories
