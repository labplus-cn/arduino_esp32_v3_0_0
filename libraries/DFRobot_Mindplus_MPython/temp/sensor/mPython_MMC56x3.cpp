/*!
 * @file mPython_MMC56x3.cpp
 *
 * @mainpage mPython MMC56x3 Breakouts
 *
 * @section intro_sec Introduction
 *
 * This is a library for the MMC5603/MMC5613 Magnentometer/compass
 *
 * Designed specifically to work with the mPython board
 *
 * These sensors use I2C to communicate, 2 pins are required to interface.
 */

#include "Arduino.h"
#include <Wire.h>

#include <limits.h>

#include "mPython_MMC56x3.h"

/***************************************************************************
 MAGNETOMETER
 ***************************************************************************/

/***************************************************************************
 CONSTRUCTOR
 ***************************************************************************/

/**************************************************************************/
/*!
    @brief  Instantiates a new MMC5603 class
*/
/**************************************************************************/
mPython_MMC5603::mPython_MMC5603() {
  // Clear the raw mag data
  x = 0;
  y = 0;
  z = 0;
}

/***************************************************************************
 PUBLIC FUNCTIONS
 ***************************************************************************/
/*!
 *    @brief  Sets up the hardware and initializes I2C
 *    @param  i2c_address
 *            The I2C address to be used.
 *    @param  wire
 *            The Wire object to be used for I2C connections.
 *    @return True if initialization was successful, otherwise false.
 */
bool mPython_MMC5603::begin(uint8_t i2c_address, TwoWire &wire) {
  _wire = &wire;
  _address = i2c_address;
  // Check connection
  // make sure we're talking to the right chip
  uint8_t id;
  readRegister(mPython_MMC56X3_PRODUCT_ID, id);
  if ((id != mPython_MMC56X3_CHIP_ID) && (id != 0x0)) {
    // No MMC56X3 detected ... return false
    return false;
  }

  reset();
  
  return true;
}
/*!
 *    @brief  Resets the sensor to an initial state
 */
void mPython_MMC5603::reset(void) {
  uint8_t temp;
  readRegister(mPython_MMC56X3_CTRL1_REG, temp);
  temp |= 0x80; // set reset bit
  writeRegister(mPython_MMC56X3_CTRL1_REG, temp); //sw reset
  delay(20);
  _odr_cache = 0;
  _ctrl2_cache = 0;
  magnetSetReset();
  setContinuousMode(false);
}

/*!
 *    @brief  Pulse large currents through the sense coils to clear any offset
 */
void mPython_MMC5603::magnetSetReset(void) {
  uint8_t temp;
  readRegister(mPython_MMC56X3_CTRL0_REG, temp);
  temp |= 0x08; 
  writeRegister(mPython_MMC56X3_CTRL0_REG, temp); // turn on set bit
  delay(1);
  readRegister(mPython_MMC56X3_CTRL0_REG, temp);
  temp |= 0x10;
  writeRegister(mPython_MMC56X3_CTRL0_REG, temp); // turn on reset bit
  delay(1);
}

/**************************************************************************/
/*!
    @brief  Sets whether we are in continuous read mode (t) or one-shot (f)
    @param mode True for continuous, False for one-shot
*/
/**************************************************************************/
void mPython_MMC5603::setContinuousMode(bool mode) {
  if (mode) {
    uint8_t temp;
    readRegister(mPython_MMC56X3_CTRL0_REG, temp);
    temp |= 0x80;
    writeRegister(mPython_MMC56X3_CTRL0_REG, temp); // turn on cmm_freq_en bit
    _ctrl2_cache |= 0x10;    // turn on cmm_en bit
  } else {
    _ctrl2_cache &= ~0x10; // turn off cmm_en bit
  }
  writeRegister(mPython_MMC56X3_CTRL2_REG, _ctrl2_cache);
}

/**************************************************************************/
/*!
    @brief Determine whether we are in continuous read mode (t) or one-shot (f)
    @returns True for continuous, False for one-shot
*/
/**************************************************************************/
bool mPython_MMC5603::isContinuousMode(void) { return _ctrl2_cache & 0x10; }

/**************************************************************************/
/*!
    @brief Read the temperature from onboard sensor - must not be in continuous
   mode for this to function it seems
    @returns Floating point temp in C, or NaN if sensor is in continuous mode
*/
/**************************************************************************/
float mPython_MMC5603::readTemperature(void) {
  if (isContinuousMode())
    return NAN;

  writeRegister(mPython_MMC56X3_CTRL0_REG, 0x02); // TM_T trigger

  uint8_t value;
  do {
      readRegister(mPython_MMC56X3_STATUS_REG, value);
      delay(5);
    }while((value & 0x80) != 0x80);

  readRegister(mPython_MMC56X3_OUT_TEMP, value);

  float temp = (float)value * 0.8; //  0.8*C / LSB
  temp -= 75;  //  0 value is -75

  return temp;
}

/**************************************************************************/
/*!
    @brief  Gets the most recent sensor event
    @param event The `sensors_event_t` to fill with event data
    @returns true, always
*/
/**************************************************************************/
bool mPython_MMC5603::getData(mPython_MMC5603_Data_t *data) {
  /* Read new data */
  if (!isContinuousMode()) {
    writeRegister(mPython_MMC56X3_CTRL0_REG, 0x01); // TM_M trigger

    uint8_t value;
    do {
      readRegister(mPython_MMC56X3_STATUS_REG, value);
      delay(5);
    }while((value & 0x40) != 0x40);
  }
  uint8_t buffer[9];
  readRegister(mPython_MMC56X3_OUT_X_L, buffer, 8);

  x = (uint32_t)buffer[0] << 12 | (uint32_t)buffer[1] << 4 |
      (uint32_t)buffer[6] >> 4;
  y = (uint32_t)buffer[2] << 12 | (uint32_t)buffer[3] << 4 |
      (uint32_t)buffer[7] >> 4;
  z = (uint32_t)buffer[4] << 12 | (uint32_t)buffer[5] << 4 |
      (uint32_t)buffer[8] >> 4;
  // fix center offsets
  x -= (uint32_t)1 << 19;
  y -= (uint32_t)1 << 19;
  z -= (uint32_t)1 << 19;

  data->timestamp = millis();
  data->magX = (float)x * 0.00625; // scale to uT by LSB in datasheet
  data->magY = (float)y * 0.00625;
  data->magZ = (float)z * 0.00625;

  return true;
}

/**************************************************************************/
/*!
    @brief  Sets the magnetometer's update rate, from 0-255 or 1000
    @param rate The new frequency data rate to set, from 0-255 or 1000
*/
/**************************************************************************/
void mPython_MMC5603::setDataRate(uint16_t rate) {
  // only 0~255 and 1000 are valid, so just move any high rates to 1000
  if (rate > 255)
    rate = 1000;
  _odr_cache = rate;

  if (rate == 1000) {
    writeRegister(mPython_MMC5603_ODR_REG, 255);
    _ctrl2_cache |= 0x80; // turn on hpower bit
    writeRegister(mPython_MMC56X3_CTRL2_REG, _ctrl2_cache);
  } else {
    writeRegister(mPython_MMC5603_ODR_REG,rate);
    _ctrl2_cache &= ~0x80; // turn off hpower bit
    writeRegister(mPython_MMC56X3_CTRL2_REG, _ctrl2_cache);
  }
}

/**************************************************************************/
/*!
    @brief  Gets the magnetometer's update rate (cached from data rate set)
    @returns The current data rate from 0-255 or 1000
*/
/**************************************************************************/
uint16_t mPython_MMC5603::getDataRate(void) { return _ctrl2_cache; }

// Private methods
bool mPython_MMC5603::writeRegister(uint8_t reg, uint8_t value) {
    if (!_wire) return false;
    
    _wire->beginTransmission(_address);
    _wire->write(reg);
    _wire->write(value);
    uint8_t error = _wire->endTransmission();
    
    return (error == 0);
}

bool mPython_MMC5603::readRegister(uint8_t reg, uint8_t *buffer, uint8_t length) {
    if (!_wire) return false;
    
    _wire->beginTransmission(_address);
    _wire->write(reg);
    uint8_t error = _wire->endTransmission(false);
    
    if (error != 0) return false;
    
    uint8_t received = _wire->requestFrom(_address, length);
    if (received != length) return false;
    
    for (uint8_t i = 0; i < length; i++) {
        buffer[i] = _wire->read();
    }
    
    return true;
}

bool mPython_MMC5603::readRegister(uint8_t reg, uint8_t &value) {
    return readRegister(reg, &value, 1);
}