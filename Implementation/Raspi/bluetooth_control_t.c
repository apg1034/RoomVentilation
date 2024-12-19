#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <pthread.h>
#include <termio.h>
#include "gattlib.h"
#include "common.h"
#include "led_control.h"
#include "action_control.h"
#include "crypto_control.h"
#include "mail_control.h"

// Requirment 6.2.1.4
#define SENSOR_ID "87654321-4321-6789-4321-fedcba987654"
#define MAC_ADDR "4C:11:AE:C9:80:12"
#define BLE_SCAN_TIMEOUT 10
#define ERROR_WAIT_TIME 10
#define READ_WAIT_TIME 5
#define HMAC_LENGTH 32 // Length of the HMAC-SHA256 digest

extern unsigned long millis();

static struct {
    char *adapter_name;
    char *mac_address;
    uuid_t uuid;
    long int value_data;
} m_argument;

static pthread_cond_t m_connection_terminated = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t m_connection_terminated_lock = PTHREAD_MUTEX_INITIALIZER;

enum { UNINITIALIZED, FOUND, CONNECTED, READERROR, DISCONNECT } state = UNINITIALIZED;
int retryCount = 0;

bool kbhit(void) {
    struct termios original;
    tcgetattr(STDIN_FILENO, &original);

    struct termios term;
    memcpy(&term, &original, sizeof(term));
    term.c_lflag &= ~ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    int characters_buffered = 0;
    ioctl(STDIN_FILENO, FIONREAD, &characters_buffered);

    tcsetattr(STDIN_FILENO, TCSANOW, &original);

    return (characters_buffered != 0);
}


void decrypt_and_verify(const unsigned char *encrypted_data, int data_length, unsigned char *decrypted_data) {
    // Requirement 6.2.3.1
    print_with_timestamp("Step 1: Received Encrypted Data:\n");
    printUnsignedCharArray(encrypted_data, data_length);

    decrypt_ciphertext(encrypted_data, data_length, decrypted_data);

    printf("Step 2: Decryption Completed. Decrypted Data (String): %s\n", decrypted_data);
    fflush(stdout);
}

// Funktion zum Schreiben in eine BLE-Charakteristik
int write_ble_characteristic(gattlib_connection_t *connection, const char *uuid_str, unsigned char *value, size_t value_len) {
    uuid_t uuid;
    if (gattlib_string_to_uuid(uuid_str, strlen(uuid_str) + 1, &uuid) < 0) {
        printf("Invalid UUID string: %s\n", uuid_str);
        return -1;
    }

    // Schreiben der Charakteristik
    int ret = gattlib_write_char_by_uuid(connection, &uuid, value, value_len);
    if (ret != GATTLIB_SUCCESS) {
        printf("Error %s\n", uuid_str);
        return -1;
    }
    printf("Success %s\n", uuid_str);
    return 0;
}

static void on_device_connect(gattlib_adapter_t *adapter, const char *dst, gattlib_connection_t *connection, int error, void *user_data) {
    int ret;
    size_t len;
    uint8_t *buffer = NULL;

    while (1) {
        // Read UUID data
        ret = gattlib_read_char_by_uuid(connection, &m_argument.uuid, (void **)&buffer, &len);
        if (ret != GATTLIB_SUCCESS) {
            char uuid_str[MAX_LEN_UUID_STR + 1];
            gattlib_uuid_to_string(&m_argument.uuid, uuid_str, sizeof(uuid_str));

            if (ret == GATTLIB_NOT_FOUND) {
                print_with_timestamp("Could not find GATT Characteristic with UUID %s.\n", uuid_str);
            } else {
                print_with_timestamp("Error while reading GATT Characteristic with UUID %s (ret:%d)\n", uuid_str, ret);
            }

            state = READERROR;
            break;
        }

        state = CONNECTED;
        retryCount = 0;
        turnOnLED();

        print_with_timestamp("Step 0: Read UUID completed:\n");
        printf("Raw Data:\n");
        printUnsignedCharArray(buffer, len);

        unsigned char decrypted_data[256] = {0}; // Ensure enough space for decrypted output
	// Requirement 6.2.4.1
        decrypt_and_verify(buffer, len, decrypted_data);

	// Requirment 6.3.6
	// Requirment 6.3.8
	// Requirment 6.3.9
        if (strstr((char *)decrypted_data, "OPEN")) {
            print_with_timestamp("Action: OPEN detected.\n");
            actionOpen();
        } else if (strstr((char *)decrypted_data, "CLOSE")) {
            print_with_timestamp("Action: CLOSE detected.\n");
            actionClose();
        } else if (strstr((char *)decrypted_data, "IDLE")) {
            print_with_timestamp("Action: IDLE detected. No action taken.\n");
        } else {
            print_with_timestamp("Unknown action detected: %s\n", decrypted_data);
        }

	// Requirement 6.1.3.1
	// TODO - Ack Handling
        const char *ack_message = "ack";  // String "ack"
        
        // Umwandlung von const char* zu unsigned char*:
        unsigned char ack_message_bytes[strlen(ack_message) + 1];  // +1 für das Nullterminierungszeichen
        memcpy(ack_message_bytes, ack_message, strlen(ack_message) + 1);  // Kopiere die Bytes in ein unsigned char Array

        // Schreiben von "ack" in die Charakteristik
        if (write_ble_characteristic(connection, "98765432-4321-6789-4321-fedcba987654", ack_message_bytes, strlen(ack_message)) != 0) {
            printf("Error writing 'ack' to characteristic.\n");
        }


        if (kbhit()) {
            print_with_timestamp("Exit.\n");
            break;
        }

	// Requirement 6.2.5.1 - for testing set to 1 second instead of 10 seconds
        sleep(READ_WAIT_TIME);
    }

    gattlib_characteristic_free_value(buffer);
    gattlib_disconnect(connection, false);

    pthread_mutex_lock(&m_connection_terminated_lock);
    pthread_cond_signal(&m_connection_terminated);
    pthread_mutex_unlock(&m_connection_terminated_lock);
}

static int stricmp(char const *a, char const *b) {
    for (;; a++, b++) {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0 || !*a)
            return d;
    }
}

static void ble_discovered_device(gattlib_adapter_t *adapter, const char *addr, const char *name, void *user_data) {
    if (stricmp(addr, m_argument.mac_address) != 0) {
        return;
    }

    state = FOUND;
    print_with_timestamp("Found bluetooth device '%s'\n", m_argument.mac_address);

    if (gattlib_adapter_scan_disable(adapter)) {
        print_with_timestamp("Scan disable failed.\n");
    }

    // Requirement 6.3.1 / 6.3.2 - vica versa
    if (gattlib_connect(adapter, addr, GATTLIB_CONNECTION_OPTIONS_NONE, on_device_connect, NULL)) {
        print_with_timestamp("Failed to connect to bluetooth device '%s'\n", addr);
    }
}

static void *ble_task(void *arg) {
    gattlib_adapter_t *adapter;

    print_with_timestamp("Open BLE Adapter.\n");
    if (gattlib_adapter_open(m_argument.adapter_name, &adapter)) {
        print_with_timestamp("Failed to open adapter.\n");
        return NULL;
    }

    print_with_timestamp("Scanning devices.\n");
    if (gattlib_adapter_scan_enable(adapter, ble_discovered_device, BLE_SCAN_TIMEOUT, arg)) {
        print_with_timestamp("Scan failed.\n");
        return NULL;
    }

    if (FOUND == state) {
        pthread_mutex_lock(&m_connection_terminated_lock);
        pthread_cond_wait(&m_connection_terminated, &m_connection_terminated_lock);
        pthread_mutex_unlock(&m_connection_terminated_lock);
    }

    gattlib_adapter_close(adapter);
    return NULL;
}

bool initializeBluetooth() {
    m_argument.adapter_name = NULL;
    m_argument.mac_address = MAC_ADDR;

    if (gattlib_string_to_uuid(SENSOR_ID, strlen(SENSOR_ID) + 1, &m_argument.uuid) < 0) {
        print_with_timestamp("Invalid UUID\n");
        return false;
    }

    return true;
}

bool runBluetooth() {
    unsigned long last_retry_time = millis(); // Track the last retry time
    // Requirement 6.1.1.5
    const unsigned long retry_interval = ERROR_WAIT_TIME * 10000; // Retry interval in milliseconds - for testing set to 1 second instead on 10 seconds

    while (1) {
        if (gattlib_mainloop(ble_task, NULL) != GATTLIB_SUCCESS) {
            print_with_timestamp("Failed to create gattlib mainloop\n");
            //return true;
        }

        if (state == READERROR) {
            // Requirement 6.3.3
            // Requirment 6.3.4
            for (int i = 0; i < 5; i++) {
                turnOffLED();
                sleep(1);
                turnOnLED();
                sleep(1);
            }

            retryCount++;
            last_retry_time = millis(); // Update retry time

            print_with_timestamp("Reading failed: retryCount = %i\n", retryCount);
            if (retryCount > 2) {
                print_with_timestamp("Retry limit exceeded. Turning off LED, sending Mail & exiting...\n");
                turnOffLED();
                // Requirement 6.1.1.4
                sendMail();
                return true;
            }
        } else {
            retryCount = 0; // Reset retry count if connection is successful
            break;
        }
    }

    return false;
}
