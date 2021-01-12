#include "st7735.h"
#include "delay.h"
#include "spi1.h"

#define DELAY 0x80

// based on Adafruit ST7735 library for Arduino
static const uint8_t
    init_cmds1[] = {           // Init for 7735R, part 1 (red or green tab)
        15,                    // 15 commands in list:
        ST7735_SWRESET, DELAY, //  1: Software reset, 0 args, w/delay
        150,                   //     150 ms delay
        ST7735_SLPOUT, DELAY,  //  2: Out of sleep mode, 0 args, w/delay
        255,                   //     500 ms delay
        ST7735_FRMCTR1, 3,     //  3: Frame rate ctrl - normal mode, 3 args:
        0x01, 0x2C, 0x2D,      //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
        ST7735_FRMCTR2, 3,     //  4: Frame rate control - idle mode, 3 args:
        0x01, 0x2C, 0x2D,      //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
        ST7735_FRMCTR3, 6,     //  5: Frame rate ctrl - partial mode, 6 args:
        0x01, 0x2C, 0x2D,      //     Dot inversion mode
        0x01, 0x2C, 0x2D,      //     Line inversion mode
        ST7735_INVCTR, 1,      //  6: Display inversion ctrl, 1 arg, no delay:
        0x07,                  //     No inversion
        ST7735_PWCTR1, 3,      //  7: Power control, 3 args, no delay:
        0xA2,
        0x02,             //     -4.6V
        0x84,             //     AUTO mode
        ST7735_PWCTR2, 1, //  8: Power control, 1 arg, no delay:
        0xC5,             //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
        ST7735_PWCTR3, 2, //  9: Power control, 2 args, no delay:
        0x0A,             //     Opamp current small
        0x00,             //     Boost frequency
        ST7735_PWCTR4, 2, // 10: Power control, 2 args, no delay:
        0x8A,             //     BCLK/2, Opamp current small & Medium low
        0x2A,
        ST7735_PWCTR5, 2, // 11: Power control, 2 args, no delay:
        0x8A, 0xEE,
        ST7735_VMCTR1, 1, // 12: Power control, 1 arg, no delay:
        0x0E,
        ST7735_INVOFF, 0, // 13: Don't invert display, no args, no delay
        ST7735_MADCTL, 1, // 14: Memory access control (directions), 1 arg:
        ST7735_ROTATION,  //     row addr/col addr, bottom to top refresh
        ST7735_COLMOD, 1, // 15: set color mode, 1 arg, no delay:
        0x05},            //     16-bit color

    init_cmds2[] = {     // Init for 7735R, part 2 (1.44" display)
        2,               //  2 commands in list:
        ST7735_CASET, 4, //  1: Column addr set, 4 args, no delay:
        0x00, 0x00,      //     XSTART = 0
        0x00, 0x7F,      //     XEND = 127
        ST7735_RASET, 4, //  2: Row addr set, 4 args, no delay:
        0x00, 0x00,      //     XSTART = 0
        0x00, 0x7F},     //     XEND = 127

    init_cmds3[] = {                                                                                                         // Init for 7735R, part 3 (red or green tab)
        4,                                                                                                                   //  4 commands in list:
        ST7735_GMCTRP1, 16,                                                                                                  //  1: Magical unicorn dust, 16 args, no delay:
        0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2d, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10, ST7735_GMCTRN1, 16,  //  2: Sparkles and rainbows, 16 args, no delay:
        0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10, ST7735_NORON, DELAY, //  3: Normal display on, no args, w/delay
        10,                                                                                                                  //     10 ms delay
        ST7735_DISPON, DELAY,                                                                                                //  4: Main screen turn on, no args w/delay
        100};                                                                                                                //     100 ms delay

static void ST7735_Select(void) { CLEAR_BIT(ST7735_CS_Port, ST7735_CS_Pin); }

static void ST7735_Unselect(void) { SET_BIT(ST7735_CS_Port, ST7735_CS_Pin); }

static void ST7735_Reset(void) {
  CLEAR_BIT(ST7735_RES_Port, ST7735_RES_Pin);
  delay_ms(5);
  SET_BIT(ST7735_RES_Port, ST7735_RES_Pin);
}

static void ST7735_WriteCommand(uint8_t cmd) {
  CLEAR_BIT(ST7735_DC_Port, ST7735_DC_Pin);
  SPI1_Transmit(&cmd, sizeof(cmd));
}

static void ST7735_WriteData(uint8_t *buff, uint16_t buff_size) {
  SET_BIT(ST7735_DC_Port, ST7735_DC_Pin);
  SPI1_Transmit(buff, buff_size);
}

static void ST7735_ExecuteCommandList(const uint8_t *addr) {
  uint8_t num_commands, num_args;
  uint16_t ms;

  num_commands = *addr++;
  while (num_commands-- > 0) {
    uint8_t cmd = *addr++;
    ST7735_WriteCommand(cmd);

    num_args = *addr++;
    ms = num_args & DELAY;
    num_args &= ~DELAY;
    if (num_args) {
      ST7735_WriteData((uint8_t *)addr, num_args);
      addr += num_args;
    }

    if (ms) {
      ms = *addr++;
      if (ms == 255)
        ms = 500;
      delay_ms(ms);
    }
  }
}

static void ST7735_SetAddressWindow(uint8_t x0, uint8_t y0, uint8_t x1,
                                    uint8_t y1) {
  ST7735_WriteCommand(ST7735_CASET);
  uint8_t data[] = {0x00, x0 + ST7735_X, 0x00, x1 + ST7735_X};
  ST7735_WriteData(data, sizeof(data));

  ST7735_WriteCommand(ST7735_RASET);
  data[1] = y0 + ST7735_Y;
  data[3] = y1 + ST7735_Y;
  ST7735_WriteData(data, sizeof(data));

  ST7735_WriteCommand(ST7735_RAMWR);
}

void ST7735_Init(void) {
  // DC -> PA9
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  GPIOA->MODER |= GPIO_MODER_MODER9_0;

  // CS -> PB4
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  GPIOB->MODER |= GPIO_MODER_MODER7_0;

  // RES -> PC7
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
  GPIOC->MODER |= GPIO_MODER_MODER7_0;

  SPI1_Init(SPI_BaudRatePrescaler_2);

  ST7735_Select();
  ST7735_Reset();
  ST7735_ExecuteCommandList(init_cmds1);
  ST7735_ExecuteCommandList(init_cmds2);
  ST7735_ExecuteCommandList(init_cmds3);
  ST7735_Unselect();
}

void ST7735_DrawPixel(uint8_t x, uint8_t y, uint16_t color) {
  if ((x >= ST7735_W) || (y >= ST7735_H))
    return;

  ST7735_Select();

  ST7735_SetAddressWindow(x, y, x + 1, y + 1);
  uint8_t data[] = {color >> 8, color & 0xFF};
  ST7735_WriteData(data, sizeof(data));

  ST7735_Unselect();
}

void ST7735_DrawRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
                          uint16_t color) {
  if ((x >= ST7735_W) || (y >= ST7735_H))
    return;
  if ((x + w - 1) >= ST7735_W)
    w = ST7735_W - x;
  if ((y + h - 1) >= ST7735_H)
    h = ST7735_H - y;
  int i;
  for (i = x; i < x + w; ++i) {
    ST7735_DrawPixel(i, y, color);
    ST7735_DrawPixel(i, y + h - 1, color);
  }
  int j;
  for (j = y; j < y + h; ++j) {
    ST7735_DrawPixel(x, j, color);
    ST7735_DrawPixel(x + w - 1, j, color);
  }
}

void ST7735_FillRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
                          uint16_t color) {
  if ((x >= ST7735_W) || (y >= ST7735_H))
    return;
  if ((x + w - 1) >= ST7735_W)
    w = ST7735_W - x;
  if ((y + h - 1) >= ST7735_H)
    h = ST7735_H - y;

  ST7735_Select();
  ST7735_SetAddressWindow(x, y, x + w - 1, y + h - 1);

  uint8_t data[] = {color >> 8, color & 0xFF};
  SET_BIT(ST7735_DC_Port, ST7735_DC_Pin);
  for (y = h; y > 0; --y) {
    for (x = w; x > 0; --x) {
      SPI1_Transmit(data, sizeof(data));
    }
  }

  ST7735_Unselect();
}

void ST7735_FillScreen(uint16_t color) {
  ST7735_FillRectangle(0, 0, ST7735_W, ST7735_H, color);
}

static void ST7735_WriteChar(uint8_t x, uint8_t y, char ch, Font_TypeDef font,
                             uint16_t color_fg, uint16_t color_bg) {
  uint32_t i, b, j;

  ST7735_SetAddressWindow(x, y, x + font.width - 1, y + font.height - 1);

  for (i = 0; i < font.height; ++i) {
    b = font.data[(ch - 32) * font.height + i];
    for (j = 0; j < font.width; ++j) {
      if ((b << j) & 0x8000) {
        uint8_t data[] = {color_fg >> 8, color_fg & 0xFF};
        ST7735_WriteData(data, sizeof(data));
      } else {
        uint8_t data[] = {color_bg >> 8, color_bg & 0xFF};
        ST7735_WriteData(data, sizeof(data));
      }
    }
  }
}

void ST7735_WriteString(uint8_t x, uint8_t y, const char *str,
                        Font_TypeDef font, uint16_t color_fg,
                        uint16_t color_bg) {
  ST7735_Select();

  while (*str) {
    if (x + font.width >= ST7735_W) {
      x = 0;
      y += font.height;
      if (y + font.height >= ST7735_H)
        break;

      if (*str == ' ') {
        ++str;
        continue;
      }
    }

    ST7735_WriteChar(x, y, *str, font, color_fg, color_bg);
    x += font.width;
    ++str;
  }

  ST7735_Unselect();
}
