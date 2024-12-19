// implements fan control
#include <wiringPi.h>
#include "fan_control.h"
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

unsigned long previousMillis = 0; // Stores the last time the motor was updated
const long interval = 10;  // Interval between steps in milliseconds

bool isFanRunning = false;   // Global flag to control fan state
pthread_t fanThread;         // Fan thread identifier


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

// initializes the fan motor
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

// creates threat for fan
void* fanControlThread(void* arg) {
    printf("Fan thread started. Fan is running...\n");
    while (isFanRunning) {
        rotateFanMotor(100, 1); // Run the fan motor steps
    }
    printf("Fan thread stopping. Fan is turned off.\n");
    return NULL;
}

// starts fan in background
void startFanBackground() {
    if (!isFanRunning) { // Only start if the fan is not already running
        isFanRunning = true;
        pthread_create(&fanThread, NULL, fanControlThread, NULL);
    }
}

// stops fan in background
void stopFanBackground() {
    if (isFanRunning) { // Stop the fan thread
        isFanRunning = false;
        pthread_join(fanThread, NULL);
        stopFan();
    }
}

// starts fan
void startFan() {
    printf("Starting fan...\n");
    rotateFanMotor(1000, 1);  // Rotate 10 steps in the forward direction
    printf("Fan started.\n");
}

// stops fan
void stopFan() {
    printf("Fan stopped.\n");
}

// rotates motor based on input
void rotateFanMotor(int steps, int direction) {
     unsigned long stepStartTime;
    unsigned long stepInterval = 1;

    // Rotate the motor either clockwise or counterclockwise
    for (int i = 0; i < steps; i++) {
        for (int j = 0; j < 8; j++) {
            int step_index = direction > 0 ? 7 - j : j; // Reverse step sequence for counterclockwise
            digitalWrite(FAN_IN1, fan_step_sequence[step_index][0]);
            digitalWrite(FAN_IN2, fan_step_sequence[step_index][1]);
            digitalWrite(FAN_IN3, fan_step_sequence[step_index][2]);
            digitalWrite(FAN_IN4, fan_step_sequence[step_index][3]);
            
           stepStartTime = millis();
           while (millis() - stepStartTime < stepInterval) {
          }
        }
    }

}
