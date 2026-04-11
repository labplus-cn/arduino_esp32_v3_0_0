/*!
 * @file mPython_MMC56x3.h
 *
 * This is a library for the MMC5603 magnentometer/compass
 *
 * Designed specifically to work with the mPython board
 *
 * These sensors use I2C to communicate, 2 pins are required to interface.
 */

#ifndef mPython_MMC56X3_MAG_H
#define mPython_MMC56X3_MAG_H

#include <Wire.h>

/*=========================================================================
    I2C ADDRESS/BITS
    -----------------------------------------------------------------------*/
#define mPython_MMC56X3_DEFAULT_ADDRESS 0x30 //!< Default address
#define mPython_MMC56X3_CHIP_ID 0x10         //!< Chip ID from WHO_AM_I register

/*=========================================================================*/

/*!
 * @brief MMC56X3 I2C register address bits
 */
typedef enum {
  mPython_MMC56X3_PRODUCT_ID = 0x39,
  mPython_MMC56X3_CTRL0_REG = 0x1B,
  mPython_MMC56X3_CTRL1_REG = 0x1C,
  mPython_MMC56X3_CTRL2_REG = 0x1D,
  mPython_MMC56X3_STATUS_REG = 0x18,
  mPython_MMC56X3_OUT_TEMP = 0x09,
  mPython_MMC56X3_OUT_X_L = 0x00,
  mPython_MMC5603_ODR_REG = 0x1A,

} mPython_mmc56x3_register_t;

// Data structure for sensor readings
typedef struct mPython_MMC5603_Data {
    float magX, magY, magZ;  // Acceleration (units depend on setting: m/s² or mg)
    float temperature;             // Temperature in °C
    uint32_t timestamp;
} mPython_MMC5603_Data_t;

/*=========================================================================*/

/**************************************************************************/
/*!
    @brief  Unified sensor driver for the magnetometer
*/
/**************************************************************************/
class mPython_MMC5603{
public:
  mPython_MMC5603();

  bool begin(uint8_t i2c_addr = mPython_MMC56X3_DEFAULT_ADDRESS, TwoWire &wire = Wire);

  bool getData(mPython_MMC5603_Data_t *data);

  void reset(void);
  void magnetSetReset(void);

  void setContinuousMode(bool mode);
  bool isContinuousMode(void);

  float readTemperature(void);

  uint16_t getDataRate();
  void setDataRate(uint16_t rate);

private:
  TwoWire *_wire;
  uint8_t _address;

  uint16_t _odr_cache = 0;
  uint8_t _ctrl2_cache = 0;

  int32_t x; ///< x-axis raw data
  int32_t y; ///< y-axis raw data
  int32_t z; ///< z-axis raw data

  bool writeRegister(uint8_t reg, uint8_t value);
  bool readRegister(uint8_t reg, uint8_t *buffer, uint8_t length);
  bool readRegister(uint8_t reg, uint8_t &value);
};

#endif