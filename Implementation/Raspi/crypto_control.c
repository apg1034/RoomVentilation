#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/aes.h>
#include "crypto_control.h"
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

int base64_decode(const char *base64_input, unsigned char **decoded_output) {
    BIO *bio, *b64;
    int decode_length = 0;
    size_t input_length = strlen(base64_input);

    // Speicher für das Ergebnis
    *decoded_output = (unsigned char *)malloc(input_length);
    if (!*decoded_output) {
        fprintf(stderr, "Speicher konnte nicht allokiert werden.\n");
        exit(EXIT_FAILURE);
    }

    // Base64 BIOs erstellen
    bio = BIO_new_mem_buf((void *)base64_input, -1);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    // Base64 BIOs konfigurieren
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); // Kein Zeilenumbruch

    // Dekodierung durchführen
    decode_length = BIO_read(bio, *decoded_output, input_length);
    BIO_free_all(bio);

    return decode_length;
}

void aes_encrypt(
    const unsigned char *plaintext,
    int plaintext_length,
    unsigned char *encrypted_data,
    const unsigned char *key,
    const unsigned char *iv
) {
    AES_KEY encrypt_key;

    AES_set_encrypt_key(key, 128, &encrypt_key);

    AES_cbc_encrypt(plaintext, encrypted_data, plaintext_length, &encrypt_key, (unsigned char *)iv, AES_ENCRYPT);
}

void aes_decrypt(
    const unsigned char *encrypted_data,
    int encrypted_length,
    unsigned char *decrypted_data,
    const unsigned char *key,
    const unsigned char *iv
) {
    AES_KEY decrypt_key;

    AES_set_decrypt_key(key, 128, &decrypt_key);

    AES_cbc_encrypt(encrypted_data, decrypted_data, encrypted_length, &decrypt_key, (unsigned char *)iv, AES_DECRYPT);
}

int main() {
    // Beispiel-Daten (Ciphertext, Schlüssel, IV)
    unsigned char encrypted_data[] = {
        0x8d, 0x89, 0x32, 0xca, 0x26, 0x2b, 0xe9, 0xe2, 0x6d, 0xec, 0xa2, 0x7e, 0xb9, 0x54, 0xa3, 0xce
    }; // Beispiel: Verschlüsselter Text
    int data_length = sizeof(encrypted_data);

    unsigned char key[16] = {
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
        0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36
    }; // Beispiel-Schlüssel: "1234567890123456"

    unsigned char iv[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    // Puffer für den entschlüsselten Text (gleiche Größe wie Eingabedaten)
    unsigned char decrypted_data[data_length];
    memset(decrypted_data, 0, data_length);

    // Entschlüsselung durchführen
    aes_decrypt(encrypted_data, data_length, decrypted_data, key, iv);

    // Ergebnis ausgeben
    printf("Entschlüsselte Daten: ");
    for (int i = 0; i < data_length; i++) {
        if (decrypted_data[i] == '\0') break; // Stoppe bei Null-Terminierung
        printf("%c", decrypted_data[i]);
    }
    printf("\n");

    return 0;
}
