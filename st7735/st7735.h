#ifndef __ST7735_H
#define __ST7735_H

#include "fonts.h"

#define ST7735_MADCTL_MY 0x80
#define ST7735_MADCTL_MX 0x40
#define ST7735_MADCTL_MV 0x20
#define ST7735_MADCTL_ML 0x10
#define ST7735_MADCTL_RGB 0x00
#define ST7735_MADCTL_BGR 0x08
#define ST7735_MADCTL_MH 0x04

#define ST7735_CS_Pin GPIO_ODR_ODR_7
#define ST7735_CS_Port GPIOB->ODR
#define ST7735_RES_Pin GPIO_ODR_ODR_7
#define ST7735_RES_Port GPIOC->ODR
#define ST7735_DC_Pin GPIO_ODR_ODR_9
#define ST7735_DC_Port GPIOA->ODR

#define ST7735_X 0
#define ST7735_Y 0
#define ST7735_W 128
#define ST7735_H 160
#define ST7735_ROTATION (0)

#define ST7735_NOP 0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID 0x04
#define ST7735_RDDST 0x09

#define ST7735_SLPIN 0x10
#define ST7735_SLPOUT 0x11
#define ST7735_PTLON 0x12
#define ST7735_NORON 0x13

#define ST7735_INVOFF 0x20
#define ST7735_INVON 0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON 0x29
#define ST7735_CASET 0x2A
#define ST7735_RASET 0x2B
#define ST7735_RAMWR 0x2C
#define ST7735_RAMRD 0x2E

#define ST7735_PTLAR 0x30
#define ST7735_COLMOD 0x3A
#define ST7735_MADCTL 0x36

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR 0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1 0xC0
#define ST7735_PWCTR2 0xC1
#define ST7735_PWCTR3 0xC2
#define ST7735_PWCTR4 0xC3
#define ST7735_PWCTR5 0xC4
#define ST7735_VMCTR1 0xC5

#define ST7735_RDID1 0xDA
#define ST7735_RDID2 0xDB
#define ST7735_RDID3 0xDC
#define ST7735_RDID4 0xDD

#define ST7735_PWCTR6 0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define ST7735_COLOR565(r, g, b)                                               \
  (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

void ST7735_Init(void);

void ST7735_DrawPixel(uint8_t x, uint8_t y, uint16_t color);
void ST7735_DrawRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
                          uint16_t color);

void ST7735_FillRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
                          uint16_t color);
void ST7735_FillScreen(uint16_t color);

void ST7735_WriteString(uint8_t x, uint8_t y, const char *str,
                        Font_TypeDef font, uint16_t color_fg,
                        uint16_t color_bg);

#endif /* ifndef __ST7735_H */
