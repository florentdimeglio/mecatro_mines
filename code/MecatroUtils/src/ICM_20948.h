/// \brief A minimal driver for the ICM 20948, focused on performance.
/// \details This driver does not aim at providing full access to the sensor, but only
/// performs specific configuration for the Mecatro project, and performs fast read of
/// gyroscope and accelerometer data.
/// \author Matthieu Vigne
#ifndef ICM20948_H
    #define ICM20948_H

    typedef struct
    {
        float gyroX = 0.0; //rad/s
        float gyroY = 0.0; //rad/s
        float gyroZ = 0.0; //rad/s
        float accelX = 0.0; //m/s2
        float accelY = 0.0; //m/s2
        float accelZ = 0.0; //m/s2
        bool isValid = true;
    }IMUData;


    class ICM20948{
        public:
            /// \brief Constructor.
            ICM20948();

            /// \brief Check sensor connection and configure it
            /// \return True on success, false is sensor could not be reached
            bool init(uint8_t const& address = 0x69);

            /// \brief Read gyroscope and accelerometer data.
            /// \details This returns the last available sensor data. Note that this signal
            ///          is read from a buffer, not a FIFO.
            IMUData read();

        private:
            /// @brief Write a single register
            /// @param registerAddress Register address
            /// @param value Register value
            void writeRegister(uint8_t const& registerAddress, uint8_t const& value);

            /// @brief Read a single register
            /// @param registerAddress Register address
            /// @return 0 on failure, value on success
            uint8_t readRegister(uint8_t const& registerAddress);

            /// @brief Read several consecutive registers
            /// @param registerAddress First register address
            /// @param nRegisters Number of registers to read
            /// @param bufferOut Output buffer, must have been pre-allocated
            bool readRegisters(uint8_t const& registerAddress, uint8_t const& nRegisters, uint8_t *bufferOut);

            uint8_t address_; ///< I2C address of the chip
    };
#endif
