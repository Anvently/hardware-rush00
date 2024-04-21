TARGET		=	main.hex

INCLUDES	=	./
SRCS_FOLDER	=	./
OBJS_FOLDER	=	.objs/

SRCS_FILES	=	main.c uart.c i2c.c log.c error_led.c game.c

SRCS		=	$(addprefix $(SRCS_FOLDER),$(SRCS_FILES))
OBJS		=	$(addprefix $(OBJS_FOLDER),$(SRCS_FILES:.c=.o))
BIN_TARGET	=	$(addprefix $(OBJS_FOLDER),$(TARGET:.hex=.bin))

PORT		=	/dev/ttyUSB0
BAUDRATE	=	115200 #serial frequency for USB port
CPU_FREQ	=	16000000UL #because board clock is 16Mhz (see schematic)

DEPS		=	Makefile i2c.h log.h uart.h error_led.h
DEFINE		=	-DF_CPU=$(CPU_FREQ)

CC			=	avr-gcc
CFlAGS		=	-I$(INCLUDES) $(DEFINE) -mmcu=atmega328p -O2

.PHONY		=	all clean hex flash re 

all: hex flash

hex: $(TARGET)

bin: $(BIN_TARGET)

#Compile .o
$(OBJS_FOLDER)%.o: $(SRCS_FOLDER)%.c $(DEPS)
	mkdir -p $(dir $@)
	$(CC) $(CFlAGS) -c -o $@ $<

#Compile .o to .bin
$(BIN_TARGET): $(OBJS) $(DEPS)
	mkdir -p $(dir $@)
	$(CC) $(CFlAGS) $(OBJS) -o $(BIN_TARGET)

#compile .bin to .hex
# -R .eeprom remove .eeprom from the output
$(TARGET): bin
	avr-objcopy -O ihex $(BIN_TARGET) $(TARGET)
	@echo "$(TARGET) has been successfully created."

flash: $(TARGET)
	-killall screen
	avrdude -c arduino -P $(PORT) -b $(BAUDRATE) -p atmega328p -D -U flash:w:$(TARGET):i
	-avrdude -c arduino -P $(PORT:USB0=USB1) -b $(BAUDRATE) -p atmega328p -D -U flash:w:$(TARGET):i
	-avrdude -c arduino -P $(PORT:USB0=USB2) -b $(BAUDRATE) -p atmega328p -D -U flash:w:$(TARGET):i

clean:
	@echo "\n-------------CLEAN--------------\n"
	rm -rfd $(TARGET) $(BIN_TARGET) $(OBJS_FOLDER)
	@echo "object files have been 	removed."

re: clean all
