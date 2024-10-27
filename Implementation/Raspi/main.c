#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "motor_control.h"
#include "led_control.h"

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

    // Replace with test loop, simulating CO2 and VOC readings
    while (1) {
        // Simulating CO2 and VOC readings
        float co2 = rand() % 1000; // Simulate a random CO2 value (0-999)
        float voc = rand() % 1000; // Simulate a random VOC value (0-999)

        printf("CO2: %.2f, VOC: %.2f\n", co2, voc);

        // Check CO2 threshold and trigger motor and LED if needed
        if (co2 > CO2_THRESHOLD) {
            trigger_motor_and_led();
        }

        sleep(10); // Read every 10 seconds
    }

    return 0;
}
