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
#include "mail_control.h"
#include "crypto_control.h"


#define SENSOR_ID "87654321-4321-6789-4321-fedcba987654"
#define MAC_ADDR "4C:11:AE:C9:80:12"
#define BLE_SCAN_TIMEOUT   10
#define ERROR_WAIT_TIME 10
#define READ_WAIT_TIME 1

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

enum { UNINITIALIZED, FOUND, CONNECTED, READERROR, DISCONNECT} state = UNINITIALIZED;

// Global Retry Count

int retryCount = 0;

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

void decrypt_ciphertext(unsigned char *encrypted_data, int data_length, unsigned char *decrypted_data) {
    unsigned char key[16] = {
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
        0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36
    };

    unsigned char iv[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    memset(decrypted_data, 0, data_length);

    aes_decrypt(encrypted_data, data_length, decrypted_data, key, iv);

}


void printUnsignedCharArray(const unsigned char* array, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        printf("%c", array[i]);
    }
    printf("\n");
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
				print_with_timestamp("Could not find GATT Characteristic with UUID %s. "
					"You might call the program with '--gatt-discovery'.\n", uuid_str);
			} else {
				print_with_timestamp("Error while reading GATT Characteristic with UUID %s (ret:%d)\n", uuid_str, ret);
			}
			
			state = READERROR;
			break;			
		}
		state = CONNECTED;
		retryCount = 0;
		turnOnLED();
		
		print_with_timestamp("Read UUID completed: ");
		
//		for (uintptr_t i = 0; i < len; i++) {
//			printf("%c ", buffer[i]);
//		}
		
		//value = bytes_to_int(buffer, 4);
                int data_length = sizeof(buffer);

		unsigned char encrypted_data[data_length];
		memcpy(encrypted_data, buffer, 5*sizeof(uint8_t));

		unsigned char decrypted_data[] = {data_length};
		decrypt_ciphertext(encrypted_data, data_length, decrypted_data);

		int decry_data_len = sizeof(decrypted_data);
		printUnsignedCharArray(decrypted_data, decry_data_len);

		//printf(" (%d)\n", value);

		// Action for OPEN, CLOSE or IDLE
		
		setAction(value);

		// Exit if any key is hit
		
		if (kbhit())
		{
			print_with_timestamp("Exit\n");
			break;
		}
		
		// Wait
		
		sleep(READ_WAIT_TIME);
	}

	// EXIT
	
	gattlib_characteristic_free_value(buffer);

	print_with_timestamp("Wait for disconnect.\n");

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

	state = FOUND;

	print_with_timestamp("Found bluetooth device '%s'\n", m_argument.mac_address);

	ret = gattlib_adapter_scan_disable(adapter);
	if (ret) {
		print_with_timestamp("Scan disabled (Error: %x).\n",ret);
	}
	
	ret = gattlib_connect(adapter, addr, GATTLIB_CONNECTION_OPTIONS_NONE, on_device_connect, NULL);
	if (ret != GATTLIB_SUCCESS) {
		print_with_timestamp("Failed to connect to the bluetooth device '%s'\n", addr);
	}
}

static void* ble_task(void* arg) {
	char* addr = arg;
	gattlib_adapter_t* adapter;
	int ret;
	int closeRetry = 0;

    print_with_timestamp("Open BLE Adapter.\n");
	
	ret = gattlib_adapter_open(m_argument.adapter_name, &adapter);
	
	if (ret) {
		print_with_timestamp("Failed to open adapter.\n");
		return NULL;
	}

    print_with_timestamp("Scanning devices.\n");
	
	ret = gattlib_adapter_scan_enable(adapter, ble_discovered_device, BLE_SCAN_TIMEOUT, addr);
	if (ret) {
		print_with_timestamp("Failed to scan (Error: %x).\n",ret);
		return NULL;
	}
	
	if (FOUND == state)
	{
		print_with_timestamp("Wait for connection.\n");

		// Wait for the device to be connected
		pthread_mutex_lock(&m_connection_terminated_lock);
		pthread_cond_wait(&m_connection_terminated, &m_connection_terminated_lock);
		pthread_mutex_unlock(&m_connection_terminated_lock);

		print_with_timestamp("Connection terminated.\n");

		int closeRetry = 0;
	}
	else
	{
		print_with_timestamp("No device found.\n");
	
		ret = gattlib_adapter_scan_disable(adapter);
		if (ret) {
			print_with_timestamp("Scan disabled (Error: %x).\n",ret);
		}	
	}
		
	while(1)
	{
		ret = gattlib_adapter_close(adapter);
		
		if(ret == GATTLIB_BUSY && closeRetry == 0)
		{
			print_with_timestamp("Gattlib busy, waiting 10 sek\n",ret);
			sleep(10);
			closeRetry++;
		}
		else
		{
			if (ret) {
				print_with_timestamp("Adapter close (Error: %x).\n",ret);
				return NULL;
			}
		}
	}
	return NULL;
}

bool initializeBluetooth()
{
	// Initialize structure
	
	m_argument.adapter_name = NULL;
	m_argument.mac_address = MAC_ADDR;
	
	// Create uuid 
	
	if (gattlib_string_to_uuid(SENSOR_ID, strlen(SENSOR_ID) + 1, &m_argument.uuid) < 0) {
		print_with_timestamp ("Invalid uuid\n");
		return false;
	}
	
	return true;
}

bool runBluetooth()
{
	int ret;
		
	while(1)
	{		
		ret = gattlib_mainloop(ble_task, NULL);
		
		if (ret != GATTLIB_SUCCESS) {
			print_with_timestamp ("Failed to create gattlib mainloop\n");
			return true;
		}
		
		if(state == READERROR)
		{
			retryCount++;
		
			print_with_timestamp ("Reading failed: retryCount = %i\n", retryCount);
		
			if(retryCount > 1)
			{
				// LED blinken
			
				turnOffLED();
				print_with_timestamp ("Signal LED\n");			
				
				if(retryCount > 2)
				{
					// Send mail and exit
					print_with_timestamp ("Signal MAIL\n");
					
					// Please comment out if all tests are done
					// sendMail();
					
					return true;
				}
			}
		}
		else
		{
			// Exit normal
			break;
		}
		sleep(ERROR_WAIT_TIME);
	}
				
	return false;
}
