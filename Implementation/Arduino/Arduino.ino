#include "MICS-VZ-89TE.h"

MICS_VZ_89TE CO2SENSOR;

#define LED_PIN 3               // Pin for the status LED
#define ERROR_LED_PIN 4         // Pin for the error LED
#define CO2_THRESHOLD 450       // CO2 level threshold in ppm
#define NUM_READINGS 5          // Number of readings for average calculation

float previousReadings[NUM_READINGS]; // Array to store previous readings
int readingIndex = 0;  // Current index for previous readings
bool initialized = false; // Flag to check if readings are initialized

unsigned long previousMillis = 0;      // Store last time we read the sensor
const long interval = 10000;            // Interval for reading sensor (10 seconds)
bool ledState = false;                  // Track LED state
unsigned long ledPreviousMillis = 0;    // Store last time LED was updated
const long ledInterval = 500;           // LED blink interval (500 ms)
bool isError = false;                   // Error state

void setup() {
    // Open serial communications and wait for port to open:
    Serial.begin(57600);
    while (!Serial) {
        ; // Wait for serial port to connect. Needed for native USB port only
    }

    // Initialize LED pins
    pinMode(LED_PIN, OUTPUT);
    pinMode(ERROR_LED_PIN, OUTPUT);

    // Turn off the error LED to indicate no error
    digitalWrite(ERROR_LED_PIN, LOW);

    // Call the begin function for the sensor
    CO2SENSOR.begin();
    Serial.println("MICS-VZ-89TE Sensor reading process started.");
}

void loop() {
    unsigned long currentMillis = millis();

    // Read CO2 and VOC values from the sensor every 10 seconds
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis; // Save the last reading time

        // Read sensor values
        CO2SENSOR.readSensor();
        float co2 = CO2SENSOR.getCO2();
        float voc = CO2SENSOR.getVOC();

        // Debug: Print raw readings
        Serial.print("Raw CO2: ");
        Serial.println(co2, 3);
        Serial.print("Raw VOC: ");
        Serial.println(voc, 3);

        // If this is the first valid reading, initialize previous readings
        if (!initialized) {
            previousReadings[readingIndex] = co2;
            readingIndex = (readingIndex + 1) % NUM_READINGS;
            // Check if we have enough readings to initialize
            if (readingIndex == 0) {
                initialized = true; // Mark as initialized after first cycle
                Serial.println("Initialized previous readings with the first valid CO2 reading.");
            }
            return; // Skip further processing until initialized
        }

        // Store the current reading in the previousReadings array
        previousReadings[readingIndex] = co2;
        readingIndex = (readingIndex + 1) % NUM_READINGS;

        // Calculate the average of the last NUM_READINGS
        float averageCO2 = 0;
        for (int i = 0; i < NUM_READINGS; i++) {
            averageCO2 += previousReadings[i];
        }
        averageCO2 /= NUM_READINGS;

        // Mark the reading as invalid if it deviates more than 50 ppm from the average
        if (abs(co2 - averageCO2) > 50) {
            Serial.print("CO2 reading marked as invalid: ");
            Serial.println(co2, 3);
            isError = true;  // Set error state
            digitalWrite(ERROR_LED_PIN, HIGH); // Turn on error LED
            return; // Drop the invalid reading
        } else {
            isError = false; // Clear error state if reading is valid
            digitalWrite(ERROR_LED_PIN, LOW); // Turn off error LED
        }

        // Print the valid CO2 and VOC readings
        Serial.print("Valid CO2: ");
        Serial.println(co2, 3);
        Serial.print("Valid VOC: ");
        Serial.println(voc, 3);

        // Check CO2 level and blink LED if above threshold
        if (co2 > CO2_THRESHOLD) {
            ledState = true; // Set LED state to HIGH
        } else {
            ledState = false; // Set LED state to LOW
        }
    }

    // Manage LED blinking
    if (ledState) {
        if (currentMillis - ledPreviousMillis >= ledInterval) {
            ledPreviousMillis = currentMillis; // Save the last LED update time
            digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Toggle the LED
        }
    } else {
        digitalWrite(LED_PIN, LOW); // Ensure LED is off if CO2 is below threshold
    }
}
