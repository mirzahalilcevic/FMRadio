#ifndef __RTC_H
#define __RTC_H

#include <stdint.h>

void initRTC(uint32_t year, uint32_t day, uint32_t month, uint32_t date,
             uint32_t hour, uint32_t minut, uint32_t second, uint32_t am_pm);
uint8_t *getRTC(void);
void setupAlarmARTC(void);
uint8_t chkAlarmRTC(void);
uint32_t getHours(void);
uint32_t getMinutes(void);
uint32_t getSeconds(void);
uint32_t getYear(void);
uint32_t getMonth(void);
uint32_t getDate(void);
uint32_t getWeekDay(void);
uint32_t getSubs(void);
char *weekDay(void);
char *Month(void);

#endif /* ifndef __RTC_H */
