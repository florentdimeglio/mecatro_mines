/*
  Utility functions for the Mecatro project, Mines.
  Author: Matthieu Vigne ; 2023
*/

namespace mecatro{

    /// @brief The control loop function, to be implemented by the user.
    /// @details This function is called periodically, every N milliseconds (defined when calling configureMotorControl).
    ///          Note that this function should take less than N milliseconds to complete for the next control iteration to occur.
    void controlLoop();

    /// @brief Configure the Arduino for motor control: hardware PWM and timer interrupt.
    /// @details This function needs to be called during the init phase.
    ///          It performs two required action for motor control:
    ///             - configure the Arduino hardware TIMER1 to generate a high-frequency signal
    ///             to drive the motors.
    ///             - configure the Arduino hardware TIMER2 to generate a time-based interruption every
    ///              5ms.
    /// @note     This function assumes that the motor driver is mapped to the following pins:
    ///              - right motor: PWM pin 9 (aka OCR1A) direction pin 8
    ///              - left motor: PWM pin 10 (aka OCR1B) direction pin 12
    /// @param[in] controlPeriodMs Control loop period, in ms. Do not use a value above 16ms as this will lead to unexpected results.
    void configureArduino(unsigned char const controlPeriodMs = 5);

    /// @brief Wait for gains comming from master over WiFi.
    ///
    /// @details "Gains" is an array of nGains float32. This function blocks until a valid
    ///          message has been recieved.
    ///          A gain message is of the form [0xFF <float0> <float1> ... <floatN> <checksum>],
    ///          where checksum = 255 - sum(float bytes).
    ///
    /// @param[in] nGains Number of gains needed.
    /// @param[out] gainsArray preallocated array to recieve the gains.
    void recieveGains(unsigned char const nGains, float *gainsArray);

    /// @brief Set the duty cycle value of both motor drivers.
    /// @details The motors of the robot are controlled using H-bridges. The control input is
    ///          thus the duty cycle i.e. the percentage of time that the transistors are on.
    ///          The duty cycle variable also encodes direction: thus, -1 is full reverse, 1 is full
    ///          forward, 0 is power off.
    /// @param[in] rightMotorDC Duty cycle for right motor, between -1 (full reverse) and 1 (full foward).
    /// @param[in] leftMotorDC Duty cycle for left motor, between -1 (full reverse) and 1 (full foward).
    void setMotorDutyCycle(float const& leftMotorDC, float const& rightMotorDC);

    /// @brief Initialize telemetry
    /// @details Telemetry consists of n variables, n being constant. This function both defines n,
    ///          sets the names of the variable, and sends them to the telemetry client, connected through WiFi.
    ///          Note that this function blocks until a client is indeed connected through WiFi, and has sent
    //           the letter 's' to start the process.
    /// @note Telemetry is internally limitted to 100 variables.
    /// @note It takes about 200us at 230400bps to send a single variable.
    ///
    /// @param wifiSSID SSID, aka name, of the WiFi network created by the Arduino
    /// @param wifiPassword Password of the WiFi network
    /// @param numberOfVariables Number of variables, i.e. length of variableNames
    /// @param variableNames String array, the name of the variables.
    void initTelemetry(char* const wifiSSID, char* const wifiPassword, unsigned int const numberOfVariables, String* variableNames, unsigned char const controlPeriodMs);

    /// @brief Log a variable for post-processing.
    /// @details The maximum length for a variable name is 6 characters, exceeding values will be cropped.
    ///          When performing telemetry, you should not use the serial port for anything else to avoid
    ///          messing up the Matlab script.
    /// @param[in] variableName Name of the variable.
    /// @param[in] variableValue Value of the variable
    /// @param[in] controlPeriodMs Control loop period, in ms. Do not use a value above 16ms as this will lead to unexpected results.
    /// @param[in] controlPeriodMs Control loop period, in ms. Do not use a value above 16ms as this will lead to unexpected results.
    void log(unsigned int const variableId, float const variableValue);

    /// @brief Actually execute the code - to be called in the loop method
    /// @note This function is blocking (endless loop)
    void run();
};
