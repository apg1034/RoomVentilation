#include <ArduinoBLE.h>
#include "MICS-VZ-89TE.h"

MICS_VZ_89TE CO2SENSOR;

// LED and sensor parameters
#define LED_PIN 3
#define ERROR_LED_PIN 4
#define CO2_THRESHOLD 450
#define NUM_READINGS 5

// Variables for averaging
float previousReadings[NUM_READINGS];
int readingIndex = 0;
bool initialized = false;

// BLE characteristics
BLEService sensorService("12345678-1234-5678-1234-56789abcdef0");
BLEIntCharacteristic co2Characteristic("87654321-4321-6789-4321-fedcba987654", BLERead | BLENotify);
BLEIntCharacteristic vocCharacteristic("abcdef01-2345-6789-0123-456789abcdef", BLERead | BLENotify);


// Timing
unsigned long previousMillis = 0;
const long interval = 10000; // 10 seconds
unsigned long noCentralPreviousMillis = 0;
const long noCentralInterval = 1000; // 1 second for BLE advertising

void setup() {
    Serial.begin(57600);
    while (!Serial) { ; }

    pinMode(LED_PIN, OUTPUT);
    pinMode(ERROR_LED_PIN, OUTPUT);
    digitalWrite(ERROR_LED_PIN, LOW);

    CO2SENSOR.begin();

    if (!BLE.begin()) {
        Serial.println("Starting BLE failed!");
        while (1);
    }

    // Set BLE properties
    BLE.setLocalName("ArduinoNano33IoT");
    BLE.setAdvertisedService(sensorService);
    BLE.setAdvertisingInterval(50); // Set to 50 ms
    sensorService.addCharacteristic(co2Characteristic);
    sensorService.addCharacteristic(vocCharacteristic);
    BLE.addService(sensorService);

    BLE.advertise();
    Serial.println("BLE advertising and sensor started...");
}

void loop() {
    BLEDevice central = BLE.central();
    delay(5000);

    if (central) {
        Serial.print("Connected to central: ");
        Serial.println(central.address());

        unsigned long currentMillis = millis();

        // Read and send data every interval
        if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis;

            // Read sensor values
            CO2SENSOR.readSensor();
            int co2 = 400;//CO2SENSOR.getCO2();
            int voc = 50;//CO2SENSOR.getVOC();

            // Store readings for averaging
            previousReadings[readingIndex] = co2;
            readingIndex = (readingIndex + 1) % NUM_READINGS;

            if (!initialized && readingIndex == 0) {
                initialized = true;
                Serial.println("Initialized readings.");
            }

            // Average calculation
            float averageCO2 = 0;
            for (int i = 0; i < NUM_READINGS; i++) {
                averageCO2 += previousReadings[i];
            }
            averageCO2 /= NUM_READINGS;

            // Validate reading
            if (initialized && abs(co2 - averageCO2) > 50) {
                Serial.println("Invalid CO2 reading detected.");
                digitalWrite(ERROR_LED_PIN, HIGH);
            } else {
                digitalWrite(ERROR_LED_PIN, LOW);
            }

            // Send valid data
            co2Characteristic.writeValue(co2);
            vocCharacteristic.writeValue(voc);

            Serial.print("Sent CO2: ");
            Serial.println(co2);
            Serial.print("Sent VOC: ");
            Serial.println(voc);
        }
    } else {
        // Restart advertising if no central is connected
        unsigned long currentMillis = millis();
        if (currentMillis - noCentralPreviousMillis >= noCentralInterval) {
            noCentralPreviousMillis = currentMillis;
            Serial.println("No central connected, restarting advertising...");
            BLE.advertise();
        }
    }
}
