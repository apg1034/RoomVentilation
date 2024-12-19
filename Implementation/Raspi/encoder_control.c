// implements the encoder logic
#include "encoder_control.h"
#include <stdio.h>

// Global variables for encoder
volatile int encoder_position = 0;  // Tracks encoder steps
static int last_encoder_state = 0;  // Last state of CLK for edge detection


// initializes the encoder
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

/*
void encoderISR() {
    static unsigned long last_interrupt_time = 0; // Last ISR trigger time
    unsigned long interrupt_time = millis();

    // Debounce to ignore rapid noise or bouncing
    if (interrupt_time - last_interrupt_time < 5) { // Adjust debounce time (5ms works well for most encoders)
        return; // Ignore noise
    }

    // Read current states of CLK and DT
    int clk_state = digitalRead(ENCODER_CLK);
    int dt_state = digitalRead(ENCODER_DT);

    // Check transition and determine direction
    if (clk_state != last_encoder_state) {
        if (clk_state == dt_state) {
            encoder_position--; // Counterclockwise rotation
        } else {
            encoder_position++; // Clockwise rotation
        }
    }

    // Update states
    last_encoder_state = clk_state;
    last_interrupt_time = interrupt_time;

    // Debugging output to confirm clean transitions
    printf("ISR: CLK=%d, DT=%d, Position=%d\n", clk_state, dt_state, encoder_position);
}*/


// encoder interrupt service routine
/*void encoderISR() {
    static unsigned long last_interrupt_time = 0; // To debounce the signal
    unsigned long interrupt_time = millis();

    // Debounce condition: Ignore fast successive signals
    if (interrupt_time - last_interrupt_time > 5) { // 5 ms debounce time
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
    last_interrupt_time = interrupt_time;
}*/


void encoderISR() {
    int current_state = digitalRead(ENCODER_CLK);

    // Detect direction based on DT pin
    if (current_state != last_encoder_state) {
        if (digitalRead(ENCODER_DT) != current_state) {
            encoder_position++; // Clockwise rotation
        } else {
            encoder_position--; // Counterclockwise rotation
        }
    }
    last_encoder_state = current_state;
}


// resets encode
void resetEncoder() {
    encoder_position = 0; // Reset encoder position to 0
}

// reads encoder position
int getEncoderPosition() {
    return encoder_position; // Return current encoder position
}
