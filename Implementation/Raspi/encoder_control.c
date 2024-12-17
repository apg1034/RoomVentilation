#include "encoder_control.h"
#include <stdio.h>

// Global variables for encoder
volatile int encoder_position = 0;  // Tracks encoder steps
static int last_encoder_state = 0;  // Last state of CLK for edge detection


void initializeEncoder() {
    // Initialize wiringPi library
    wiringPiSetupGpio(); // Use BCM GPIO numbering

    // Set encoder pins as inputs with pull-up resistors
    pinMode(ENCODER_CLK, INPUT);
    pinMode(ENCODER_DT, INPUT);
    pullUpDnControl(ENCODER_CLK, PUD_UP);
    pullUpDnControl(ENCODER_DT, PUD_UP);

    // Initialize encoder state
    last_encoder_state = digitalRead(ENCODER_CLK);

    // Attach interrupt for CLK pin
    wiringPiISR(ENCODER_CLK, INT_EDGE_BOTH, &encoderISR);
    printf("Encoder initialized. Waiting for pulses...\n");
}


void encoderISR() {
    int current_state = digitalRead(ENCODER_CLK);

    if (current_state != last_encoder_state) {
        if (digitalRead(ENCODER_DT) != current_state) {
            encoder_position++;

       
 } else {
            encoder_position--;


        }
    }
    last_encoder_state = current_state;
}

void resetEncoder() {
    encoder_position = 0; // Reset encoder position to 0
}

int getEncoderPosition() {
    return encoder_position; // Return current encoder position
}
