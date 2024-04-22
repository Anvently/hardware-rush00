#ifndef I2C_H
# define I2C_H

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/twi.h>
#include <error_led.h>
#include <log.h>

#ifndef CPU_FREQ
 #define CPU_FREQ 16000000
#endif

#ifndef TRUE
 #define TRUE 1
#endif

#ifndef FALSE
 #define FALSE 0
#endif

#define I2C_PRESCALER 1

#define I2C_MODE_TR 0
#define I2C_MODE_LVL 1
#define I2C_MODE_TX 0
#define I2C_MODE_RX 1
#define I2C_MODE_MASTER 0
#define I2C_MODE_SLAVE 1

#define I2C_MODE_MASTER_TX 0
#define I2C_MODE_MASTER_RX 1
#define I2C_MODE_SLAVE_TX 2
#define I2C_MODE_SLAVE_RX 3

#define I2C_ENABLE_GC (1 << 7)

#define I2C_ERROR_UNLIKELY -1
#define I2C_ERROR_PARAMS -2

#define I2C_READY (TWCR & (1 << TWINT))

int8_t	i2c_init(uint32_t frequency, uint8_t address, uint8_t mode);
int8_t	i2c_start(uint8_t address, uint8_t mode);
int8_t	i2c_stop(void);
int8_t	i2c_write(uint8_t data);
int8_t	i2c_read(uint8_t* dest, uint8_t stop);

#endif