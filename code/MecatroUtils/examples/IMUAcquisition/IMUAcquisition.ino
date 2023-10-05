/* A demo of how to read data from the IMU, and send it over to Matlab by telemetry
*/

// Include the current library
#include "MecatroUtils.h"

// Include the IMU driver
#include "ICM_20948.h"

// Define the control loop period, in ms.
#define CONTROL_LOOP_PERIOD 5

// Note that no multiplexer is used here: the IMU must be plugged into the I2C port of the Arduino directly.
ICM20948 imu;

void setup()
{
  // Setup serial communication with the PC - for debugging and logging.
  Serial.begin(1000000);

  // Initialize telemetry
  unsigned int const nVariables = 6;
  String variableNames[nVariables] = {"accelX", "accelY", "accelZ", "gyroX", "gyroY", "gyroZ"};
  mecatro::initTelemetry(nVariables, variableNames);

  if (!imu.init())
  {
      Serial.println("Error communicating with IMU. Check wiring");
  }
  else
  {
      // Configure motor control and feedback loop call.
      mecatro::configureArduino(CONTROL_LOOP_PERIOD);
  }
}

void loop()
{
  // Don't forget to call this, otherwise nothing will happen !
  // This function never returns, put all your code inside mecatro::controlLoop.
  mecatro::run();
}


// This function is called periodically, every CONTROL_LOOP_PERIOD ms.
// Put all your code here.
void mecatro::controlLoop()
{
  // Read data from IMU.
  IMUData const imuData = imu.read();
  // On very rare occasion, the IMU reading might fail. It's best to take that into account
  // to avoid unwanted control behavior
  if (!imuData.isValid)
  {
    mecatro::log(0, imuData.accelX);
    mecatro::log(1, imuData.accelY);
    mecatro::log(2, imuData.accelZ);
    mecatro::log(3, imuData.gyroX);
    mecatro::log(4, imuData.gyroY);
    mecatro::log(5, imuData.gyroZ);
  }
}
