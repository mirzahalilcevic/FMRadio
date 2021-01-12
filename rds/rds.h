#ifndef __RDS_H
#define __RDS_H

#include <stdint.h>
#include <string.h>

void init_rds();
void proccess_rds_data(uint16_t block1, uint16_t block2, uint16_t block3,
                       uint16_t block4);

// RDS values
extern uint8_t rdsGroupType, rdsTP, rdsPTY;
extern uint8_t _textAB, _last_textAB;

// Program services names
extern char _PSName1[10];
extern char _PSName2[10];
extern char programServiceName[10]; // found station name or empty. Is max. 8
                                    // character long.

extern char _RDSText[64 + 2];

#endif /* ifndef __RDS_H */
