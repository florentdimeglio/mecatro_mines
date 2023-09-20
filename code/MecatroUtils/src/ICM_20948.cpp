#include <Wire.h>
#include "Arduino.h"
#include "ICM_20948.h"

// Register map of the IMU, see datasheet page 32 onward

#define REG_WHO_AM_I 0x00
#define REG_BANK_SEL 0x7F

#define REG_USER_CTRL 0x03
#define REG_LP_CONFIG 0x05
#define REG_PWR_MGMT_1 0x06
#define REG_PWR_MGMT_2 0x07

#define REG_GYRO_SMPLRT_DIV 0x00
#define REG_GYRO_CONFIG_1 0x01
#define REG_GYRO_CONFIG_2 0x02

#define ACCEL_SMPLRT_DIV_1  0x12
#define ACCEL_SMPLRT_DIV_2  0x13
#define REG_ACCEL_CONFIG  0x14
#define REG_ACCEL_CONFIG2 0x15


#define REG_ACCEL_XOUT_H 0x2D

#define GYRO_RAW_TO_RADS 0.00013323124061025415
#define ACCEL_RAW_TO_MS2 0.0005987548828125

ICM20948::ICM20948():
    address_(0)
{
}

bool ICM20948::init(uint8_t const& address)
{
    address_ = address;

    // Set device to bank zero for WHO AM I.
    writeRegister(REG_BANK_SEL, 0);

    // Disable DMP and FIFO
    writeRegister(REG_USER_CTRL, 0);
    // Enable sensors, in low-noise
    writeRegister(REG_LP_CONFIG, 0b00000000);
    writeRegister(REG_PWR_MGMT_1, 0b00001001);
    writeRegister(REG_PWR_MGMT_2, 0);


    // Check that correct device is present
    if (readRegister(REG_WHO_AM_I) != 0xEA)
        return false;

    // Configure sensor
    writeRegister(REG_BANK_SEL, 0b00100000);

    // Gyro measurements range 250dps, digital low-pass at 119Hz
    writeRegister(REG_GYRO_CONFIG_1, 0b00010001);

    // Gyro output data rate of 281 Hz
    writeRegister(REG_GYRO_SMPLRT_DIV, 3);

    // Disable averaging
    writeRegister(REG_GYRO_CONFIG_2, 0);

    // Accelerometer: 2g scale, low-pass at 111Hz
    writeRegister(REG_ACCEL_CONFIG, 0b00010001);

    // Disable averaging
    writeRegister(REG_ACCEL_CONFIG2, 0);

    // Accelerometer ODR of 281Hz
    writeRegister(ACCEL_SMPLRT_DIV_1, 0);
    writeRegister(ACCEL_SMPLRT_DIV_2, 3);

    // Set device to bank 0
    writeRegister(REG_BANK_SEL, 0);

    return true;
}


IMUData ICM20948::read()
{
    IMUData data;

    uint8_t rawData[12];
    bool success = readRegisters(REG_ACCEL_XOUT_H, 12, rawData);
    // Retry a second time
    if (!success)
      success = readRegisters(REG_ACCEL_XOUT_H, 12, rawData);
    if (!success)
    {
      data.isValid = false;
      return data;
    }

    // Convert to SI unit
    data.accelX = (int16_t) ((rawData[0] << 8) + rawData[1]) * ACCEL_RAW_TO_MS2;
    data.accelY = (int16_t) ((rawData[2] << 8) + rawData[3]) * ACCEL_RAW_TO_MS2;
    data.accelZ = (int16_t) ((rawData[4] << 8) + rawData[5]) * ACCEL_RAW_TO_MS2;

    data.gyroX = (int16_t) ((rawData[6] << 8) + rawData[7]) * GYRO_RAW_TO_RADS;
    data.gyroY = (int16_t) ((rawData[8] << 8) + rawData[9]) * GYRO_RAW_TO_RADS;
    data.gyroZ = (int16_t) ((rawData[10] << 8) + rawData[11]) * GYRO_RAW_TO_RADS;

    return data;
}


void ICM20948::writeRegister(uint8_t const& registerAddress, uint8_t const& value)
{
    Wire.beginTransmission(address_);
    Wire.write(registerAddress);
    Wire.write(value);
    Wire.endTransmission();
}


uint8_t ICM20948::readRegister(uint8_t const& registerAddress)
{
    Wire.beginTransmission(address_);
    Wire.write(registerAddress);
    Wire.endTransmission(false); // Send repeated start

    uint32_t num_received = Wire.requestFrom(address_, (uint8_t) 1);

    if (num_received == 1)
        return Wire.read();
  return 0;
}


bool ICM20948::readRegisters(uint8_t const& registerAddress, uint8_t const& nRegisters, uint8_t *bufferOut)
{
    Wire.beginTransmission(address_);
    Wire.write(registerAddress);
    Wire.endTransmission(false); // Send repeated start

    uint32_t num_received = Wire.requestFrom(address_, nRegisters);

    if (num_received == nRegisters)
    {
        for (int i = 0; i < nRegisters; i++)
            bufferOut[i] = Wire.read();
        return true;
    }
    return false;
}
