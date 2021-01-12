#include <stdio.h>
#include <string.h>

#include "gui.h"
#include "rtc.h"
#include "si4703.h"
#include "st7735.h"

#define MARGIN_LEFT     5
#define MARGIN_RIGHT  123
#define MARGIN_TOP     10
#define MARGIN_BOTTOM 156

#define FONT_SMALL_W  7
#define FONT_SMALL_H 10

#define FONT_LARGE_W 11
#define FONT_LARGE_H 18

#define CENTER(x) ((ST7735_W - x) / 2)

#define CENTER_SMALL(x) CENTER(x * FONT_SMALL_W)
#define CENTER_LARGE(x) CENTER(x * FONT_LARGE_W)

/******************************************************************************/

#define VOLUME_LEN 2

#define VOLUME_X (MARGIN_RIGHT - VOLUME_W)
#define VOLUME_Y (MARGIN_BOTTOM - VOLUME_H)
#define VOLUME_W (VOLUME_LEN * FONT_SMALL_W)
#define VOLUME_H FONT_SMALL_H

#define VOLUME_BAR_X MARGIN_LEFT
#define VOLUME_BAR_Y (VOLUME_Y - 1)
#define VOLUME_BAR_W (MARGIN_RIGHT - VOLUME_BAR_X - VOLUME_W - 4)
#define VOLUME_BAR_H VOLUME_H

#define VOLUME_BAR(x) (VOLUME_BAR_W * (float)x / VOLUME_MAX)

/******************************************************************************/

#define FREQUENCY_LEN 12

#define FREQUENCY_X CENTER_SMALL(FREQUENCY_LEN)
#define FREQUENCY_Y (STATION_Y + STATION_H + 5)
#define FREQUENCY_W (FREQUENCY_LEN * FONT_SMALL_W)
#define FREQUENCY_H FONT_SMALL_H

/******************************************************************************/

#define STATION_LEN (STATION_W / FONT_LARGE_W)

#define STATION_X MARGIN_LEFT
#define STATION_Y MARGIN_TOP
#define STATION_W (MARGIN_RIGHT - STATION_X)
#define STATION_H FONT_LARGE_H

/******************************************************************************/

#define CONTENT_LEN (CONTENT_W / FONT_SMALL_W)

#define CONTENT_X VOLUME_BAR_X
#define CONTENT_Y (VOLUME_BAR_Y - CONTENT_H - 5)
#define CONTENT_W (MARGIN_RIGHT - CONTENT_X)
#define CONTENT_H FONT_SMALL_H

/******************************************************************************/

#define TIME_LEN 8

#define TIME_X CENTER_LARGE(TIME_LEN)
#define TIME_Y (FREQUENCY_Y + FREQUENCY_H + 25)
#define TIME_W (TIME_LEN * FONT_LARGE_W)
#define TIME_H FONT_LARGE_H

#define DATE_LEN 12

#define DATE_X CENTER_SMALL(DATE_LEN)
#define DATE_Y (TIME_Y + TIME_H + 5)
#define DATE_W (DATE_LEN * FONT_SMALL_W)
#define DATE_H FONT_SMALL_H

/******************************************************************************/

#define COLOR_BG        ST7735_COLOR565(0x0A, 0x09, 0x08)
#define COLOR_BORDER    ST7735_COLOR565(0xA5, 0x1C, 0x30)
#define COLOR_VOLUME    ST7735_COLOR565(0x4C, 0xC5, 0xAD)
#define COLOR_FREQUENCY ST7735_COLOR565(0xD9, 0xC2, 0xAD)
#define COLOR_STATION   ST7735_COLOR565(0xDD, 0x3C, 0x4B)
#define COLOR_CONTENT   ST7735_COLOR565(0xD9, 0xC2, 0xAD)
#define COLOR_DATE_TIME ST7735_COLOR565(0xEA, 0xEA, 0xEA)

typedef enum {
  GUI_SELECTION_VOLUME = 0,
  GUI_SELECTION_SEEK,
  GUI_SELECTION_FREQUENCY
} GUI_Selection_TypeDef;

typedef enum {
  GUI_SEEK_STATE_IDLE = 0,
  GUI_SEEK_STATE_UP,
  GUI_SEEK_STATE_DOWN
} GUI_SeekState_TypeDef;

GUI_Info_TypeDef gui_info;
GUI_ShouldRender_TypeDef gui_should_render = {true, true, true, true, true};

static GUI_Selection_TypeDef gui_selection = GUI_SELECTION_VOLUME;
static GUI_SeekState_TypeDef gui_seek_state = GUI_SEEK_STATE_IDLE;

static uint16_t gui_frequency_fg = COLOR_FREQUENCY;
static uint16_t gui_frequency_bg = COLOR_BG;

static const char *gui_seek_chars = "-\\|/";
static int gui_seek_char_index = 0;

static void GUI_UpdateClock(void) {
  gui_info.date_time.hours = getHours();
  gui_info.date_time.minutes = getMinutes();
  gui_info.date_time.seconds = getSeconds();
  gui_info.date_time.year = getYear();
  gui_info.date_time.date = getDate();
  gui_info.date_time.month = getMonth();
  gui_info.date_time.week_day = weekDay();
  gui_should_render.date_time = true;
}

void GUI_Init(void) {
  ST7735_Init();
  ST7735_FillScreen(COLOR_BG);
  ST7735_DrawRectangle(0, 0, ST7735_W, ST7735_H, COLOR_BORDER);
  GUI_UpdateClock();
}

static void GUI_Rotate(char *buff) {
  if (buff == NULL)
    return;

  int n = strlen(buff), i;
  for (i = 0; i < n - 1; ++i) {
    char c = buff[i];
    buff[i] = buff[i + 1];
    buff[i + 1] = c;
  }
}

static void GUI_RenderVolume(void) {
  char volume[VOLUME_LEN + 1];
  sprintf(volume, "%2d", gui_info.volume);

  ST7735_FillRectangle(VOLUME_BAR_X, VOLUME_BAR_Y, VOLUME_BAR_W, VOLUME_BAR_H,
                       COLOR_BG);

  ST7735_DrawRectangle(VOLUME_BAR_X, VOLUME_BAR_Y, VOLUME_BAR_W, VOLUME_BAR_H,
                       COLOR_VOLUME);

  ST7735_FillRectangle(VOLUME_BAR_X, VOLUME_BAR_Y, VOLUME_BAR(gui_info.volume),
                       VOLUME_BAR_H, COLOR_VOLUME);

  ST7735_WriteString(VOLUME_X, VOLUME_Y, volume, Font_7x10, COLOR_VOLUME,
                     COLOR_BG);

  gui_should_render.volume = false;
}

static void GUI_RenderFrequency(void) {
  char frequency[FREQUENCY_LEN + 1];
  if (gui_selection != GUI_SELECTION_SEEK)
    sprintf(frequency, "FM %5.1f MHz", gui_info.frequency);
  else {
    switch (gui_seek_state) {
    case GUI_SEEK_STATE_IDLE:
      sprintf(frequency, "SEEK MODE");
      break;
    case GUI_SEEK_STATE_UP:
      sprintf(frequency, "SEEKING+ %c", gui_seek_chars[gui_seek_char_index]);
      break;
    case GUI_SEEK_STATE_DOWN:
      sprintf(frequency, "SEEKING- %c", gui_seek_chars[gui_seek_char_index]);
      break;
    }
  }

  ST7735_FillRectangle(FREQUENCY_X, FREQUENCY_Y, FREQUENCY_W, FREQUENCY_H,
                       COLOR_BG);

  ST7735_WriteString(CENTER_SMALL(strlen(frequency)), FREQUENCY_Y, frequency,
                     Font_7x10, gui_frequency_fg, gui_frequency_bg);

  gui_should_render.frequency = false;
}

static void GUI_RenderStation(void) {
  char station[STATION_LEN + 1];
  sprintf(station, "%.*s", STATION_LEN, gui_info.station ?: "N/A");

  ST7735_FillRectangle(STATION_X, STATION_Y, STATION_W, STATION_H, COLOR_BG);

  ST7735_WriteString(CENTER_LARGE(strlen(station)), STATION_Y, station,
                     Font_11x18, COLOR_STATION, COLOR_BG);

  gui_should_render.station = false;
}

static void GUI_RenderContent(void) {
  char content[CONTENT_LEN + 1];
  sprintf(content, "%.*s", CONTENT_LEN, gui_info.content ?: "Not available");

  ST7735_FillRectangle(CONTENT_X, CONTENT_Y, CONTENT_W, CONTENT_H, COLOR_BG);

  ST7735_WriteString(CONTENT_X, CONTENT_Y, content, Font_7x10, COLOR_CONTENT,
                     COLOR_BG);

  gui_should_render.content = false;
}

static void GUI_RenderDateTime(void) {
  char time[TIME_LEN + 1];
  sprintf(time, "%02ld:%02ld:%02ld", gui_info.date_time.hours,
          gui_info.date_time.minutes, gui_info.date_time.seconds);

  char date[DATE_LEN + 1];
  sprintf(date, "%s %02ld.%02ld.%02ld", gui_info.date_time.week_day,
          gui_info.date_time.date, gui_info.date_time.month,
          gui_info.date_time.year);

  ST7735_FillRectangle(TIME_X, TIME_Y, TIME_W, TIME_H + DATE_H + 5, COLOR_BG);

  ST7735_WriteString(TIME_X, TIME_Y, time, Font_11x18, COLOR_DATE_TIME,
                     COLOR_BG);

  ST7735_WriteString(DATE_X, DATE_Y, date, Font_7x10, COLOR_DATE_TIME,
                     COLOR_BG);

  gui_should_render.date_time = false;
}

void GUI_Render(void) {
  if (gui_should_render.volume)
    GUI_RenderVolume();

  if (gui_should_render.frequency)
    GUI_RenderFrequency();

  if (gui_should_render.station)
    GUI_RenderStation();

  if (gui_should_render.content)
    GUI_RenderContent();

  if (gui_should_render.date_time)
    GUI_RenderDateTime();
}

static void GUI_HandleTurnLeft(void) {
  switch (gui_selection) {
  case GUI_SELECTION_VOLUME:
    if (gui_info.volume > VOLUME_MIN) {

      gui_info.volume -= VOLUME_DELTA;
      gui_should_render.volume = true;

      set_volume(gui_info.volume);
    }
    return;
  case GUI_SELECTION_SEEK:
    if (gui_seek_state == GUI_SEEK_STATE_IDLE) {

      gui_seek_state = GUI_SEEK_STATE_DOWN;
      gui_frequency_fg = COLOR_FREQUENCY;
      gui_frequency_bg = COLOR_BG;
      gui_should_render.frequency = true;

      seek_down();
    }
    return;
  case GUI_SELECTION_FREQUENCY:
    if (gui_info.frequency > FREQUENCY_MIN) {

      gui_info.frequency = get_frequency() / 100. - FREQUENCY_DELTA;
      gui_should_render.frequency = true;

      set_frequency(gui_info.frequency * 100);
    }
    return;
  }
}

static void GUI_HandleTurnRight(void) {
  switch (gui_selection) {
  case GUI_SELECTION_VOLUME:
    if (gui_info.volume < VOLUME_MAX) {

      gui_info.volume += VOLUME_DELTA;
      gui_should_render.volume = true;

      set_volume(gui_info.volume);
    }
    return;
  case GUI_SELECTION_SEEK:
    if (gui_seek_state == GUI_SEEK_STATE_IDLE) {

      gui_seek_state = GUI_SEEK_STATE_UP;
      gui_frequency_fg = COLOR_FREQUENCY;
      gui_frequency_bg = COLOR_BG;
      gui_should_render.frequency = true;

      seek_up();
    }
    return;
  case GUI_SELECTION_FREQUENCY:
    if (gui_info.frequency < FREQUENCY_MAX) {

      gui_info.frequency = get_frequency() / 100. + FREQUENCY_DELTA;
      gui_should_render.frequency = true;

      set_frequency(gui_info.frequency * 100);
    }
    return;
  }
}

void GUI_HandleTurn(GUI_TurnDirection_TypeDef direction) {
  switch (direction) {
  case GUI_TURN_DIR_LEFT:
    GUI_HandleTurnLeft();
    return;
  case GUI_TURN_DIR_RIGHT:
    GUI_HandleTurnRight();
    return;
  }
}

void GUI_HandlePress(void) {
  switch (gui_selection) {
  case GUI_SELECTION_VOLUME:
    gui_selection = GUI_SELECTION_SEEK;
    gui_seek_state = GUI_SEEK_STATE_IDLE;
    gui_frequency_fg = COLOR_BG;
    gui_frequency_bg = COLOR_FREQUENCY;
    break;
  case GUI_SELECTION_SEEK:
    if (gui_seek_state == GUI_SEEK_STATE_IDLE) {
      gui_selection = GUI_SELECTION_VOLUME;
      gui_frequency_fg = COLOR_FREQUENCY;
      gui_frequency_bg = COLOR_BG;
    }
    break;
  case GUI_SELECTION_FREQUENCY:
    gui_selection = GUI_SELECTION_VOLUME;
    gui_frequency_fg = COLOR_FREQUENCY;
    gui_frequency_bg = COLOR_BG;
    break;
  }

  gui_should_render.frequency = true;
}

void GUI_HandleLongPress(void) {
  switch (gui_selection) {
  case GUI_SELECTION_VOLUME:
    gui_selection = GUI_SELECTION_FREQUENCY;
    gui_frequency_fg = COLOR_BG;
    gui_frequency_bg = COLOR_FREQUENCY;
    break;
  case GUI_SELECTION_SEEK:
    if (gui_seek_state == GUI_SEEK_STATE_IDLE) {
      gui_selection = GUI_SELECTION_FREQUENCY;
      gui_frequency_fg = COLOR_BG;
      gui_frequency_bg = COLOR_FREQUENCY;
    }
    break;
  case GUI_SELECTION_FREQUENCY:
    gui_selection = GUI_SELECTION_VOLUME;
    gui_frequency_fg = COLOR_FREQUENCY;
    gui_frequency_bg = COLOR_BG;
    break;
  }

  gui_should_render.frequency = true;
}

void GUI_HandleTimer(void) {
  static uint8_t cnt = 0;

  if (cnt % 10 == 0)
    GUI_UpdateClock();

  if (cnt % 5 == 0 && gui_info.station &&
      strlen(gui_info.station) > STATION_LEN) {
    GUI_Rotate(gui_info.station);
    gui_should_render.station = true;
  }

  if (cnt % 2 == 0 && gui_info.content &&
      strlen(gui_info.content) > CONTENT_LEN) {
    GUI_Rotate(gui_info.content);
    gui_should_render.content = true;
  }

  if (cnt % 6 == 0 && ((gui_selection == GUI_SELECTION_SEEK &&
                        gui_seek_state == GUI_SEEK_STATE_IDLE) ||
                       gui_selection == GUI_SELECTION_FREQUENCY)) {
    uint16_t temp = gui_frequency_fg;
    gui_frequency_fg = gui_frequency_bg;
    gui_frequency_bg = temp;
    gui_should_render.frequency = true;
  }

  if (gui_selection == GUI_SELECTION_SEEK &&
      gui_seek_state != GUI_SEEK_STATE_IDLE) {

    gui_seek_char_index = (gui_seek_char_index + 1) % 4;
    gui_should_render.frequency = true;

    _check_for_tune_complete();
  }

  cnt = (cnt + 1) % 60;
}

void GUI_HandleSeekStop(void) {
  gui_info.frequency = get_frequency() / 100.;
  gui_selection = GUI_SELECTION_VOLUME;
  gui_seek_state = GUI_SEEK_STATE_IDLE;
  gui_should_render.frequency = true;
}
