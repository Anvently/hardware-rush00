#ifndef UART_H
# define UART_H

#include <avr/interrupt.h>
#include <avr/io.h>

#ifndef CPU_FREQ
 #define CPU_FREQ 16000000
#endif

#ifndef TRUE
 #define TRUE 1
#endif

#ifndef FALSE
 #define FALSE 0
#endif

#define UART_MODE_ASYNC 0
#define UART_MODE_SYNC 1
#define UART_MODE_MASTER 3

#define UART_PARITY_MODE_DISABLE 0
#define UART_PARITY_MODE_ODD 2
#define UART_PARITY_MODE_EVEN 3

#define UART_TX_ENABLE 0b00100000
#define UART_RX_ENABLE 0b00010000

#define UART_TX_COMPLETE (UCSR0A & (1 << UDRE0))
#define UART_RX_COMPLETE (UCSR0A & (1 << RXC0))

#define UART_MODE UART_MODE_ASYNC
#define UART_PARITY_BIT_E FALSE
#define UART_BAUDRATE 115200
#define UART_DATA_BITS 8
#define UART_STOP_BITS 1

#define UART_ERROR_PARITY -1
#define UART_ERROR_STOP_BIT -2
#define UART_ERROR_DATA_LOSS -3

uint8_t	uart_init(uint8_t mode, uint32_t baudrate, uint8_t dataBits, uint8_t parityBit, uint8_t stopBits);

void	uart_send(uint16_t data);

int32_t	uart_rx_16(void);
char	uart_rx(void);

void	uart_printstr(const char* str, uint8_t newline);

#endif
