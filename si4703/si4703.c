#include "si4703.h"
#include "gui.h"

uint16_t _freq_low = 8750;
uint16_t _freq_high = 10800;
uint16_t _freq_steps = 10;

uint8_t _volume;
uint16_t registers[16];

void init_si4703() {

  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

  GPIOB->MODER &= ~(GPIO_MODER_MODER4_1 |
                    GPIO_MODER_MODER9_1); // Configure RESET and SCK as outputs
  GPIOB->MODER |= (GPIO_MODER_MODER4_0 |
                   GPIO_MODER_MODER9_0); // Configure RESET and SCK as outputs
  GPIOB->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR9); // set alternate function

  GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR4);
  GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR9);

  GPIOB->ODR &= ~(GPIO_ODR_ODR_9);
  GPIOB->ODR &= ~(GPIO_ODR_ODR_4);
  delay_ms(1);
  GPIOB->ODR |= GPIO_ODR_ODR_4; // Put Si4703 into reset
  delay_ms(25);
  i2c_init();
  delay_ms(25);

  _read_registers(); // Read all registers from si4703

  registers[0x07] = 0x8100; // Enable the external oscilator

  _write_registers(); // Update all registers to si4703

  delay_ms(500); // Wait for clock to take effect
  _read_registers();

  registers[POWERCFG] = 0x4001;           // Enable Si4703
  registers[SYSCONFIG1] |= (1 << RDS);    // Enable RDS function
  registers[SYSCONFIG1] |= (1 << DE);     // 50kHz Europe setup
  registers[SYSCONFIG2] |= (1 << SPACE0); // 100kHz channel spacing for Europe
  _volume = 1;
  registers[SYSCONFIG2] &= 0xFFF0;           // Clear volume bits
  registers[SYSCONFIG2] |= (_volume & 0x0F); // Set volume
  // Set seek parameters
  registers[SYSCONFIG2] |= SEEKTH_MID;
  registers[SYSCONFIG3] &= ~(SKSNR_MASK); // Clear seek mask bits
  registers[SYSCONFIG3] |= SKSNR_MID;
  registers[SYSCONFIG3] &= ~(SKCNT_MASK); // Clear seek mask bits
  registers[SYSCONFIG3] |= SKCNT_MID;

  _write_registers(); // Update all registers to si4703
  delay_ms(500); // Wait for power up maximum amount of time Datasheet page 13
}

void _read_registers() {
  uint8_t recived_data[32];
  i2c_read_many(0, recived_data, 32);

  // Register 0x0A comes in first so we have to shuffle the array around a bit
  uint8_t x;
  uint8_t y = 0;
  for (x = 0x0A;; x++) { // Read in these 32 bytes
    if (x == 0x10)
      x = 0; // Loop back to zero
    registers[x] = ((recived_data[y] << 8) +
                    recived_data[y + 1]); // Put together hi and lo byte
    y += 2;                               // Increment to next pair of byte-s
    if (x == 0x09)
      break; // We're done!
  }
}

// Save writable registers back to the chip
// The registers 02 through 06, containing the configuration
// using the sequential write access mode.
void _write_registers() {
  // Write the current 9 control registers (0x02 to 0x07) to the Si4703
  // The Si4703 assumes you are writing to 0x02 first, then increments
  // A write command automatically begins with register 0x02 so no need to send
  // a write-to address
  delay_ms(1);
  i2c_start(1);

  // First we send the 0x02 to 0x07 control registers
  // In general, we should not write to registers 0x08 and 0x09

  uint8_t regSpot;
  uint8_t high_byte;
  uint8_t low_byte;
  for (regSpot = 0x02; regSpot < 0x08; regSpot++) {
    high_byte = registers[regSpot] >> 8;
    low_byte = registers[regSpot] & 0x00FF;
    i2c_write(high_byte); // Upper 8 bits
    delay_ms(1);
    i2c_write(low_byte); // Lower 8 bits
  }

  delay_ms(1);
  // End this transmission
  i2c_stop();
} // _saveRegisters

void set_volume(uint8_t new_volume) {
  if (new_volume > 15)
    new_volume = 15;
  _read_registers();                   // Read the current register set
  registers[SYSCONFIG2] &= 0xFFF0;     // Clear volume bits
  registers[SYSCONFIG2] |= new_volume; // Set new volume
  _write_registers();                  // Update
  _volume = new_volume;
}

void set_mono(uint8_t switch_on) {
  _read_registers();
  if (switch_on) {
    registers[POWERCFG] |= (1 << SETMONO);
  } else {
    registers[POWERCFG] &= ~(1 << SETMONO);
  }
  _write_registers();
}

void set_mute(uint8_t switch_on) {
  printUSART2("citam!\n");
  _read_registers();
  printUSART2("procito!\n");

  if (switch_on) {
    registers[POWERCFG] &= ~(1 << DMUTE);
  } else {
    registers[POWERCFG] |= (1 << DMUTE);
  }
  printUSART2("pisem!\n");

  _write_registers();
  printUSART2("zapiso!\n");
}

uint16_t get_frequency(void) {
  _read_registers();
  uint16_t channel = registers[READCHAN] & 0x03FF;          // Get lower 10 bits
  uint16_t frequency = (channel * _freq_steps) + _freq_low; // Calculate freq
  return frequency;
}

void set_frequency(uint16_t freq) {
  if (freq < _freq_low) {
    freq = _freq_low;
  }
  if (freq > _freq_high) {
    freq = _freq_high;
  }

  _read_registers();

  uint16_t channel =
      (freq - _freq_low) / _freq_steps; // Calcualte channel based on new freq
  registers[CHANNEL] &= 0xFE00;         // Clear out the channel bits
  registers[CHANNEL] |= channel;        // Mask in the new channel
  registers[CHANNEL] |= (1 << TUNE);    // Set the TUNE bit to start
  _write_registers();

  _wait_for_tune_complete(); // Wait for TUNE action to complet
  init_rds();
}

void seek_up() {
  _seek(1);
}

void seek_down() {
  _seek(0);
}

void _seek(uint8_t seek_direction_up) {
  uint16_t power_cfg;
  _read_registers();
  power_cfg =
      registers[POWERCFG] &
      ~((1 << SKMODE) | (1 << SEEKUP)); // Seek down , No wrapping around
  if (seek_direction_up) {
    power_cfg |= (1 << SEEKUP); // Seek up mode
  }
  power_cfg |= (1 << SEEK); // Start seeking
  registers[POWERCFG] = power_cfg;
  _write_registers();        // Start seeking and tuneing
  _check_for_tune_complete(); // Wait for seek and tune to complete;
}

void _wait_for_tune_complete() {

  do {
    _read_registers();
  } while ((registers[STATUSRSSI] & STC) == 0); // Wait for STC HIGH

  delay_ms(2);
  _read_registers();
  registers[POWERCFG] &= ~(1 << SEEK);
  registers[CHANNEL] &= ~(1 << TUNE);
  _write_registers();

  do {
    _read_registers();
  } while ((registers[STATUSRSSI] & STC) != 0); // Wait for STC LOW
}

void _check_for_tune_complete() {

  static bool done = false;

  if (!done) {

    _read_registers();
    if ((registers[STATUSRSSI] & STC) == 0) // Wait for STC HIGH
      return;

    done = true;

    delay_ms(2);
    _read_registers();
    registers[POWERCFG] &= ~(1 << SEEK);
    registers[CHANNEL] &= ~(1 << TUNE);
    _write_registers();
  }

  _read_registers();
  if ((registers[STATUSRSSI] & STC) != 0) // Wait for STC LOW
    return;

  done = false;

  init_rds();

  GUI_HandleSeekStop();
}

void check_RDS() {
  _read_registers();
  // check for a RDS data set ready
  if (registers[STATUSRSSI] & RDSR) {
    proccess_rds_data(registers[RDSA], registers[RDSB], registers[RDSC],
                      registers[RDSD]);
  }
}
