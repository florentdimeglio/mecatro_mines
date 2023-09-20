/* A demo of how to perform telemetry, i.e. get structured data out of the Arduino.
   To analyze the behavior and performance of our controller, we want to be able to plot
   the evolution of some variables through time (target, current state, command...).

   This is done through the log function of MecatroUtils, that will send the data. A master
   script in Matlab will then listen to this data and store it in a nice table.
   
   mecatro::log takes two arguments: the name of the variable (max 20 characters) and its value.

   As example, we here output the value of sin and cos with a period of 2 pi.

   NOTE: when performing telemetry, you should NOT print anything else than this telemetry, otherwise the Matlab script might crash.
*/

// Include the current library
#include "MecatroUtils.h"

// Define the control loop period, in ms.
#define CONTROL_LOOP_PERIOD 5


void setup()
{
  // Setup serial communication with the PC - for debugging and logging.
  // 230400 is the fastest speed for bluetooth ; if using USB, you can go up to 
  // 1000000.
  Serial.begin(230400);

  // Initialize telemetry
  unsigned int const nVariables = 2;
  String variableNames[nVariables] = {"sin" , "cos"};
  mecatro::initTelemetry(nVariables, variableNames);
  
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
  // Log the value of sin and cos as example.
  // The first argument is the variable (column) id ; recall that in C++, numbering starts at 0.
  mecatro::log(0,  sin(millis() / 1000.0)); // sin
  mecatro::log(1,  cos(millis() / 1000.0)); // cos

  // Keep the motor off, i.e. at 0 duty cycle (1 is full forward, -1 full reverse)
  mecatro::setMotorDutyCycle(0.0, 0.0);
}
