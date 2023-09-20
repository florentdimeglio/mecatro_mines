/* Demo code for reading the encoders of the robot.

  Note: this code requires the following libraries (install them through the library manager):
     - SparkFun I2C Mux Arduino Library
     - AS5600 library
*/

// Include the current library
#include "MecatroUtils.h"

// Include the AS5600 library (for the encoders) and Sparkfun I2C Mux (for multiplexer)
#include "AS5600.h"
#include "SparkFun_I2C_Mux_Arduino_Library.h"

// Header for I2C communication
#include "Wire.h"

// Define the control loop period, in ms.
#define CONTROL_LOOP_PERIOD 5

// Define the Multiplexer pins corresponding to each encoder
#define LEFT_ENCODER_PIN
#define RIGHT_ENCODER_PIN

QWIICMUX multiplexer;
AS5600 rightEncoder, leftEncoder;

void setup()
{
  // Setup serial communication with the PC - for debugging and logging.
  Serial.begin(1000000);
  // Start I2C communication
  Wire.begin();
  // Set I2C clock speed to 400kHz (fast mode)
  Wire.setClock(400000);

  // Init multiplexer
  if (!multiplexer.begin())
  {
    Serial.println("Error: I2C multiplexer not found. Check wiring.");
  }
  else
  {
    bool isInit = true;
    // Set multiplexer to use port RIGHT_ENCODER_PIN to talk to right encoder.
    multiplexer.setPort(RIGHT_ENCODER_PIN);
    rightEncoder.begin();
    if (!rightEncoder.isConnected())
    {
      Serial.println("Error: could not connect to right encoder. Check wiring.");
      isInit = false;
    }
    // Set multiplexer to use port LEFT_ENCODER_PIN to talk to left encoder.
    multiplexer.setPort(LEFT_ENCODER_PIN);
    leftEncoder.begin();
    if (!leftEncoder.isConnected())
    {
      Serial.println("Error: could not connect to left encoder. Check wiring.");
      isInit = false;
    }

    if (isInit)
    {
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


// This function is called periodically, every CONTROL_LOOP_PERIOD ms.
// Put all your code here.
void mecatro::controlLoop()
{
  // Set multiplexer to use port 0 to talk to right encoder.
  multiplexer.setPort(RIGHT_ENCODER_PIN);
  Serial.print("Right encoder: raw angle ");
  // Raw encoder measurement - from 0 to 360 degrees
  Serial.print(rightEncoder.rawAngle() * AS5600_RAW_TO_DEGREES);

  // Software feature: the encoder itself does not measure multi-turn information nor rotation speed.
  // These features are thus implemented as software, taking two consequtive measurements and computing
  // their difference.
  // This of course assumes that the encoder has performed less than half of a turn between two calls (otherwise there
  // is no way to know how many turns were performed, or in which direction).
  // This is not a problem here: with a typical update rate of 5ms in this function, the maximum speed would be 60000rpm !
  Serial.print("°, cumulative position ");
  Serial.print(rightEncoder.getCumulativePosition() * AS5600_RAW_TO_DEGREES);
  Serial.print("° speed ");
  Serial.print(rightEncoder.getAngularSpeed());
  Serial.print("°/s ");

  // Check magnet positioning - this is for debug purposes only and is not required in normal operation.
  if (rightEncoder.magnetTooStrong())
  {
    Serial.print(" ; warning: magnet too close.");
  }
  if (rightEncoder.magnetTooWeak())
  {
    Serial.print(" ; warning: magnet too far.");
  }
  Serial.println();

  // Set multiplexer to use port LEFT_ENCODER_PIN to talk to left encoder.
  multiplexer.setPort(LEFT_ENCODER_PIN);
  Serial.print("Left encoder: ");
  Serial.print(leftEncoder.rawAngle() * AS5600_RAW_TO_DEGREES);
  Serial.print("°, cumulative position ");
  Serial.print(leftEncoder.getCumulativePosition() * AS5600_RAW_TO_DEGREES);
  Serial.print("° speed ");
  Serial.print(leftEncoder.getAngularSpeed());
  Serial.print("°/s ");

  // Check magnet positioning - this is for debug purposes only and is not required in normal operation.
  if (leftEncoder.magnetTooStrong())
  {
    Serial.print(" ; warning: magnet too close.");
  }
  if (leftEncoder.magnetTooWeak())
  {
    Serial.print(" ; warning: magnet too far.");
  }
  Serial.println();

  // Keep the motor off, i.e. at 0 duty cycle (1 is full forward (i.e. trigonometric sense in the motor frame), -1 full reverse)
  mecatro::setMotorDutyCycle(0.0, 0.0);
}