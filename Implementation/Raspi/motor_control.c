// implements stepper motor control
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

// initializes Motor
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

// rotates Motor
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

// rotates motor till threshold
void rotateMotorWithThreshold(int direction, int threshold) {
    printf("Starting motor rotation towards threshold: %d\n", threshold);

    int last_position = -1; // Keep track of the last printed position
    int current_position;

    unsigned long last_update_time = millis(); // Track the last time the motor was updated
    unsigned long step_interval = 10;         // Time interval between updates (in milliseconds)

    while ((direction > 0 && (current_position = getEncoderPosition()) < threshold) || // Use < instead of <=
           (direction < 0 && (current_position = getEncoderPosition()) > threshold)) { // Use > instead of >=
        // Check if it's time to step the motor
        if (millis() - last_update_time >= step_interval) {
            rotateMotor(1, direction); // Rotate one step
            last_update_time = millis(); // Update the last update time
        }

        // Print encoder position only when it changes
       if (current_position != last_position) {
         printf("Encoder Position: %d\n", current_position);
           last_position = current_position; // Update the last printed position
       }
    }

    // Explicitly print the threshold position if the loop exits
    printf("Threshold reached. Final Encoder Position: %d\n", threshold);
}
