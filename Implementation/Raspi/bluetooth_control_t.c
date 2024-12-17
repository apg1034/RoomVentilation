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

#define SENSOR_ID "87654321-4321-6789-4321-fedcba987654"
#define MAC_ADDR "4C:11:AE:C9:80:12"
#define BLE_SCAN_TIMEOUT 10
#define ERROR_WAIT_TIME 10
#define READ_WAIT_TIME 1
#define HMAC_LENGTH 32 // Length of the HMAC-SHA256 digest

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

void printUnsignedCharArray(const unsigned char *array, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        printf("%02X ", array[i]);
        if ((i + 1) % 16 == 0) { // Add a newline every 16 bytes for readability
            printf("\n");
        }
    }
    printf("\n"); // Ensure a newline at the end
    fflush(stdout); // Flush the output to ensure it appears immediately
}

void decrypt_and_verify(const unsigned char *encrypted_data, int data_length, unsigned char *decrypted_data) {
    printf("Step 1: Received Encrypted Data:\n");
    printUnsignedCharArray(encrypted_data, data_length);

    decrypt_ciphertext(encrypted_data, data_length, decrypted_data);

    printf("Step 2: Decryption Completed. Decrypted Data (String): %s\n", decrypted_data);
    fflush(stdout);
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
        decrypt_and_verify(buffer, len, decrypted_data);

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

        if (kbhit()) {
            print_with_timestamp("Exit.\n");
            break;
        }

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
    while (1) {
        if (gattlib_mainloop(ble_task, NULL) != GATTLIB_SUCCESS) {
            print_with_timestamp("Failed to create gattlib mainloop\n");
            return true;
        }

        if (state == READERROR) {
            retryCount++;
            if (retryCount > 2) {
                turnOffLED();
                return true;
            }
        } else {
            break;
        }

        sleep(ERROR_WAIT_TIME);
    }

    return false;
}
