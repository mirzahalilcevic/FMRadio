#include <ctype.h>

#include "gui.h"
#include "rotary.h"
#include "rtc.h"
#include "si4703.h"

static char station[12];
static char content[69], gui_content[69];

char *ltrim(char *s) {
  while (isspace(*s))
    s++;
  return s;
}

char *rtrim(char *s) {
  char *back = s + strlen(s);
  while (isspace(*--back))
    ;
  *(back + 1) = '\0';
  return s;
}

char *trim(char *s) { return rtrim(ltrim(s)); }

void init_rtc_usart(void) {

  bool failed = false;

  putcharUSART2(1);

  int8_t day = getcharUSART2();
  if (day == -1)
    failed = true;
  
  int8_t month = getcharUSART2();
  if (month == -1)
    failed = true;

  int8_t year = getcharUSART2();
  if (year == -1)
    failed = true;

  int8_t week_day = getcharUSART2();
  if (week_day == -1)
    failed = true;

  int8_t hours = getcharUSART2();
  if (hours == -1)
    failed = true;

  int8_t minutes = getcharUSART2();
  if (minutes == -1)
    failed = true;

  int8_t seconds = getcharUSART2();
  if (seconds == -1)
    failed = true;

  if (!failed)
    initRTC(year, week_day + 1, month, day, hours, minutes, seconds, 0);
  else
    initRTC(20, 1, 2, 10, 8, 0, 0, 0);
}

void add_whitespace(char *str, int n) {

  str[n - 1] = ' ';
  str[n    ] = ' ';
  str[n + 1] = ' ';

  str[n + 2] = '\0';
}

int main(void) {

  initUSART2(USART2_BAUDRATE_115200);

  init_si4703();

  gui_info.volume = 10;
  set_volume(10);

  seek_up();
  _wait_for_tune_complete();

  gui_info.frequency = get_frequency() / 100.;

  gui_info.station = NULL;
  gui_info.content = NULL;

  initSYSTIMER();
  init_rtc_usart();
  init_rotary_encoder();

  GUI_Init();

  uint32_t animation_time = getSYSTIMER(), rds_time = getSYSTIMER(),
           display_time = getSYSTIMER(), long_press_time;

  bool pending_press = false;

  while (1) {
    
    GUI_Render();
    serviceIRQA();

    if (button_state == 1) {
      if (pending_press) {
        pending_press = false;
        GUI_HandleLongPress();
      } else {
        pending_press = true;
        long_press_time = getSYSTIMER();
      }
      button_state = 0;
    }

    int delta = RE_Count - previous_RE_Count, i;
    if (delta < 0) {
      for (i = 0; i < -delta; ++i)
        GUI_HandleTurn(GUI_TURN_DIR_LEFT);
      previous_RE_Count = RE_Count;
    } else if (delta > 0) {
      for (i = 0; i < delta; ++i)
        GUI_HandleTurn(GUI_TURN_DIR_RIGHT);
      previous_RE_Count = RE_Count;
    }
    
    if (chk4TimeoutSYSTIMER(rds_time, 1000) != SYSTIMER_KEEP_ALIVE) {
      check_RDS();
      rds_time = getSYSTIMER();
    }

    if (chk4TimeoutSYSTIMER(animation_time, 100000) != SYSTIMER_KEEP_ALIVE) {
      GUI_HandleTimer();
      animation_time = getSYSTIMER();
    }

    if (chk4TimeoutSYSTIMER(display_time, 1000000) != SYSTIMER_KEEP_ALIVE) {
      char *station_trimmed = trim(programServiceName);
      if (strcmp(station_trimmed, "") == 0) {
        if (gui_info.station != NULL) {
          gui_info.station = NULL;
          gui_should_render.station = true;
        }
      } else if (strcmp(station_trimmed, station) != 0) {
        gui_info.station = strdup(strcpy(station, station_trimmed));
        gui_should_render.station = true;
      }
      char *content_trimmed = trim(_RDSText);
      if (strcmp(content_trimmed, "") == 0) {
        if (gui_info.content != NULL) {
          gui_info.content = NULL;
          gui_should_render.content = true;
        }
      } else if (strcmp(content_trimmed, content) != 0) {
        gui_info.content =
            strcpy(gui_content, strcpy(content, content_trimmed));
        add_whitespace(gui_info.content, 64);
        gui_should_render.content = true;
      }
      display_time = getSYSTIMER();
    }

    if (pending_press &&
        chk4TimeoutSYSTIMER(long_press_time, 500000) != SYSTIMER_KEEP_ALIVE) {
      pending_press = false;
      GUI_HandlePress();
    }
  }

  return 0;
}
