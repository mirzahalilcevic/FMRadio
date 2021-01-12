#ifndef _SI4703_H
#define _SI4703_H

#include "stm32f4xx.h"

#include "delay.h"
#include "i2c.h"
#include "rds.h"
#include "usart.h"
#include <stdint.h>

void init_si4703(void);
void set_volume(uint8_t new_volume);
void set_mono(uint8_t switch_on);
void set_mute(uint8_t switch_on);
uint16_t get_frequency(void);
void set_frequency(uint16_t freq);
void seek_up(void);
void seek_down(void);
void check_RDS(void);

// Private functions
void _read_registers(void);
void _write_registers(void);
uint16_t _read16(uint8_t hiByte, uint8_t loByte);
void _write16(uint16_t val);
void _wait_for_tune_complete(void);
void _check_for_tune_complete(void);
void _seek(uint8_t seek_direction_up);

extern uint16_t _freq_low;
extern uint16_t _freq_high;
extern uint16_t _freq_steps;

extern uint8_t _volume;
extern uint16_t registers[16];

// I2C adress
#define SI4703_ADR 0x10

// Define the register names
#define DEVICEID 0x00
#define CHIPID 0x01
#define POWERCFG 0x02
#define CHANNEL 0x03
#define SYSCONFIG1 0x04
#define SYSCONFIG2 0x05
#define SYSCONFIG3 0x06
#define STATUSRSSI 0x0A
#define READCHAN 0x0B
#define RDSA 0x0C
#define RDSB 0x0D
#define RDSC 0x0E
#define RDSD 0x0F

// Register 0x02 - POWERCFG
#define DSMUTE 15
#define DMUTE 14
#define SETMONO 13
#define SKMODE 10
#define SEEKUP 9
#define SEEK 8

// Register 0x03 - CHANNEL
#define TUNE 15

// Register 0x04 - SYSCONFIG1
#define RDS 12
#define DE 11

#define DE 11

// Register 0x05 - SYSCONFIG2
#define SEEKTH_MASK 0xFF00
#define SEEKTH_MIN 0x0000
#define SEEKTH_MID 0x1000
#define SEEKTH_MAX 0x7F00

#define SPACE1 5
#define SPACE0 4

// Register 0x06 - SYSCONFIG3
#define SKSNR_MASK 0x00F0
#define SKSNR_OFF 0x0000
#define SKSNR_MIN 0x0010
#define SKSNR_MID 0x0030
#define SKSNR_MAX 0x0070

#define SKCNT_MASK 0x000F
#define SKCNT_OFF 0x0000
#define SKCNT_MIN 0x000F
#define SKCNT_MID 0x0003
#define SKCNT_MAX 0x0001

// Register 0x0A - STATUSRSSI
#define RDSR 0x8000 ///< RDS ready
#define STC 0x4000  ///< Seek Tune Complete
#define SFBL 0x2000 ///< Seek Fail Band Limit
#define AFCRL 0x1000
#define RDSS 0x0800 ///< RDS syncronized
#define SI 0x0100   ///< Stereo Indicator
#define RSSI 0x00FF

#endif /* ifndef _SI4703_H */
