#include <wiringPi.h>
#include "fan_control.h"
#include <stdio.h>

unsigned long previousMillis = 0; // Stores the last time the motor was updated
const long interval = 10;  // Interval between steps in milliseconds

// Fan Motor Step Sequence
int fan_step_sequence[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};

void initializeFanMotor() {
    // Initialize wiringPi for fan motor
    printf("Initializing fan motor...\n");
    if (wiringPiSetupGpio() == -1) {
        printf("wiringPi setup failed!\n");
        return;
    }
    pinMode(FAN_IN1, OUTPUT);
    pinMode(FAN_IN2, OUTPUT);
    pinMode(FAN_IN3, OUTPUT);
    pinMode(FAN_IN4, OUTPUT);
    printf("Fan Motor initialized.\n");
}

void startFan() {
    printf("Starting fan...\n");
    rotateFanMotor(1000, 1);  // Rotate 10 steps in the forward direction
    printf("Fan started.\n");
}

void stopFan() {
    // Rotate the fan motor to turn the fan off
    rotateFanMotor(1000, -1);  // Reverse direction to stop the fan
    printf("Fan stopped.\n");
}

void rotateFanMotor(int steps, int direction) {
    unsigned long currentMillis;
    int step_index;

    for (int i = 0; i < steps; i++) {
        currentMillis = millis(); // Get current time in milliseconds

        // Rotate motor only if the interval has passed
        if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis;  // Save the last time we updated the motor

            printf("Rotating step %d: direction %d\n", i + 1, direction);

            // Update motor step sequence
            for (int j = 0; j < 8; j++) {
                step_index = direction > 0 ? 7 - j : j;  // Reverse step sequence for counterclockwise

                digitalWrite(FAN_IN1, fan_step_sequence[step_index][0]);
                digitalWrite(FAN_IN2, fan_step_sequence[step_index][1]);
                digitalWrite(FAN_IN3, fan_step_sequence[step_index][2]);
                digitalWrite(FAN_IN4, fan_step_sequence[step_index][3]);

                // Non-blocking: don't use delay, it allows other tasks to run.
                // We break the loop after each step to check time next iteration
                break;
            }
        }
    }
}
