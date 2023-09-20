/* This is a basic Hello world program to demonstrate the working principle of
   our Arduino code, in the context of the Mecatro Course.
   
   "Arduino software" is basically some simple C++ code, compiled for AVR microcontrollers.
   Arduino itself adds a layer of helper functions above the standard C++ supported by avr-gcc, 
   the underlying compiler.

   A pure C/C++ code starts with a function main ; Arduino however hides this function and instead
   provides two functions for the user to implement:
      - setup: called once to perform the initial setup
      - loop: called in an endless loop, executes the actual code
    
   The equivalent code is:

   void main()
   {
     setup(); // Called once
     while (true)
     {
       loop(); // Called forever
     }
   }

   Now our use case is to perform motor control at a fixed control frequency, i.e. have a controller
   being called precicely every N milliseconds.
   Both of these features (motor control signal generation and fixed-frequency function call) are performed
   by the MecatroUtils library (using TIMER1 for hardware PWM generation and interruption to trigger the main
   process, see the source code for more details) through two functions:
    - controlLoop: a function to be implemented by the user, that will be called every N milliseconds
    - configureArduino: configure the timer (signal generation and interrupt), meant to be called during the setup phase.
    
  Thus, the following is a minmum working example.
*/


// Include the library
#include "MecatroUtils.h"

// Define the control loop period, in ms.
// Here we decide to run every 5ms i.e. at 200Hz.
#define CONTROL_LOOP_PERIOD 5

// Setup function, called once at the start.
void setup()
{
  // Setup serial communication with the PC - for debugging. Note that you need to configure the Arduino IDE
  // Serial Monitor to use the same frequency, otherwise the displayed result will be garbage.
  Serial.begin(1000000);

  // Configure motor control and interruption to have controlLoop called every CONTROL_LOOP_PERIOD i.e. 5ms
  mecatro::configureArduino(CONTROL_LOOP_PERIOD);
}


// Loop: called endlessly.
void loop()
{
  // The run function will handle the interrupt, and is responsible for calling controlLoop at the
  // specified time.
  // Don't forget to call this, otherwise nothing will happen !
  // This function never returns, put all your code inside mecatro::controlLoop.
  mecatro::run();
}


// Here is the interesting part: user-code.
// There you should put all you want your controller to do.
// This function is called periodically, every CONTROL_LOOP_PERIOD ms.
void mecatro::controlLoop()
{
  // Output Hello world, and print the current time, in millisecond: that way we can verify that the function is
  // indeed called at the right frequency.
  Serial.print("Hello world ! Time: ");
  Serial.println(millis());

  // Keep the motor off, i.e. at 0 duty cycle (1 is full forward, -1 full reverse)
  mecatro::setMotorDutyCycle(0.0, 0.0);
}

