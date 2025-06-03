#include <SparkFun_I2C_Mux_Arduino_Library.h>

/* Identification of the motor response: angular position versus input duty cycle
*/

// Include the current library
#include "MecatroUtils.h"


// Include the AS5600 library (for the encoders) and Sparkfun I2C Mux (for muliplexer)
#include "AS5600.h"
#include "SparkFun_I2C_Mux_Arduino_Library.h"

// Header for I2C communication
#include "Wire.h"

QWIICMUX multiplexer;
AS5600 rightEncoder(&Wire1);
AS5600 leftEncoder(&Wire1);

// Define the control loop period, in ms.
#define CONTROL_LOOP_PERIOD 5
// Associate the multiplexer PINs with their respective encoders
#define LEFT_ENCODER_PIN 3
#define RIGHT_ENCODER_PIN 2

// Define the name and password of the wifi network
#define WIFI_SSID "ArduinoMecatro"
#define WIFI_PASSWRD "password1234"

// Target signal: slow sawtooth ; period in ms
// int const PERIOD = 5000;
// float const QUARTER_PERIOD = PERIOD / 4.0;

// Initialize time at zero
float mytime = 0;

double Ubar;

void mecatro::controlLoop()
{
  //Update time variable
  mytime += CONTROL_LOOP_PERIOD / 1e3;
   
  // TO CHANGE: CHOOSE AN EXCITATION SIGNAL (Voltage)
  Ubar = 0;

  // Log duty cycle, left angle and right angle.
  mecatro::log(0,  Ubar);
  multiplexer.setPort(LEFT_ENCODER_PIN);
  mecatro::log(1,  leftEncoder.getCumulativePosition() * AS5600_RAW_TO_RADIANS);
  multiplexer.setPort(RIGHT_ENCODER_PIN);
  mecatro::log(2,  rightEncoder.getCumulativePosition() * AS5600_RAW_TO_RADIANS);

  // Send control input to the motors
  mecatro::setMotorDutyCycle(Ubar / 12., Ubar / 12.);
}


void setup()
{
  // Setup serial communication with the PC - for debugging and logging. Max 1Mbps, then you lose threads.
  Serial.begin(1000000);

  // Start I2C communication  
  Wire1.begin();
  // Set I2C clock speed to 400kHz (fast mode)
  Wire1.setClock(400000);


  // Init muliplexer
  if (!multiplexer.begin(0x70, Wire1))
  {
    Serial.println("Error: I2C muliplexer not found. Check wiring.");
  }
  else
  {
    bool isInit = true;
    // Set muliplexer to use port 0 to talk to right encoder.
    multiplexer.setPort(LEFT_ENCODER_PIN);
    rightEncoder.begin();
    if (!rightEncoder.isConnected())
    {
      Serial.println("Error: could not connect to right encoder. Check wiring.");
      isInit = false;
    }
    multiplexer.setPort(RIGHT_ENCODER_PIN);
    leftEncoder.begin();
    if (!leftEncoder.isConnected())
    {
      Serial.println("Error: could not connect to left encoder. Check wiring.");
      isInit = false;
    }
    else 
    {
      // Initialize telemetry
      unsigned int const nVariables = 3;
      String variableNames[nVariables] = {"dutyCycle", "thetaL", "thetaR"};
      mecatro::initTelemetry(WIFI_SSID, WIFI_PASSWRD, nVariables, variableNames, CONTROL_LOOP_PERIOD);
      // Configure motor control and feedback loop call.
      mecatro::configureArduino(CONTROL_LOOP_PERIOD);
    }
  }
}


void loop()
{
  // Don't forget to call this, otherwise nothing will happen !
  // This function never returns, put all your code inside mecatro::controlLoop.
  mecatro::run();
}
