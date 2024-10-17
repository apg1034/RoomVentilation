#include "MICS-VZ-89TE.h"

MICS_VZ_89TE CO2SENSOR;

#define LED_PIN 3 // Pin for the status LED
#define CO2_THRESHOLD 450 // CO2 level threshold in ppm

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Initialize LED pin
  pinMode(LED_PIN, OUTPUT);

  CO2SENSOR.begin();

  Serial.println("MICS-VZ-89TE Implementation Step");
}

void loop() { // run over and over
  CO2SENSOR.readSensor();

  float co2 = CO2SENSOR.getCO2();
  float voc = CO2SENSOR.getVOC();

  Serial.print("CO2: ");
  Serial.println(co2, 3);

  Serial.print("VOC: ");
  Serial.println(voc, 3);

  // Check CO2 level and blink LED if above threshold
  if (co2 > CO2_THRESHOLD) {
    digitalWrite(LED_PIN, HIGH); // Turn on LED
    delay(500);                  // Keep it on for 500 milliseconds
    digitalWrite(LED_PIN, LOW);  // Turn off LED
    delay(500);                  // Keep it off for 500 milliseconds
  } else {
    digitalWrite(LED_PIN, LOW);  // Ensure LED is off if CO2 is below threshold
  }

  delay(1000); // Wait for 1 second before the next reading
}
