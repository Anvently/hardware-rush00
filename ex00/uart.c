#include <uart.h>

static uint8_t	isInit = FALSE; 

/// @brief Configure UART
/// @param mode uart mode, 4 last bits set the synchronisation mode : ASYNC, SYNC, MASTER.
/// Bit 5 set TX enable, bit 4 set RX enable
/// @param baudrate target baudrate
/// @param dataBits number of bit inside data frame, from 5 to 8 (or 9 if no parityBit)
/// @param parityBit ```0``` or ```1``` to toggle parityBit usage
/// @param stopBits number of stop bit, from ```1``` to ```2```
uint8_t	uart_init(uint8_t mode, uint32_t baudrate, uint8_t dataBits, uint8_t parityBit, uint8_t stopBits)
{
	if (isInit)
		return (0);
	if ((dataBits < 5 || (dataBits > (8 + !parityBit)))
		|| (stopBits > 2))
		return (1);

	//Set uart synchronisation mode
	UCSR0C |= ((mode & 0b00001111) << UMSEL00);

	//Set number of data bits
	if (dataBits == 9) //If 9 bits, 2 register to set
	{
		UCSR0C |= (0b11 << UCSZ00);
		UCSR0B |= (1 << UCSZ02);
	}
	else
		UCSR0C |= ((dataBits - 5) << UCSZ00);
	
	//Set parity bit
	UCSR0C |= (parityBit << UPM00);

	//Set stopBits
	UCSR0C |= ((stopBits - 1) << USBS0); // - 1 because 2 stopbits = 01 and 1 stopbit = 00

	//Set baudrate
	uint16_t ubrr = 0;
	if ((mode & 0b00001111) == UART_MODE_ASYNC)
		ubrr = (CPU_FREQ + (8 * baudrate)) / (16 * baudrate) - 1;
	else if ((mode & 0b00001111) == UART_MODE_SYNC)
		ubrr = (CPU_FREQ + (4 * baudrate)) / (8 * baudrate) - 1;
	else if ((mode & 0b00001111) == UART_MODE_MASTER)
		ubrr = (CPU_FREQ + baudrate) / (2 * baudrate) - 1;
	if ((ubrr >> 12) > 0) //Check for overflow for ubrr
		return (1);
	UBRR0L = (uint8_t) ubrr;
	UBRR0H = (uint8_t)(ubrr >> 8);

	//set TX enable
	UCSR0B |= (((mode & UART_TX_ENABLE) >> 5) << TXEN0);

	//set RX enable
	UCSR0B |= (((mode & UART_RX_ENABLE) >> 4) << RXEN0);

	isInit = TRUE;

	return (0);
}

void	uart_send(uint16_t data)
{
	while (UART_TX_COMPLETE == FALSE); //While previous frame is not transferred yet
	
	if (UART_DATA_BITS == 9) //if frame have 9 data bits
	{
		UCSR0B &= ~(1 << TXB80); //Clear previous bit
		UCSR0B |= (((uint8_t) data & (0x0100)) << TXB80); //Read 9th bit of data and assign it to TXB80
	}
	UDR0 = (uint8_t) data;
}

void	uart_printstr(const char* str, uint8_t newline)
{
	for (uint8_t i = 0; str[i]; i++)
		uart_send((uint8_t) str[i]);
	if (newline)
	{
		uart_send('\r'); //to display correctly on screen
		uart_send('\n');
	}
}

/// @brief Return 
/// @param  
/// @return 
int32_t	uart_rx_16(void)
{
	while (UART_RX_COMPLETE == FALSE); //Wait until something was received

	uint8_t		status = UCSR0A;
	uint16_t	data;

	cli(); //Disable interrupt because 16bits manipulation is going to happen

	if (UART_DATA_BITS == 9)
		data = ((UCSR0B & RXB80) << 7) | UDR0; //Retrieve the 9th bit in UCSR0B and add it to the other 8 bits
	else
		data = UDR0; //Read RX_BUFFER

	sei();

	//Check error flags
	if (status & (1 << FE0))
		return (UART_ERROR_STOP_BIT);
	if (status & (1 << DOR0))
		return (UART_ERROR_DATA_LOSS);
	if (status & (1 << UPE0))
		return (UART_ERROR_PARITY);

	return (data);
}

char	uart_rx(void)
{
	return ((char) uart_rx_16());
}