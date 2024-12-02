#ifndef AES_CRYPT_H
#define AES_CRYPT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/aes.h>

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

#endif // AES_CRYPT_H
