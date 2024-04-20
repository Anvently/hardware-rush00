#include <log.h>
#include <error_led.h>
#include <uart.h>

void	print(const char* str, uint8_t newline)
{
	if (uart_init(UART_MODE_ASYNC | UART_TX_ENABLE, UART_BAUDRATE, 
			UART_DATA_BITS, UART_PARITY_BIT_E, UART_STOP_BITS))
	{
		error();
		return;
	}
	uart_printstr(str, newline);
}

void	LOGI(const char* str)
{
	if (LOG_LVL < LOG_INFO)
		return;
	print(str, 1);
}

void	LOGE(const char* str)
{
	if (LOG_LVL < LOG_ERROR)
		return;
	print(str, 1);
}

void	LOGD(const char* str)
{
	if (LOG_LVL < LOG_DEBUG)
		return;
	print(str, 1);
}

void	printHexa(uint8_t c)
{
	if (uart_init(UART_MODE_ASYNC | UART_TX_ENABLE, UART_BAUDRATE, 
			UART_DATA_BITS, UART_PARITY_BIT_E, UART_STOP_BITS))
	{
		error();
		return;
	}
	uart_printstr("0x", 0);
	uart_send(HEXCODE[c >> 4]);
	uart_send(HEXCODE[c & 0b00001111]);
}

void	printHexa16(uint16_t data)
{
	if (uart_init(UART_MODE_ASYNC | UART_TX_ENABLE, UART_BAUDRATE, 
			UART_DATA_BITS, UART_PARITY_BIT_E, UART_STOP_BITS))
	{
		error();
		return;
	}
	uart_printstr("0x", 0);
	for (uint8_t i = 16; i > 0 ; i -= 8)
	{
		uart_send(HEXCODE[(data >> (i - 12)) & 0x0F]);
		uart_send(HEXCODE[(data >> (i - 8)) & 0x0F]);
	}
}

void	printHexa32(uint32_t data)
{
	if (uart_init(UART_MODE_ASYNC | UART_TX_ENABLE, UART_BAUDRATE, 
			UART_DATA_BITS, UART_PARITY_BIT_E, UART_STOP_BITS))
	{
		error();
		return;
	}
	uart_printstr("0x", 0);
	for (uint8_t i = 32; i > 0 ; i -= 8)
	{
		uart_send(HEXCODE[(data >> (i - 12)) & 0x0F]);
		uart_send(HEXCODE[(data >> (i - 8)) & 0x0F]);
	}
}

void	printHexa64(uint64_t data)
{
	if (uart_init(UART_MODE_ASYNC | UART_TX_ENABLE, UART_BAUDRATE, 
			UART_DATA_BITS, UART_PARITY_BIT_E, UART_STOP_BITS))
	{
		error();
		return;
	}
	uart_printstr("0x", 0);
	for (uint8_t i = 64; i > 0 ; i -= 8)
	{
		uart_send(HEXCODE[(data >> (i - 12)) & 0x0F]);
		uart_send(HEXCODE[(data >> (i - 8)) & 0x0F]);
	}
}
