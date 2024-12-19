// entire funcionality of the arduino is coded in this file
#include "Base64.h"
#include <ArduinoBLE.h>
#include <string>
#include <string.h>
#include <AESLib.h>
#include <SHA256.h>
#include "MICS-VZ-89TE.h"

MICS_VZ_89TE CO2SENSOR;

// LED and sensor parameters
#define LED_PIN 3
#define ERROR_LED_PIN 4
#define CO2_THRESHOLD 1000
#define NUM_READINGS 5

// Variables for averaging
float previousReadings[NUM_READINGS];
int readingIndex = 0;
bool initialized = false;
bool waitingforack = false;
int retryCount = 0;

// BLE characteristics
BLEService sensorService("12345678-1234-5678-1234-56789abcdef0");
BLECharacteristic statusCharacteristic("87654321-4321-6789-4321-fedcba987654", BLERead | BLENotify, 64);
BLECharacteristic ackCharacteristic("98765432-4321-6789-4321-fedcba987654", BLEWrite , 64);

// Requirement 6.2.1.3
// TODO - nicht abgeschlossen - DH

// AES Keys
AESLib aes;
byte aes_key[16] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36};
byte aes_iv[16] = {0};
char cleartext[32] = {0};
char ciphertext[128] = {0};
byte enc_iv_to[N_BLOCK] = {0};
byte enc_iv_from[N_BLOCK] = {0};

// HMAC Key
// Requirement 6.2.1.1
byte hmac_key[16] = {0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50};

// Timing
const long interval = 10000; // 10 seconds
const long blinkInterval = 1000; // 1 seconds
long timestamp = 0;
long ledtimestamp = 0;
bool ledswitch = false;

// State management
enum SystemState { IDLE, OPEN, CLOSE };
SystemState currentState = IDLE;

int highCounter = 0;
int lowCounter = 0;

unsigned long noCentralPreviousMillis = 0;
const long noCentralInterval = 1000;

char statusStr[32] = {0};

// Requirement 6.2.1.2
// Requirement 6.2.2.1
// Requirement 6.2.4.1
// Method to encrypt the plaintext and calculate the HMAC
uint16_t encrypt_to_ciphertext_with_hmac(char *msg, byte iv[]) {
    int msgLen = strlen(msg);
    int paddedLen = ((msgLen + 15) / 16) * 16; // Round up to the nearest multiple of 16

    // Add PKCS7 padding
    byte paddedMsg[paddedLen];
    memset(paddedMsg, 0, paddedLen);
    memcpy(paddedMsg, msg, msgLen);
    byte paddingValue = paddedLen - msgLen;
    for (int i = msgLen; i < paddedLen; i++) {
        paddedMsg[i] = paddingValue;
    }

    // Print padded message
    Serial.print("Padded Message (Hex): ");
    for (int i = 0; i < paddedLen; i++) {
        char buffer[4];
        sprintf(buffer, "%02X ", paddedMsg[i]);
        Serial.print(buffer);
    }
    Serial.println();

    // Encrypt the message
    byte encrypted_bytes[paddedLen];
    uint16_t enc_length = aes.encrypt(paddedMsg, paddedLen, encrypted_bytes, aes_key, sizeof(aes_key), iv);

    // Print encrypted message
    Serial.print("Encrypted Message (Hex): ");
    for (int i = 0; i < enc_length; i++) {
        char buffer[4];
        sprintf(buffer, "%02X ", encrypted_bytes[i]);
        Serial.print(buffer);
    }
    Serial.println();

    // Generate HMAC for the encrypted data
    byte hmac[32];
    SHA256 sha256;
    sha256.resetHMAC(hmac_key, sizeof(hmac_key));
    sha256.update(encrypted_bytes, enc_length);
    sha256.finalizeHMAC(hmac_key, sizeof(hmac_key), hmac, sizeof(hmac));

    // Print generated HMAC
    Serial.print("Generated HMAC (Hex): ");
    for (int i = 0; i < sizeof(hmac); i++) {
        char buffer[4];
        sprintf(buffer, "%02X ", hmac[i]);
        Serial.print(buffer);
    }
    Serial.println();

    // Append HMAC to encrypted data
    byte final_message[enc_length + sizeof(hmac)];
    memcpy(final_message, encrypted_bytes, enc_length);
    memcpy(final_message + enc_length, hmac, sizeof(hmac));

    // Base64 encode the final message
    int base64Length = Base64.encodedLength(enc_length + sizeof(hmac));
    char base64Encoded[base64Length + 1];
    Base64.encode(base64Encoded, (char *)final_message, enc_length + sizeof(hmac));
    base64Encoded[base64Length] = '\0';

    // Print Base64 encoded message
    Serial.print("Base64 Encoded Message: ");
    Serial.println(base64Encoded);

    snprintf(ciphertext, sizeof(ciphertext), "%s", base64Encoded);
    return base64Length;
}

// print the current Uptime
void printUptime(){
  unsigned long currentMillis = millis(); // Get the current time in milliseconds

  // Convert millis to hours, minutes, seconds, and milliseconds
  unsigned long totalSeconds = currentMillis / 1000;
  unsigned long milliseconds = currentMillis % 1000;
  unsigned long seconds = totalSeconds % 60;
  unsigned long minutes = (totalSeconds / 60) % 60;
  unsigned long hours = (totalSeconds / 3600);

  // Print the formatted time
  Serial.print(hours);
  Serial.print(":");
  if (minutes < 10) Serial.print("0"); // Add leading zero
  Serial.print(minutes);
  Serial.print(":");
  if (seconds < 10) Serial.print("0"); // Add leading zero
  Serial.print(seconds);
  Serial.print(".");
  if (milliseconds < 100) Serial.print("0"); // Add leading zeros for milliseconds
  if (milliseconds < 10) Serial.print("0");
  Serial.println(milliseconds);
}

// Setup & Initialisation
void setup() {
    // Initialize Serial communication
    Serial.begin(57600);
    while (!Serial) {
        ; // Wait for the Serial connection to initialize
    }

    // Initialize LEDs
    pinMode(LED_PIN, OUTPUT);
    pinMode(ERROR_LED_PIN, OUTPUT);
    digitalWrite(ERROR_LED_PIN, LOW); // Error LED OFF initially

    Serial.println("Starting setup...");

    bool initSuccess = true;
    unsigned long previousMillis = millis();

    // Warm-up period for the CO2 sensor
    Serial.println("Warming up CO2 sensor...");
    unsigned long warmUpStart = millis();
    const unsigned long warmUpTime = 10; // 120 seconds
    while (millis() - warmUpStart < warmUpTime) {
        if ((millis() - warmUpStart) % 10000 == 0) { // Print every 10 seconds
            Serial.print("Warming up: ");
            Serial.print((millis() - warmUpStart) / 1000);
            Serial.println(" seconds elapsed...");
        }
    }
    Serial.println("Warm-up complete. Initializing CO2 sensor...");

    // Retry mechanism for CO2 Sensor
    Serial.println("Initializing CO2 sensor...");
    int retryCount = 0;
    bool sensorInitialized = false;
    while (retryCount < 3) {
        if (millis() - previousMillis >= 1000) { // Wait 1 second using millis()
            previousMillis = millis(); // Reset previousMillis
            retryCount++;
            Serial.print("CO2 Sensor initialization attempt ");
            Serial.println(retryCount);
            sensorInitialized = CO2SENSOR.begin();
            Serial.print("CO2SENSOR.begin() returned: ");
            Serial.println(sensorInitialized);
            if (sensorInitialized) {
                break; // Exit loop if initialization succeeds
            }
        }
    }

    // initialize the Average-Array of measured values
    CO2SENSOR.readSensor();
    float co2 = CO2SENSOR.getCO2();
    previousReadings[0] = co2;
//    delay(1000);
    co2 = CO2SENSOR.getCO2();
    previousReadings[1] = co2;
//    delay(1000);
    co2 = CO2SENSOR.getCO2();
    previousReadings[2] = co2;
//    delay(1000);
    co2 = CO2SENSOR.getCO2();
    previousReadings[3] = co2;
//   delay(1000);
    co2 = CO2SENSOR.getCO2();
    previousReadings[4] = co2;

    if (!sensorInitialized) {
        Serial.println("CO2 Sensor initialization failed! Check wiring and power.");
        initSuccess = false;
    } else {
        Serial.println("CO2 Sensor initialized successfully.");
    }

    // Retry mechanism for BLE Initialization
    Serial.println("Initializing BLE...");
    previousMillis = millis(); // Reset previousMillis
    retryCount = 0;
    bool bleInitialized = false;
    while (retryCount < 3) {
        if (millis() - previousMillis >= 1000) { // Wait 1 second using millis()
            previousMillis = millis(); // Reset previousMillis
            retryCount++;
            Serial.print("BLE initialization attempt ");
            Serial.println(retryCount);
            bleInitialized = BLE.begin();
            if (bleInitialized) {
                break; // Exit loop if initialization succeeds
            }
        }
    }
    if (!bleInitialized) {
        Serial.println("BLE initialization failed! Check hardware and connections.");
        initSuccess = false;
    } else {
        BLE.setLocalName("ArduinoNano33IoT");
        BLE.setAdvertisedService(sensorService);
        sensorService.addCharacteristic(statusCharacteristic);
        sensorService.addCharacteristic(ackCharacteristic);
        BLE.addService(sensorService);
        BLE.advertise();
        Serial.println("BLE advertising and sensor started...");
    }

    // Final Initialization Check
    
    if (initSuccess) {
        // Requirement 6.1.1.1
        digitalWrite(ERROR_LED_PIN, HIGH); // Turn ON Error LED for successful initialization
        Serial.println("Initialization successful. Error LED ON.");
    } else {
        // Requirement 6.1.1.0
        Serial.println("Initialization failed. Error LED remains ON.");
        digitalWrite(ERROR_LED_PIN, LOW); // Turn OFF Error LED for unsuccessful initialization
    }
}

// Loop for running-state
void loop() {

    BLEDevice central = BLE.central();

    // Requirement 6.3.1 / 6.3.2 - vica versa
    // Arduino waiting for connection
    if (central) {
        // Requirement 6.3.5
        // Update/Send the Characteristic every 10 Seconds
        if (millis() - timestamp > interval) {
            Serial.print("Connected to central: ");
            Serial.println(central.address());

            // Read sensor values
            CO2SENSOR.readSensor();
            float co2 = CO2SENSOR.getCO2();

            // Requirement 6.1.2.1
            // Requirement 6.1.2.2
            // Calculate the average of the last NUM_READINGS
            float averageCO2 = 0;
            Serial.print("Letzte Werte: ");
            for (int i = 0; i < NUM_READINGS; i++) {
                averageCO2 += previousReadings[i];
                Serial.print(previousReadings[i]);
                Serial.print(", ");
            }
            averageCO2 /= NUM_READINGS;
            Serial.println("");

            // check for valid meassurements
            if (abs(co2 - averageCO2) > 50) {
                Serial.print("CO2 reading marked as invalid and dropped: ");
                Serial.println(co2);
            }else if (!waitingforack){
                // Store the current reading in the previousReadings array
                previousReadings[readingIndex] = co2;
                readingIndex = (readingIndex + 1) % NUM_READINGS;
            }

            // Print CO2 ppm value to serial monitor
            Serial.print("CO2 ppm: ");
            Serial.println(co2);

            // Determine system state dynamically based on CO2 level-averages and thresholds
            if (averageCO2 > CO2_THRESHOLD) {
                currentState = OPEN;
                Serial.println("System State: OPEN");
            } else if (averageCO2 <= CO2_THRESHOLD - 2) {
                currentState = CLOSE;
                Serial.println("System State: CLOSE");
            } else {
                currentState = IDLE;
                Serial.println("System State: IDLE");
            }

            // Encrypt and send the state message
            // Requirement 6.2.4.2 - default strings are "open" & "close"
            snprintf(statusStr, sizeof(statusStr), "State: %s", currentState == OPEN ? "OPEN" : currentState == CLOSE ? "CLOSE" : "IDLE");
            memcpy(enc_iv_to, aes_iv, sizeof(aes_iv));
            encrypt_to_ciphertext_with_hmac(statusStr, enc_iv_to);

            // Requirement 6.2.3.1
            Serial.print("Uptime:");
            printUptime();
            Serial.print("Sent Encrypted State with HMAC: ");
            Serial.println(ciphertext);            

            
            // Requirement 6.1.3.1
            // Requirement 6.1.3.2
            // Requirement 6.1.3.3
            // Requirement 6.1.3.4
            // Requirement 6.2.5.2
            // Requirement 6.2.5.3
            // Acknowledgement Handling
            waitingforack = true;
            if(ackCharacteristic.written()){
              waitingforack = false;
              ackCharacteristic.writeValue("");
              Serial.println("Ack recieved");
              retryCount=0;
            }else{
              Serial.println("Still Waiting for Ack");
              retryCount+=1;
            }

            // Requirement 6.3.7
            statusCharacteristic.writeValue((byte *)ciphertext, strlen(ciphertext));

            timestamp = millis();

        // blinking LED if no Acks recieved
        } else if (retryCount>2){
          if (millis() - ledtimestamp > blinkInterval) {
            if (ledswitch) {
              digitalWrite(ERROR_LED_PIN, LOW);
              ledswitch=false;
            }else{
              digitalWrite(ERROR_LED_PIN, HIGH);
              ledswitch=true;
            }
            ledtimestamp = millis();
          }
        }

    } else {
    unsigned long currentMillis = millis();
    if (currentMillis - noCentralPreviousMillis >= noCentralInterval) {
        noCentralPreviousMillis = currentMillis;
        BLE.advertise();
    }
}

}
