// implements LED control
#include <wiringPi.h>
#include "led_control.h"

// initializes LED
void initializeLED() {
   
    wiringPiSetupGpio(); // Use BCM GPIO numbering

    // Set the LED pin as output
    pinMode(LED_PIN, OUTPUT);
}

// tunrs LED on
void turnOnLED() {
    digitalWrite(LED_PIN, HIGH); // Turn on the LED
}

// turns LED off
void turnOffLED() {
    digitalWrite(LED_PIN, LOW); // Turn off the LED
}
