# MecatroUtils

Utility functions for the mechatronics projects at Ecole des Mines.

The functions in this library performs required low-level configuration of the Arduino, namely:

 - hardware PWM generation (using TIMER1) for motor PWM signal.
 - time-based interrupt (using TIMER2) for the control loop.

The library also contains a modified, efficient library for reading the Sparkfun ICM_20948 IMU.
 
