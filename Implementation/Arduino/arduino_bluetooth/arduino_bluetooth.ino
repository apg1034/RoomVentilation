#include <ArduinoBLE.h>
#include <string>
#include <string.h>
#include <AESLib.h>
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
BLECharacteristic statusCharacteristic("87654321-4321-6789-4321-fedcba987654", BLERead | BLENotify,32);

//AES Keys
AESLib aes;
byte aes_key[16] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36};
byte aes_iv[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
char cleartext[32] = {0};
char ciphertext[64] = {0};
byte enc_iv_to[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
byte enc_iv_from[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// Timing
const long interval = 10000; // 10 seconds
long timestamp = 0;

unsigned long noCentralPreviousMillis = 0;
const long noCentralInterval = 1000; // 1 second for BLE advertising

char status[32] = {0};

uint16_t encrypt_to_ciphertext(char * msg, byte iv[]) {
  int msgLen = strlen(msg);
  int cipherlength = aes.get_cipher64_length(msgLen);
  char encrypted_bytes[cipherlength];
  uint16_t enc_length = aes.encrypt64((byte*)msg, msgLen, encrypted_bytes, aes_key, sizeof(aes_key), iv);
  sprintf(ciphertext, "%s", encrypted_bytes);
  return enc_length;
}

void decrypt_to_cleartext(char * msg, uint16_t msgLen, byte iv[]) {
  Serial.println("Calling decrypt64...");
#ifdef ESP8266
  Serial.print("[decrypt_to_cleartext] free heap: "); Serial.println(ESP.getFreeHeap());
#endif
  uint16_t decLen = aes.decrypt64(msg, msgLen, (byte*)cleartext, aes_key, sizeof(aes_key), iv);
  Serial.print("Decrypted bytes: "); Serial.println(decLen);
}


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
    sensorService.addCharacteristic(statusCharacteristic);
    BLE.addService(sensorService);

    BLE.advertise();
    Serial.println("BLE advertising and sensor started...");

    /*
    memcpy(enc_iv_to, aes_iv, sizeof(aes_iv));
    memcpy(enc_iv_from, aes_iv, sizeof(aes_iv));
    strncpy(cleartext, "Hallo, Welt!", sizeof(cleartext) - 1);
    Serial.println(cleartext);
    uint16_t len = encrypt_to_ciphertext(cleartext, enc_iv_to);
    Serial.println(ciphertext);
    Serial.println("Encrypted. Decrypting..."); Serial.flush();
    decrypt_to_cleartext(ciphertext, len, enc_iv_from);
    Serial.println(cleartext);
    */

}

void loop() {
    BLEDevice central = BLE.central();

    if (central) {
        // Read and send data every interval
        if (millis() - timestamp > interval) {

            Serial.print("Connected to central: ");
            Serial.println(central.address());

            // Read sensor values
            CO2SENSOR.readSensor();
            int co2 = CO2SENSOR.getCO2();
            int voc = CO2SENSOR.getVOC();

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

            strncpy(status, "1", sizeof(status) - 1);

            memcpy(enc_iv_to, aes_iv, sizeof(aes_iv));
            uint16_t len = encrypt_to_ciphertext(status, enc_iv_to);

            Serial.print("Sent Status: ");
            Serial.println(ciphertext);

            // Send valid data
            statusCharacteristic.writeValue((byte*)ciphertext,strlen(ciphertext));

            Serial.print("Sent Status: ");
            Serial.println(ciphertext);

            timestamp = millis();
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