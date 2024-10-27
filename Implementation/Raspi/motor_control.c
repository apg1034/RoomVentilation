#include <unistd.h>
#include <stdio.h>
#include <wiringPi.h>
#include "motor_control.h"

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
}

void rotateMotor(int steps, int direction) {
    // Rotate the motor
    for (int i = 0; i < steps; i++) {
        for (int j = 0; j < 8; j++) {
            // Set the GPIO pins according to the step sequence
            digitalWrite(IN1, step_sequence[j][0]);
            digitalWrite(IN2, step_sequence[j][1]);
            digitalWrite(IN3, step_sequence[j][2]);
            digitalWrite(IN4, step_sequence[j][3]);
            usleep(1000); // Wait for a short time
        }
    }
}

