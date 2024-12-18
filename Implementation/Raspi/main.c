#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "fan_control.h"
#include "motor_control.h"
#include "led_control.h"
#include "bluetooth_control.h"
#include "crypto_control.h"
#include "action_control.h"
#include "encoder_control.h"
#include "constants.h"

int main() {
    // Initialize motor, LED, and encoder
    initializeMotor();
    // Requirement 6.1.1.2
    // Requirement 6.1.1.3
    initializeLED();
    initializeEncoder();
    calibrateWindowPosition();
    initializeFanMotor();
    printf("Motor, LED, and Encoder Initialized.\n");
// Perform initial open and close check
    printf("Performing initial window open and close check...\n");

    // Requirement 6.1.1.6
    // Perform initial window open and close check
    printf("Performing initial window open and close check...\n");
    actionOpen();

    if (getEncoderPosition() >= OPEN_THRESHOLD - 2) { // Verify successful opening
        actionClose();

        if (getEncoderPosition() <= CLOSE_THRESHOLD + 2) { // Verify successful closing
            printf("Window fully opened and closed successfully. Keeping LED ON.\n");
            turnOnLED(); // LED stays on after successful check
        } else {
            printf("Initial close failed. Turning LED OFF.\n");
            turnOffLED();
        }
    } else {
        printf("Initial open failed. Turning LED OFF.\n");
        turnOffLED();
    }
   
    // Initialize Bluetooth
    if (initializeBluetooth()) {
        printf("Bluetooth initialized.\n");
    } else {
        printf("Cannot initialize Bluetooth.\n");
	turnOffLED();
        return EXIT_FAILURE; // Exit the program if Bluetooth initialization fails
    }

    // Run the Bluetooth measurement loop
    if (!runBluetooth()) {
        printf("Program exit by user... \n");
    } else {
        printf("Program exit with failure ... \n");
    }

 // Clean up
    turnOffLED(); // Ensure the LED is turned off before exiting
    printf("System shutdown complete.\n");

    return EXIT_SUCCESS;
}
