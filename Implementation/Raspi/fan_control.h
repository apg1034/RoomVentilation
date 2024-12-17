#ifndef FAN_CONTROL_H
#define FAN_CONTROL_H

// Fan Motor Pins
#define FAN_IN1 17  // GPIO 17
#define FAN_IN2 27  // GPIO 27
#define FAN_IN3 22  // GPIO 22
#define FAN_IN4 5   // GPIO 5

#define FAN_STEPS_PER_REV 2048 // Full revolution for 28BYJ-48 (can adjust if necessary)

void initializeFanMotor();              // Function to initialize the fan motor
void rotateFanMotor(int steps, int direction);  // Function to rotate the fan motor in steps
void startFan();                        // Function to start the fan motor
void stopFan();                         // Function to stop the fan motor

#endif // FAN_CONTROL_H
