#include "encoder_control.h"
#include "constants.h"
#include "action_control.h"
#include "motor_control.h"
#include <stdio.h>
#include "fan_control.h"
#include <pthread.h>
#include "led_control.h"

// Define window states
typedef enum {
    WINDOW_IDLE,
    WINDOW_OPEN,
    WINDOW_CLOSED
} WindowState;

// Global variable to track the state of the window
static WindowState currentWindowState = WINDOW_IDLE;

void calibrateWindowPosition() {
    printf("Calibrating window position...\n");
    
    // Move the motor towards the closed direction until the encoder stops changing
    int previous_position = getEncoderPosition();
    int stable_count = 0; // Count how many times the position remains stable
    const int stable_threshold = 5; // Number of stable readings to confirm closed position

    while (stable_count < stable_threshold) {
        rotateMotor(1, -1); // Rotate one step in the closing direction
        int current_position = getEncoderPosition();
        printf("Calibrating... Current Encoder Position: %d\n", current_position);

        if (current_position == previous_position) {
            stable_count++;
        } else {
            stable_count = 0; // Reset if the position changes
            previous_position = current_position;
        }
    }

    resetEncoder(); // Set the current position as the "zero" point for closed state
    printf("Calibration complete. Encoder Position reset to: %d\n", getEncoderPosition());
}


void setAction(int value) {
    int current_position = getEncoderPosition(); // Get the current encoder position
    printf("Current Encoder Position: %d\n", current_position);
   
    // Handle the OPEN action
    if (value >= OPEN_THRESHOLD) {
        if (currentWindowState != WINDOW_OPEN) { // Only open if not already open
            // Requirement 6.3.3.2
            printf("Action: OPEN detected. Performing open operation.\n");
            currentWindowState = WINDOW_OPEN; // Update the state
            startFan();
            actionOpen();

        } else {
            printf("Window already fully open. No action taken.\n");
        }
    }
    // Handle the CLOSE action
    else if (value <= CLOSE_THRESHOLD) {
        if (currentWindowState != WINDOW_CLOSED) { // Only close if not already closed
                // Requirement 6.3.3.2
		printf("Action: CLOSE detected. Performing close operation.\n");
                actionClose();
                currentWindowState = WINDOW_CLOSED; // Update the state
           
        } else {
            printf("Window already fully closed. No action taken.\n");
        }
    } 

    else {
        printf("Action: IDLE detected. No action taken.\n");
        currentWindowState = WINDOW_IDLE; // Update the state
    }
}

void actionOpen() {
int current_position = getEncoderPosition();
    printf("Opening window. Initial Encoder Position: %d\n", current_position);
    turnOnLED();
    pthread_t motorThread, fanThread; // Thread identifiers
    int open_threshold = OPEN_THRESHOLD;

    // Start the fan in a background thread
    startFanBackground();

    // Rotate motor while fan is running
    printf("Rotating motor to open window...\n");
    rotateMotorWithThreshold(1, OPEN_THRESHOLD);

    // Requirement 6.1.4.1
    // Requirement 6.1.4.2
    // TODO - implement 6.1.4.2
    // Requirement 6.1.4.4
    // TODO - implement 6.1.4.4
    // Requirement 6.1.4.5
    // TODO - implement 6.1.4.5
    // Verify the position at the end
    current_position = getEncoderPosition();
    if (current_position < OPEN_THRESHOLD - 2) { // Allow 1 cm tolerance
        printf("Error: Window failed to reach open position. Current Encoder Position: %d\n", current_position);
    } else {
        printf("Window successfully opened. Final Encoder Position: %d\n", current_position);
    }
}

void actionClose() {
    int current_position = getEncoderPosition();
    printf("Closing window. Initial Encoder Position: %d\n", current_position);

   // Stop the fan immediately when closing starts
    stopFanBackground();

    // Rotate in the direction to decrease encoder position until the close threshold is reached
    printf("Rotating motor to close window...\n");
    rotateMotorWithThreshold(-1, CLOSE_THRESHOLD);

    // Requirement 6.1.4.3
    // TODO - implement 6.1.4.3
    // Requirement 6.1.4.4
    // TODO - implement 6.1.4.4
    // Requirement 6.1.4.5
    // TODO - implement 6.1.4.5
    // Verify the position at the end
    current_position = getEncoderPosition();
    if (current_position > CLOSE_THRESHOLD + 1) { // Allow 1 cm tolerance
        printf("Error: Window failed to reach close position. Current Encoder Position: %d\n", current_position);
        turnOffLED(); // Turn off LED if an error occurs

    } else {
        printf("Window successfully closed. Final Encoder Position: %d\n", current_position);
    }
   
}
