/* A demo of how to read data from the IMU, and send it over to Matlab by telemetry
*/

// Include the current library
#include "MecatroUtils.h"

// Include the IMU driver
#include "SparkFun_BMI270_Arduino_Library.h"

// Define the name and password of the wifi network
#define WIFI_SSID "ArduinoMecatro"
#define WIFI_PASSWRD "password1234"

// Define the control loop period, in ms.
#define CONTROL_LOOP_PERIOD 5

// Note that no multiplexer is used here: the IMU must be plugged into the I2C port of the Arduino directly.
BMI270 imu;

void setup()
{
  // Setup serial communication with the PC - for debugging and logging.
  Serial.begin(1000000);

  // Setup I2C communication
  Wire1.begin();
  if (imu.beginI2C(BMI2_I2C_PRIM_ADDR, Wire1) != BMI2_OK) {
    Serial.println("Failed to initialize IMU!");
  }
  else
  {
    // Set the I2C frequency to 400 kHz. **Must be done after initializing the IMU, otherwise gets overwritten.** 
    Wire1.setClock(400000);
    // Initialize telemetry
    unsigned int const nVariables = 6;
    String variableNames[nVariables] = {"accelX", "accelY", "accelZ", "gyroX", "gyroY", "gyroZ"};
    mecatro::initTelemetry(WIFI_SSID, WIFI_PASSWRD, nVariables, variableNames, CONTROL_LOOP_PERIOD);

 
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
  imu.getSensorData();

  mecatro::log(0, imu.data.accelX);
  mecatro::log(1, imu.data.accelY);
  mecatro::log(2, imu.data.accelZ);
  mecatro::log(3, imu.data.gyroX);
  mecatro::log(4, imu.data.gyroY);
  mecatro::log(5, imu.data.gyroZ);
}
