#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "motor_control.h"
#include "led_control.h"
#include "bluetooth_control.h"

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

    return 0;
}
