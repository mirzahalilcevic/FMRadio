#ifndef __FONTS_H
#define __FONTS_H

#include <stdint.h>

typedef struct {
  const uint8_t width;
  const uint8_t height;
  const uint16_t *data;
} Font_TypeDef;

extern const Font_TypeDef Font_7x10;
extern const Font_TypeDef Font_11x18;

#endif /* __FONTS_H */
