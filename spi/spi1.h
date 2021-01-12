#ifndef __SPI1_H
#define __SPI1_H

#include <stdint.h>

#define SPI_BaudRatePrescaler_2   ((uint16_t)0x0000)
#define SPI_BaudRatePrescaler_4   ((uint16_t)0x0008)
#define SPI_BaudRatePrescaler_8   ((uint16_t)0x0010)
#define SPI_BaudRatePrescaler_16  ((uint16_t)0x0018)
#define SPI_BaudRatePrescaler_32  ((uint16_t)0x0020)
#define SPI_BaudRatePrescaler_64  ((uint16_t)0x0028)
#define SPI_BaudRatePrescaler_128 ((uint16_t)0x0030)
#define SPI_BaudRatePrescaler_256 ((uint16_t)0x0038)
#define SPI_I2S_FLAG_RXNE         ((uint16_t)0x0001)
#define SPI_I2S_FLAG_TXE          ((uint16_t)0x0002)
#define I2S_FLAG_CHSIDE           ((uint16_t)0x0004)
#define I2S_FLAG_UDR              ((uint16_t)0x0008)
#define SPI_FLAG_CRCERR           ((uint16_t)0x0010)
#define SPI_FLAG_MODF             ((uint16_t)0x0020)
#define SPI_I2S_FLAG_OVR          ((uint16_t)0x0040)
#define SPI_I2S_FLAG_BSY          ((uint16_t)0x0080)
#define SPI_I2S_FLAG_TIFRFE       ((uint16_t)0x0100)

void SPI1_Init(uint16_t prescaler);
void SPI1_Transmit(uint8_t *buff, uint32_t buff_size);

#endif /* ifndef __SPI1_H */
