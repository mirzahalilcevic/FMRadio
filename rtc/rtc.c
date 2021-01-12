#include "rtc.h"
#include "misc.h"
#include "stm32f4xx.h"

volatile uint8_t rtc_str[20];
volatile uint8_t rtc_aaflag = 0;

void initRTC(uint32_t year, uint32_t day, uint32_t month, uint32_t date,
             uint32_t hour, uint32_t minut, uint32_t second, uint32_t am_pm) {
  RCC->APB1ENR |= RCC_APB1ENR_PWREN; // Enable APB1 clock to RTC
  PWR->CR |= PWR_CR_DBP; // Enable access to RTC and RTC Backup registers and
                         // backup SRAM enabled

  // select additional HSE clock prescaler (max 31)
  RCC->CFGR |= 0x00080000; // RTC clock set to HSE(8MHz)/8 = 1MHz

  RCC->BDCR |= RCC_BDCR_RTCSEL; // HSE as clock source
  RCC->BDCR |= RCC_BDCR_RTCEN;  // enable RTC clock

  RTC->WPR = 0xCA; // disable write protection for RTC (Write protection key)
  RTC->WPR = 0x53;
  RTC->ISR |=
      RTC_ISR_RSF; // enable calendar and time shadow register syncronisation
  while ((RTC->ISR & RTC_ISR_RSF) != RTC_ISR_RSF)
    ;

  RTC->ISR |= RTC_ISR_INIT; // enter initialization mode
  while ((RTC->ISR & RTC_ISR_INITF) != RTC_ISR_INITF)
    ; // wait for initialization

  RTC->PRER =
      0x00630000; // PREDIV_A = 99, must be completed in two write cycles
  RTC->PRER |= 0x0000270F; // PREDIV_S = 79999
                           // (PREDIV_A+1)*(PREDIV_S+1) = 8MHz == HSE
  RTC->TR = (((hour / 10) << 20) + ((hour % 10) << 16) + ((minut / 10) << 12) +
             ((minut % 10) << 8) + ((second / 10) << 4) + (second % 10) +
             (am_pm << 12)); // enter time values in shadow register
  RTC->DR = (((year / 10) << 20) + ((year % 10) << 16) + ((month / 10) << 12) +
             ((month % 10) << 8) + ((date / 10) << 4) + (date % 10) +
             (day << 13)); // enter date values in shadow register
  RTC->SSR = 0x00000000;

  // RTC->CR &= ~(RTC_CR_TSE);    // disable time stamp
  RTC->ISR &= ~(RTC_ISR_INIT); // exit initialization mode
  RTC->CR &= ~(RTC_CR_TSIE);   // disable timestamp interrupt

  RTC->WPR = 0xFF; // enable write protection for RTC
}

void setupAlarmARTC(void) { /// setup Alarm A with one minute alarm at each :00
                            /// seconds, with interrupt
  EXTI->IMR |= (1 << 17);   // enable interrupt on EXTI_Line17
  EXTI->EMR &= ~(1 << 17);  // disable event on EXTI_Line17
  EXTI->RTSR |= (1 << 17);
  EXTI->FTSR &= ~(1 << 17);

  RTC->WPR = 0xCA; // disable write protection for RTC (Write protection key)
  RTC->WPR = 0x53;

  RTC->CR &= ~(RTC_CR_ALRAE); // disable Alarm A
  while ((RTC->ISR & RTC_ISR_ALRAWF) != RTC_ISR_ALRAWF)
    ; // wait until we can update the Alarm A values
  RTC->ALRMAR = (RTC_ALRMAR_MSK4) | (RTC_ALRMAR_MSK3) | (RTC_ALRMAR_MSK2);
  RTC->ALRMAR &= ~((RTC_ALRMAR_ST) | (RTC_ALRMAR_SU));

  RTC->ALRMAR = 0x80808002;   // alarm at each 00 second
  RTC->ALRMASSR = 0x00000000; // sub second register value is omitted

  RTC->ISR &= ~(RTC_ISR_ALRAF);
  RTC->CR &= ~(RTC_CR_TSIE); // disable timestamp interrupt
  RTC->CR |= (RTC_CR_ALRAIE) | (RTC_CR_ALRAE);

  RTC->WPR = 0xFF;                // disable write protection for RTC
  NVIC_EnableIRQ(RTC_Alarm_IRQn); // enable alarm irq
}

void RTC_Alarm_IRQHandler(void) { /// RTC Alarm Handler
  if ((RTC->ISR & RTC_ISR_ALRAF) == RTC_ISR_ALRAF) {
    RTC->ISR &= ~(RTC_ISR_ALRAF); // clear the Alarm A flag
    rtc_aaflag = 1;
    EXTI->PR |= (1 << 17); // clear EXTI_Line17 interrupt flag
  }
}

uint8_t chkAlarmRTC(void) {
  uint8_t rval = rtc_aaflag;

  if (rtc_aaflag) {
    rtc_aaflag = 0;
  }

  return rval;
}

uint32_t getHours(void) {
  uint32_t hour;
  hour = (((RTC->TR >> 20) & 0x03) * 10) + ((RTC->TR >> 16) & 0x0f);
  return hour;
}

uint32_t getMinutes(void) {
  uint32_t minut;
  minut = (((RTC->TR >> 12) & 0x07) * 10) + ((RTC->TR >> 8) & 0x0f);
  return minut;
}

uint32_t getSeconds(void) {
  uint32_t second;
  second = (((RTC->TR >> 4) & 0x07) * 10) + (RTC->TR & 0x0f);
  return second;
}

uint32_t getYear(void) {
  uint32_t year;
  year = (((RTC->DR >> 20) & 0x07) * 10) + ((RTC->DR >> 16) & 0x0f);
  return year;
}

uint32_t getMonth(void) {
  uint32_t month;
  month = (((RTC->DR >> 12) & 0x01) * 10) + ((RTC->DR >> 8) & 0x0f);
  return month;
}

uint32_t getDate(void) {
  uint32_t date;
  date = (((RTC->DR >> 4) & 0x03) * 10) + (RTC->DR & 0x0f);
  return date;
}

uint32_t getWeekDay(void) {
  uint32_t week_day;
  week_day = (RTC->DR & RTC_DR_WDU) >> 13;
  return week_day;
}

uint32_t getSubs(void) { return RTC->SSR; }

char *weekDay(void) {
  uint32_t temp = getWeekDay();
  char *day;

  if (temp == 1) {
    day = "Mon";
  } else if (temp == 2) {
    day = "Tue";
  } else if (temp == 3) {
    day = "Wed";
  } else if (temp == 4) {
    day = "Thu";
  } else if (temp == 5) {
    day = "Fri";
  } else if (temp == 6) {
    day = "Sat";
  } else {
    day = "Sun";
  }

  return day;
}

char *Month(void) {
  uint32_t temp = getMonth();
  char *month;

  if (temp == 1) {
    month = "Jan";
  } else if (temp == 2) {
    month = "Feb";
  } else if (temp == 3) {
    month = "Mar";
  } else if (temp == 4) {
    month = "Apr";
  } else if (temp == 5) {
    month = "May";
  } else if (temp == 6) {
    month = "Jun";
  } else if (temp == 7) {
    month = "Jul";
  } else if (temp == 8) {
    month = "Aug";
  } else if (temp == 9) {
    month = "Sep";
  } else if (temp == 10) {
    month = "Oct";
  } else if (temp == 11) {
    month = "Nov";
  } else {
    month = "Dec";
  }

  return month;
}
