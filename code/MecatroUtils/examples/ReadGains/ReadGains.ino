/* A demo of how to receive control gains from the computer over WiFi.

  Once the telemetry is started (see TelemetryDemo), call the function recieveGains
  to wait for n floats sent from Matlab using the provided function.
*/

// Include the current library
#include "MecatroUtils.h"

// Define the control loop period, in ms.
#define CONTROL_LOOP_PERIOD 5

// Define the name and password of the wifi network
#define WIFI_SSID "ArduinoMecatro"
#define WIFI_PASSWRD "password1234"

// Define the gains we want to read
float Kp = 0.0;
float Kd = 0.0;
float Ki = 0.0;

void setup()
{
  // Setup serial communication with the PC - for debugging and logging.
  // 230400 is the fastest speed for bluetooth ; if using USB, you can go up to
  // 1000000.
  Serial.begin(230400);

  // Initialize telemetry
  unsigned int const nVariables = 3;
  String variableNames[nVariables] = {"Kp" , "Kd", "Ki"};
  mecatro::initTelemetry(WIFI_SSID, WIFI_PASSWRD, nVariables, variableNames, CONTROL_LOOP_PERIOD);

  // We want 3 gains - we define an array of float large enough for it, then call
  // the corresponding function.
  float floatArray[3];
  mecatro::recieveGains(3, floatArray);
  // Now we attribute the gains
  Kp = floatArray[0];
  Kd = floatArray[1];
  Ki = floatArray[2];

  // Configure motor control and feedback loop call.
  mecatro::configureArduino(CONTROL_LOOP_PERIOD);
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
  // We send back the gains over telemetry
  mecatro::log(0,  Kp);
  mecatro::log(1,  Kd);
  mecatro::log(2,  Ki);

  // Keep the motor off, i.e. at 0 duty cycle (1 is full forward, -1 full reverse)
  mecatro::setMotorDutyCycle(0.0, 0.0);
}
