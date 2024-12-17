#include "constants.h"
#include <unistd.h>
#include <stdio.h>
#include <wiringPi.h>
#include "motor_control.h"
#include "encoder_control.h" // Include the encoder header for integration

// Step sequence for the 28BYJ-48 motor
int step_sequence[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};

void initializeMotor() {
    // Initialize wiringPi
    wiringPiSetupGpio(); // Use BCM GPIO numbering

    // Set the pins as outputs
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    printf("Motor initialized.\n");

}

void rotateMotor(int steps, int direction) {
    unsigned long stepStartTime;
    unsigned long stepInterval = 1;

    // Rotate the motor either clockwise or counterclockwise
    for (int i = 0; i < steps; i++) {
        for (int j = 0; j < 8; j++) {
            int step_index = direction > 0 ? 7 - j : j; // Reverse step sequence for counterclockwise
            digitalWrite(IN1, step_sequence[step_index][0]);
            digitalWrite(IN2, step_sequence[step_index][1]);
            digitalWrite(IN3, step_sequence[step_index][2]);
            digitalWrite(IN4, step_sequence[step_index][3]);
            
           stepStartTime = millis();
           while (millis() - stepStartTime < stepInterval) {
          }


        }
    }
}

void rotateMotorWithThreshold(int direction, int threshold) {
    printf("Starting motor rotation towards threshold: %d\n", threshold);

    while ((direction > 0 && getEncoderPosition() < threshold) ||
           (direction < 0 && getEncoderPosition() > threshold)) {
        rotateMotor(1, direction); // Rotate one step
        printf("Encoder Position: %d\n", getEncoderPosition());
    }

    printf("Threshold reached. Final Encoder Position: %d\n", getEncoderPosition());
}
