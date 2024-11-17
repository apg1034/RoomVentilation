#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <pthread.h>
#include <termio.h>

#include "gattlib.h"

#define SENSOR_ID "87654321-4321-6789-4321-fedcba987654"
#define MAC_ADDR "4C:11:AE:C9:80:12"
#define BLE_SCAN_TIMEOUT   10

static struct {
	char *adapter_name;
	char* mac_address;
	uuid_t uuid;
	long int value_data;
} m_argument;

// Declaration of thread condition variable
static pthread_cond_t m_connection_terminated = PTHREAD_COND_INITIALIZER;

// declaring mutex
static pthread_mutex_t m_connection_terminated_lock = PTHREAD_MUTEX_INITIALIZER;

// state of program
// enum { UNINITIALIZED, INIT, CONNECTED, DISCONNECT} state = UNINITIALIZED;

bool kbhit(void)
{
    struct termios original;
    tcgetattr(STDIN_FILENO, &original);

    struct termios term;
    memcpy(&term, &original, sizeof(term));

    term.c_lflag &= ~ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    int characters_buffered = 0;
    ioctl(STDIN_FILENO, FIONREAD, &characters_buffered);

    tcsetattr(STDIN_FILENO, TCSANOW, &original);

    bool pressed = (characters_buffered != 0);

    return pressed;
}
int bytes_to_int(unsigned char *bytes, size_t length) {
    int result = 0;
    for (size_t i = 0; i < length; i++) {
        result |= (bytes[i] << (i * 8));
    }
    return result;
}

static void on_device_connect(gattlib_adapter_t* adapter, const char *dst, gattlib_connection_t* connection, int error, void* user_data) {
	int ret;
	size_t len;

	uint8_t *buffer = NULL;
	int value = 0;

	// Read loop for values
	
	while (1)
	{
		// Read values
		
		ret = gattlib_read_char_by_uuid(connection, &m_argument.uuid, (void **)&buffer, &len);
		if (ret != GATTLIB_SUCCESS) {
			char uuid_str[MAX_LEN_UUID_STR + 1];

			gattlib_uuid_to_string(&m_argument.uuid, uuid_str, sizeof(uuid_str));

			if (ret == GATTLIB_NOT_FOUND) {
				printf("Could not find GATT Characteristic with UUID %s. "
					"You might call the program with '--gatt-discovery'.", uuid_str);
			} else {
				printf("Error while reading GATT Characteristic with UUID %s (ret:%d)", uuid_str, ret);
			}
			break;			
		}

		printf("Read UUID completed: ");
		
		for (uintptr_t i = 0; i < len; i++) {
			printf("%02x ", buffer[i]);
		}
		
		value = bytes_to_int(buffer, 4);
		
		printf(" (%d)\n", value);

		// Exit if any key is hit
		
		if (kbhit())
		{
			printf("Exit\n");
			break;
		}
		
		// Wait
		
		sleep(1);
	}

	// EXIT
	
	gattlib_characteristic_free_value(buffer);

	printf("Wait for disconnect.\n");

	gattlib_disconnect(connection, false /* wait_disconnection */);

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

static void ble_discovered_device(gattlib_adapter_t* adapter, const char* addr, const char* name, void *user_data) {
	int ret;

	if (stricmp(addr, m_argument.mac_address) != 0) {
		return;
	}

	printf("Found bluetooth device '%s'\n", m_argument.mac_address);

	ret = gattlib_connect(adapter, addr, GATTLIB_CONNECTION_OPTIONS_NONE, on_device_connect, NULL);
	if (ret != GATTLIB_SUCCESS) {
		printf("Failed to connect to the bluetooth device '%s'\n", addr);
	}
}

static void* ble_task(void* arg) {
	char* addr = arg;
	gattlib_adapter_t* adapter;
	int ret;

    printf("Open BLE Adapter.\n");
	
	ret = gattlib_adapter_open(m_argument.adapter_name, &adapter);
	
	if (ret) {
		printf("Failed to open adapter.\n");
		return NULL;
	}

    printf("Scanning devices.\n");
	
	ret = gattlib_adapter_scan_enable(adapter, ble_discovered_device, BLE_SCAN_TIMEOUT, addr);
	if (ret) {
		printf("Failed to scan.\n");
		return NULL;
	}

	printf("Wait for connection.\n");

	// Wait for the device to be connected
	pthread_mutex_lock(&m_connection_terminated_lock);
	pthread_cond_wait(&m_connection_terminated, &m_connection_terminated_lock);
	pthread_mutex_unlock(&m_connection_terminated_lock);

	printf("Connection terminated.\n");

	return NULL;
}

bool initializeBluetooth()
{
	// Initialize structure
	
	m_argument.adapter_name = NULL;
	m_argument.mac_address = MAC_ADDR;
	
	// Create uuid 
	
	if (gattlib_string_to_uuid(SENSOR_ID, strlen(SENSOR_ID) + 1, &m_argument.uuid) < 0) {
		printf ("Invalid uuid\n");
		return false;
	}
	
	return true;
}

bool runBluetooth()
{
	int ret;
	
	ret = gattlib_mainloop(ble_task, NULL);
	
	if (ret != GATTLIB_SUCCESS) {
		printf("Failed to create gattlib mainloop");
		return false;
	}
				
	return true;
}