/***************************************************************************
 
 Hervé Grabas
 
 This is a library for the MICS_VZ_89TE Indoor Air Quality Sensor.
 
 No warranty is given
 
***************************************************************************/

#include "MICS-VZ-89TE.h"

#include <math.h>
#include <Wire.h>

MICS_VZ_89TE::MICS_VZ_89TE(void) {
    _i2caddr = MICS_VZ_89TE_ADDR;
    co2 = 0.0;
    voc = 0.0;
}

float MICS_VZ_89TE::getCO2(void) {
    return co2;
}

float MICS_VZ_89TE::getVOC(void) {
    return voc;
}

float MICS_VZ_89TE::getYear(void) {
    return year;
}

float MICS_VZ_89TE::getMonth(void) {
    return month;
}

float MICS_VZ_89TE::getDay(void) {
    return day;
}

float MICS_VZ_89TE::getRev(void) {
    return rev;
}

float MICS_VZ_89TE::getCrc(void) {
    return crc;
}

float MICS_VZ_89TE::getStatus(void) {
    return status;
}

void MICS_VZ_89TE::readSensor(void) {
    static uint8_t data[7];
    readData(MICS_VZ_89TE_ADDR_CMD_GETSTATUS, data);

    // Print raw sensor data
    Serial.print("Raw Sensor Data: ");
    for (int i = 0; i < 7; i++) {
        Serial.print(data[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    status = data[5];

    if (data[1] >= 13 && data[1] <= 242) {
        co2 = (data[1] - 13) * (1600.0 / 229) + 400; // Valid CO2 range
    } else {
        Serial.println("Error: Invalid CO2 value received.");
        co2 = 400; // Default fallback
    }

    if (data[0] >= 13 && data[0] <= 242) {
        voc = (data[0] - 13) * (1000.0 / 229); // Valid VOC range
    } else {
        Serial.println("Error: Invalid VOC value received.");
        voc = 0; // Default fallback
    }
}


void MICS_VZ_89TE::getVersion(void) {
    static uint8_t data[7];
    readData(MICS_VZ_89TE_DATE_CODE, data);
    
    year = data[0];
    month = data[1];
    day = data[2];
    rev = data[3];
    crc = data[6];
}


bool MICS_VZ_89TE::begin() {
    Wire.begin();
    Serial.println("Waiting for sensor power-on self-test...");

    unsigned long startMillis = millis();
    const unsigned long selfTestDuration = 2000; // 2 seconds

    while (millis() - startMillis < selfTestDuration) {
        // Do nothing, wait for sensor to warm up
    }

    // Check I2C communication
    Serial.println("Checking I2C communication...");
    Wire.beginTransmission(_i2caddr);
    uint8_t error = Wire.endTransmission();

    if (error == 0) {
        Serial.println("Sensor at 0x70 is responding.");

        // Read status byte to confirm sensor state
        uint8_t testData[7];
        readData(MICS_VZ_89TE_ADDR_CMD_GETSTATUS, testData);
        if (testData[1] == 0x00 || testData[1] == 0xFF) {
            Serial.println("Sensor returned invalid data during initialization.");
            return false;
        }

        return true;
    } else {
        Serial.print("I2C communication failed. Error code: ");
        Serial.println(error);
        return false;
    }
}





/*
 After Power-on self-test (2 seconds) , the device will provide either a single “Failed Diagnostic
 Level” in case of sensor failure of the sensor or PWM multiplexed output indicating
 “CO_ equivalent_ Level” and “VOC _isobutylene_equivalent Level” referred to the isobutylene
 sensitivity unit.
 
 Out of this initial period, the device will have the I2C data CO2 equivalent [ppm] and tVOC equivalent
 referred to the isobutylene sensitivity unit [ppb].
 
 D1 : Data_byte_1: VOC_short [13…242]
 D2 : Data_byte_2: CO2_equ: [13…242] -> CO2_equ [ppm] = (D2 -13) * (1600/229) + 400
 D3 : Data_byte_3: Raw sensor first byte (MSB)
 D4 : Data_byte_4: Raw sensor second byte
 D5 : Data_byte_5: Raw sensor third byte (MSB) -> Resistor value [W] = 10*(D5+ (256*D4) + (65536*D3))
 D6 : Status
 D7 : CRC
 
 return CO2 equivalent [ppm] and tVOC equivalent referred to the isobutylene sensitivity unit [ppb].
 *
 */
void MICS_VZ_89TE::readData(byte reg, uint8_t data[]) {
    uint8_t i =0;
    Wire.beginTransmission((uint8_t)_i2caddr);
    Wire.write((uint8_t)reg); //This send the command to get data
    Wire.write(0x0);
    Wire.write(0x0);
    Wire.write(0x0);
    Wire.write(0x0);
    Wire.write(0x0);
    Wire.endTransmission();
    
    Wire.beginTransmission(0xE1);
    Wire.endTransmission();
    
    Wire.requestFrom((uint8_t)_i2caddr, (byte)7);
    for (i=0; i<7; i++) {
    data[i] = Wire.read();
    }

}


