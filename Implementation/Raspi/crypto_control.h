#ifndef AES_CRYPT_H
#define AES_CRYPT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/aes.h>
<<<<<<< HEAD
#include <openssl/sha.h>

=======
>>>>>>> adea0023f5c375dfcd32ecb78233ae5518a6c95d

// Funktionsprototypen

/**
 * @brief Funktion zum Base64-Dekodieren eines Base64-kodierten Strings
 * @param base64_input Der Base64-kodierte Eingabestring
 * @param decoded_output Der Puffer, der die dekodierten Rohdaten enthalten wird
 * @return Die Länge der dekodierten Daten
 */
int base64_decode(const char *base64_input, unsigned char **decoded_output);

/**
 * @brief Funktion zur AES-Verschlüsselung im CBC-Modus mit 128-Bit-Schlüssel
 * @param plaintext Die Klartext-Daten (zu verschlüsselnde Daten)
 * @param plaintext_length Die Länge der Klartext-Daten
 * @param encrypted_data Der Zielpuffer, in dem die verschlüsselten Daten gespeichert werden
 * @param key Der 16-Byte-Schlüssel für AES
 * @param iv Der 16-Byte-Initialisierungsvektor für AES-CBC
 */
void aes_encrypt(
    const unsigned char *plaintext,    // Klartext (zu verschlüsselnde Daten)
    int plaintext_length,              // Länge des Klartexts
    unsigned char *encrypted_data,     // Zielpuffer für verschlüsselte Daten
    const unsigned char *key,          // 16-Byte-Schlüssel für AES
    const unsigned char *iv            // 16-Byte-Initialisierungsvektor für AES-CBC
);

/**
<<<<<<< HEAD
 * @brief Calculates the HMAC of a given input using a provided key.
 * @param data The input data for which HMAC is to be calculated.
 * @param data_length The length of the input data.
 * @param output_buffer The buffer to store the computed HMAC.
 * @param key The HMAC key.
 * @param key_length The length of the HMAC key.
 */
void calculate_hmac(const unsigned char *data, int data_length, unsigned char *output_buffer,
                    const unsigned char *key, int key_length);

/**
=======
>>>>>>> adea0023f5c375dfcd32ecb78233ae5518a6c95d
 * @brief Funktion zur AES-Entschlüsselung im CBC-Modus mit 128-Bit-Schlüssel
 * @param encrypted_data Die verschlüsselten Daten (Ciphertext)
 * @param encrypted_length Die Länge der verschlüsselten Daten
 * @param decrypted_data Der Zielpuffer, in dem der entschlüsselte Klartext gespeichert wird
 * @param key Der 16-Byte-Schlüssel für AES
 * @param iv Der 16-Byte-Initialisierungsvektor für AES-CBC
 */
void aes_decrypt(
    const unsigned char *encrypted_data,  // Eingabedaten (Ciphertext)
    int encrypted_length,                 // Länge der verschlüsselten Daten
    unsigned char *decrypted_data,        // Zielpuffer für Klartext
    const unsigned char *key,             // 16-Byte-Schlüssel
    const unsigned char *iv               // 16-Byte-Initialisierungsvektor
);

<<<<<<< HEAD
void decrypt_ciphertext(
    const unsigned char *encrypted_data, // Eingabedaten (Ciphertext + HMAC)
    int data_length,                     // Gesamtlänge der verschlüsselten Daten inkl. HMAC
    unsigned char *decrypted_data        // Zielpuffer für den entschlüsselten Klartext
);

/**
 * @brief Prints an array of unsigned characters in hexadecimal format.
 * @param array The input array.
 * @param length The length of the array.
 */
void printUnsignedCharArray(const unsigned char *array, size_t length);



=======
>>>>>>> adea0023f5c375dfcd32ecb78233ae5518a6c95d
#endif // AES_CRYPT_H
