#include "i2c.h"

void i2c_init() {
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; // init I2C on pins PB9 (SDA) & PB6 (SCK)
  RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;  // enable I2CS GPIO peripheral clock

  GPIOB->MODER &= ~((GPIO_MODER_MODER6) | (GPIO_MODER_MODER9));
  GPIOB->MODER |=
      (GPIO_MODER_MODER6_1) | (GPIO_MODER_MODER9_1); // set alternate function
  GPIOB->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR6 |
                      GPIO_OSPEEDER_OSPEEDR9); // set alternate function

  // GPIOB->PUPDR |= ((GPIO_PUPDR_PUPDR6) | (GPIO_PUPDR_PUPDR9));
  GPIOB->OTYPER |= (GPIO_OTYPER_OT_6) | (GPIO_OTYPER_OT_9);
  GPIOB->AFR[0] = 0x04000000;
  GPIOB->AFR[1] = 0x00000040;

  I2C1->CR1 |= (I2C_CR1_SWRST);
  I2C1->CR1 &= ~(I2C_CR1_SWRST);
  I2C1->CR1 &= ~(I2C_CR1_PE);   // disable peripheral
  I2C1->CR2 = (I2C_CR2_FREQ_3); // f_pclk/1e6 = 8;
  I2C1->CCR = 0x00000028;       // for 8MHz PCLK and 100kHz I2C SCK
  I2C1->TRISE = 0x00000009;     //
  I2C1->OAR1 = 0x4033;
  I2C1->CR1 |= (I2C_CR1_ACK) | (I2C_CR1_PE);
}

uint8_t i2c_read_nack(void) {
  uint32_t sreg;
  uint8_t data;
  I2C1->CR1 &= ~(I2C_CR1_ACK);
  sreg = I2C1->SR1;
  sreg = I2C1->SR2;

  I2C1->CR1 |= I2C_CR1_STOP;
  do {
    sreg = (I2C1->SR2) << 16;
    sreg |= I2C1->SR1;
  } while ((sreg & 0x00030040) != 0x00030040); /* BUSY, MSL and RXNE flags */

  data = I2C1->DR;
  return data;
}

uint8_t i2c_read_ack(void) {
  uint32_t sreg;
  uint8_t data;
  I2C1->CR1 |= I2C_CR1_ACK;

  while (1) {
    sreg = (I2C1->SR2) << 16;
    sreg |= I2C1->SR1;

    delay_us(1);

    if ((sreg & 0x00030040) == 0x00030040) /* BUSY, MSL and RXNE flags */
      break;
  }
  data = I2C1->DR;

  return data;
}

void i2c_write(uint8_t data) {
  uint32_t sreg;
  while (1) {
    sreg = (I2C1->SR2) << 16;
    sreg |= I2C1->SR1;
    delay_ms(1);
    if ((sreg & 0x00070080) == 0x00070080) /* TRA, BUSY, MSL, TXE flags */
      break;
  }
  I2C1->DR = data;
}

void i2c_stop(void) {
  uint32_t sreg;
  I2C1->CR1 |= I2C_CR1_STOP;
  while (1) {
    sreg = (I2C1->SR2) << 16;
    sreg |= I2C1->SR1;
    if ((sreg & 0x00070084) ==
        0x00070084) /* TRA, BUSY, MSL, TXE and BTF flags */
      break;
  }
}

void i2c_start(uint8_t type) { /// start condition for the I2C
  uint32_t sreg;

  while (I2C1->SR2 & I2C_SR2_BUSY)
    ;

  // Clear ADDR register by reading SR1 then SR2 register (SR1 has already been
  // read) */
  sreg = (I2C1->SR2);
  sreg = I2C1->SR1;

  I2C1->CR1 |= I2C_CR1_START;
  while (1) {
    sreg = (I2C1->SR2) << 16;
    sreg |= I2C1->SR1;

    if ((sreg & 0x00030001) == 0x00030001)
      break;
  }

  if (type == 0) {
    I2C1->DR = 0x20 | 0x01;
    while (1) {
      sreg = (I2C1->SR2) << 16;
      sreg |= I2C1->SR1;

      // printUSART2("-> SYS: M5 [%h]\n",&sreg);
      if ((sreg & 0x00030002) == 0x00030002) /* BUSY, MSL and ADDR flags */
        break;
    }
  } else {
    I2C1->DR = 0x20;
    while (1) {
      sreg = (I2C1->SR2) << 16;
      sreg |= I2C1->SR1;

      // printUSART2("-> SYS: M2 [%h]\n",&sreg);
      if ((sreg & 0x00070082) ==
          0x00070082) /* BUSY, MSL, ADDR, TXE and TRA flags */
        break;
    }
  }
}

void i2c_read_many(uint8_t reg, uint8_t *data, uint8_t nbyte) {
  i2c_start(0); // start condition
  uint16_t k;

  for (k = 0; k < nbyte; k++) {
    if (k == (nbyte - 1)) {
      data[k] = i2c_read_nack();
    } else {
      data[k] = i2c_read_ack();
    }
  }
}
