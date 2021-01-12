#ifndef __I2C_H
#define __I2C_H

#include "delay.h"
#include "stm32f4xx.h"
#include "usart.h"

#define SI4703_I2C_8BIT_ADDRESS 0x20

void i2c_init(void);
void i2c_read_many(uint8_t address, uint8_t *result, uint8_t length);
void i2c_start(uint8_t type);
void i2c_stop(void);
void i2c_write(uint8_t data);
uint8_t i2c_read_ack(void);
uint8_t i2c_read_nack(void);

#endif /* ifndef __I2C_H */
