#ifndef __GUI_H
#define __GUI_H

#include <stdbool.h>
#include <stdint.h>

#define VOLUME_MIN   0
#define VOLUME_MAX  15
#define VOLUME_DELTA 1

#define FREQUENCY_MIN  87.50
#define FREQUENCY_MAX 108.00
#define FREQUENCY_DELTA 0.10

typedef enum {
  GUI_TURN_DIR_LEFT = 0,
  GUI_TURN_DIR_RIGHT
} GUI_TurnDirection_TypeDef;

typedef struct {
  uint32_t hours;
  uint32_t minutes;
  uint32_t seconds;
  uint32_t year;
  uint32_t date;
  uint32_t month;
  char *week_day;
} GUI_DateTime_TypeDef;

typedef struct {
  int volume;
  float frequency;
  char *station;
  char *content;
  GUI_DateTime_TypeDef date_time;
} GUI_Info_TypeDef;

typedef struct {
  bool volume;
  bool frequency;
  bool station;
  bool content;
  bool date_time;
} GUI_ShouldRender_TypeDef;

void GUI_Init(void);
void GUI_Render(void);

void GUI_HandleTurn(GUI_TurnDirection_TypeDef);
void GUI_HandlePress(void);
void GUI_HandleLongPress(void);
void GUI_HandleTimer(void);
void GUI_HandleSeekStop(void);

extern GUI_Info_TypeDef gui_info;
extern GUI_ShouldRender_TypeDef gui_should_render;

#endif /* ifndef __GUI_H */
