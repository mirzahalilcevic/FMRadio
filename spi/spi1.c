#include "spi1.h"
#include "stm32f4xx.h"

void SPI1_Init(uint16_t prescaler) {
  // PA5 -> SCK, PA7 -> MOSI
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  GPIOA->MODER |= GPIO_MODER_MODER5_1 | GPIO_MODER_MODER7_1;
  GPIOA->OTYPER &= ~(GPIO_OTYPER_OT_5 | GPIO_OTYPER_OT_7);
  GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR5_1 | GPIO_OSPEEDER_OSPEEDR7_1;
  GPIOA->AFR[0] |= 0x50500000;

  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
  SPI1->CR1 |= SPI_CR1_MSTR;
  SPI1->CR1 |= SPI_CR1_SSI | SPI_CR1_SSM;
  SPI1->CR1 |= prescaler;
  SPI1->CR1 |= SPI_CR1_SPE;
}

void SPI1_Transmit(uint8_t *buff, uint32_t buff_size) {
  int i;
  for (i = 0; i < buff_size; ++i) {
    SPI1->DR = buff[i];
    while (!(SPI1->SR & SPI_I2S_FLAG_TXE))
      ;
    while (!(SPI1->SR & SPI_I2S_FLAG_RXNE))
      ;
    while (SPI1->SR & SPI_I2S_FLAG_BSY)
      ;
  }
}
