#ifndef ENCODER_CONTROL_H
#define ENCODER_CONTROL_H

#include <wiringPi.h>

// Encoder Pin Definitions
#define ENCODER_CLK 6   // GPIO6 (Pin 31) - Clock
#define ENCODER_DT  20  // GPIO20 (Pin 38) - Data
#define ENCODER_SW  21  // GPIO21 (Pin 40) - Switch (Button)

// Function prototypes
void initializeEncoder();
void resetEncoder();
int getEncoderPosition();
int isEncoderSwitchPressed();
void encoderISR();

#endif // ENCODER_CONTROL_H
