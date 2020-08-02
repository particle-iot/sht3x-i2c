#pragma once

/* sht3x-i2c library by Mariano Goluboff
 */

// This will load the definition for common Particle variable types
#include "Particle.h"

#define SHT31_STATE_IDLE            0x00
#define SHT31_STATE_CONTINUOUS      0x01

#define SHT31_ACCURACY_HIGH         0x00
#define SHT31_ACCURACY_MEDIUM       0x01
#define SHT31_ACCURACY_LOW          0x02

#define SHT31_I2C_ERROR             -0x01
#define SHT31_I2C_LOCK_ERROR        -0x02
#define SHT31_CRC_ERROR             -0x03

// This is your main class that users will import into their application
class Sht3xi2c
{
public:
  /**
   * Constructor
   */
  Sht3xi2c(TwoWire& interface);

  void begin(uint32_t speed = CLOCK_SPEED_400KHZ);
  int singleShotRead(float* temp, float* humid, uint8_t accuracy = SHT31_ACCURACY_MEDIUM, uint8_t i2c_addr = 0x44, bool clock_stretching = false);

  void process();

private:
  TwoWire* _wire;
  uint8_t _state;
  bool _break_command(uint8_t i2c_addr);
};

// uint8_t i2c_addr = 0x44
