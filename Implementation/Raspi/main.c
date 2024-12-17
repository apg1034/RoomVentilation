#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

<<<<<<< HEAD
#include "fan_control.h"
=======
>>>>>>> adea0023f5c375dfcd32ecb78233ae5518a6c95d
#include "motor_control.h"
#include "led_control.h"
#include "bluetooth_control.h"
#include "crypto_control.h"
<<<<<<< HEAD
#include "action_control.h"
#include "encoder_control.h"
#include "constants.h"

int main() {
    // Initialize motor, LED, and encoder
    initializeMotor();
    initializeLED();
    initializeEncoder();
    calibrateWindowPosition();
    printf("Motor, LED, and Encoder Initialized.\n");

    // Initialize Bluetooth
    if (initializeBluetooth()) {
        printf("Bluetooth initialized.\n");
    } else {
        printf("Cannot initialize Bluetooth.\n");
        return EXIT_FAILURE; // Exit the program if Bluetooth initialization fails
    }
=======

#define CO2_THRESHOLD 450

void trigger_motor_and_led() {
    printf("CO2 level above threshold. Triggering motor and LED.\n");
    turnOnLED();          // Turn on the LED
    rotateMotor(STEPS_PER_REV, 1); // Rotate the motor clockwise
}

int main() {
    
	// Initialize motor and LED
    
	initializeMotor();
    initializeLED();
	
    printf("Motor and LED initialized.\n");

	// Initialize Bluetooth
	
	if(true == initializeBluetooth())
	{
		printf("Bluetooth initialized.\n");
	}
	else
	{
		printf("Cannot initialize Bluetooth.\n");
	}
	
	// Run measurement loop
	
	if(false == runBluetooth())
	{
		printf("Program exit by user... \n");
	}
	else
	{
		printf("Program exit with failure ... \n");
	}
>>>>>>> adea0023f5c375dfcd32ecb78233ae5518a6c95d

    // Run the Bluetooth measurement loop
    if (!runBluetooth()) {
        printf("Program exit by user... \n");
    } else {
        printf("Program exit with failure ... \n");
    }

    // Main operation loop
    while (1) {
        int current_position = getEncoderPosition();

        if (current_position < OPEN_THRESHOLD) {
            printf("Opening window. Current Encoder Position: %d\n", current_position);
            rotateMotorWithThreshold(1, OPEN_THRESHOLD);
            printf("Window opened. Final Encoder Position: %d\n", getEncoderPosition());
        }

        if (current_position > CLOSE_THRESHOLD) {
            printf("Closing window. Current Encoder Position: %d\n", current_position);
            rotateMotorWithThreshold(-1, CLOSE_THRESHOLD);
            printf("Window closed. Final Encoder Position: %d\n", getEncoderPosition());
        }
    }

    // Clean up
    turnOffLED(); // Ensure the LED is turned off before exiting
    printf("System shutdown complete.\n");

    return EXIT_SUCCESS;
}
