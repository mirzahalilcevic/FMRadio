#include "rds.h"

// RDS values
uint8_t rdsGroupType;
uint8_t _textAB, _last_textAB;

// Program services names
char _PSName1[10];
char _PSName2[10];
char programServiceName[10]; // found station name or empty. Is max. 8 character
                             // long.

char _RDSText[64 + 2];

void init_rds() {
  strcpy(_PSName1, "--------");
  strcpy(_PSName2, _PSName1);
  strcpy(programServiceName, "        ");
  memset(_RDSText, 0, sizeof(_RDSText));
}

void proccess_rds_data(uint16_t block1, uint16_t block2, uint16_t block3,
                       uint16_t block4) {
  uint8_t idx; // index of rdsText
  char c1, c2;

  if (block1 == 0) {
    init_rds();
    return;
  }

  rdsGroupType = 0x0A | ((block2 & 0xF000) >> 8) | ((block2 & 0x0800) >> 11);

  switch (rdsGroupType) {
  case 0x0A:
  case 0x0B:
    // The data received is part of the Service Station Name
    idx = 2 * (block2 & 0x0003);

    // new data is 2 chars from block 4
    c1 = block4 >> 8;
    c2 = block4 & 0x00FF;

    // check that the data was received successfully twice
    // before publishing the station name

    if ((_PSName1[idx] == c1) && (_PSName1[idx + 1] == c2)) {
      // retrieved the text a second time: store to _PSName2
      _PSName2[idx] = c1;
      _PSName2[idx + 1] = c2;
      _PSName2[8] = '\0';

      if ((idx == 6) && strcmp(_PSName1, _PSName2) == 0) {
        if (strcmp(_PSName2, programServiceName) != 0) {
          // publish station name
          strcpy(programServiceName, _PSName2);
        }
      }
    }

    if ((_PSName1[idx] != c1) || (_PSName1[idx + 1] != c2)) {
      _PSName1[idx] = c1;
      _PSName1[idx + 1] = c2;
      _PSName1[8] = '\0';
    }

    break;

  case 0x2A:
    // The data received is part of the RDS Text.
    _textAB = (block2 & 0x0010);
    idx = 4 * (block2 & 0x000F);

    if (_textAB != _last_textAB) {
      // when this bit is toggled the whole buffer should be cleared.
      _last_textAB = _textAB;
      memset(_RDSText, 0, sizeof(_RDSText));
    }

    // new data is 2 chars from block 3
    _RDSText[idx] = (block3 >> 8);
    idx++;
    _RDSText[idx] = (block3 & 0x00FF);
    idx++;

    // new data is 2 chars from block 4
    _RDSText[idx] = (block4 >> 8);
    idx++;
    _RDSText[idx] = (block4 & 0x00FF);
    idx++;

    break;
  }
}
