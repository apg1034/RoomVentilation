#include <wiringPi.h>
#include "led_control.h"

void initializeLED() {
   
    wiringPiSetupGpio(); // Use BCM GPIO numbering

    // Set the LED pin as output
    pinMode(LED_PIN, OUTPUT);
}

void turnOnLED() {
    digitalWrite(LED_PIN, HIGH); // Turn on the LED
}

void turnOffLED() {
    digitalWrite(LED_PIN, LOW); // Turn off the LED
}
