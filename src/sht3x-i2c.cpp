/* sht3x-i2c library by Mariano Goluboff
 */

#include "sht3x-i2c.h"

uint8_t crc8(uint8_t *buf, size_t len)
{
    uint8_t crc(0xFF);

    for (int j = len; j; --j)
    {
        crc ^= *buf++;

        for (int i = 8; i; --i)
        {
            crc = (crc & 0x80)
                      ? (crc << 1) ^ 0x31
                      : (crc << 1);
        }
    }
    return crc;
}

/**
 * Constructor.
 */
Sht3xi2c::Sht3xi2c(TwoWire& interface)
{
  // be sure not to call anything that requires hardware be initialized here, put those in begin()
  _wire = &interface;
  _state = SHT31_STATE_IDLE;
}

/**
 * Example method.
 */
void Sht3xi2c::begin(uint32_t speed)
{
    if (!_wire->isEnabled())
    {
        _wire->setSpeed(speed);
        _wire->begin();
    }
}

int Sht3xi2c::singleShotRead(float* temp, float* humid, uint8_t accuracy, uint8_t i2c_addr, bool clock_stretching)
{
    int ret_value = 0;
    uint8_t time_delay;
    if (_state != SHT31_STATE_IDLE)
    {
        if(!_break_command(i2c_addr)) return SHT31_I2C_ERROR;
    }
    if (!_wire->lock()) return SHT31_I2C_LOCK_ERROR;

    if (!clock_stretching)
    {
        _wire->beginTransmission(i2c_addr);
        _wire->write(0x24);
        switch (accuracy)
        {
        case SHT31_ACCURACY_HIGH:
            _wire->write(0x00);
            time_delay = 25;
            break;
        case SHT31_ACCURACY_LOW:
            _wire->write(0x16);
            time_delay = 10;
            break;
        default:
            _wire->write(0x0B);
            time_delay = 15;
            break;
        }
        if (_wire->endTransmission() != 0)
        {
            ret_value = -1;
            goto end_command;
        }
        delay(time_delay);       // Add a delay because when there's a NACK, I2C bus is reset
        _wire->requestFrom(i2c_addr, 6);
        if (_wire->available() != 6)
        {
            ret_value = SHT31_I2C_ERROR;
            goto end_command;
        }
        *(temp+1) = _wire->read();
        *temp = _wire->read();
        if (_wire->read() != crc8((uint8_t*)temp, 2))
        {
            ret_value = SHT31_CRC_ERROR;
            goto end_command;
        }
        *(humid+1) = _wire->read();
        *humid = _wire->read();
        if (_wire->read() != crc8((uint8_t*)humid, 2))
        {
            ret_value = SHT31_CRC_ERROR;
            goto end_command;
        }
    } 
    else
    { // With clock stretching
        _wire->beginTransmission(i2c_addr);
        _wire->write(0x2C);
        switch (accuracy)
        {
        case SHT31_ACCURACY_HIGH:
            _wire->write(0x06);
            break;
        case SHT31_ACCURACY_LOW:
            _wire->write(0x10);
            break;
        default:
            _wire->write(0x0D);
            break;
        }
        if (_wire->endTransmission() != 0)
        {
            ret_value = SHT31_I2C_ERROR;
            goto end_command;
        }
        // TODO: add read/but nrf52840 has issues with clock stretching?
    }
end_command:
    _wire->unlock();
    return ret_value;
}

/**
 * Example method.
 */
void Sht3xi2c::process()
{
    // do something useful
    Serial.println("called process");
}

bool Sht3xi2c::_break_command(uint8_t i2c_addr)
{
    if (!_wire->lock()) return false;
    _wire->beginTransmission(i2c_addr);
    _wire->write(0x30);
    _wire->write(0x93);
    if (_wire->endTransmission() == 0) _state = SHT31_STATE_IDLE;
    _wire->unlock();
    return (_state == SHT31_STATE_IDLE);
}
