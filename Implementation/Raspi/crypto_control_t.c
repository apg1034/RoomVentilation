#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include "crypto_control.h"
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/err.h>
#include "common.h"

// Base64 decoding function
int base64_decode(const char *base64_input, unsigned char **decoded_output) {
    BIO *bio, *b64;
    int decode_length = 0;
    size_t input_length = strlen(base64_input);

    *decoded_output = (unsigned char *)malloc(input_length);
    if (!*decoded_output) {
        fprintf(stderr, "Error: Memory allocation failed in base64_decode.\n");
        exit(EXIT_FAILURE);
    }

    bio = BIO_new_mem_buf((void *)base64_input, -1);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

    decode_length = BIO_read(bio, *decoded_output, input_length);
    if (decode_length <= 0) {
        fprintf(stderr, "Error: Base64 decoding failed.\n");
    }

    BIO_free_all(bio);
    return decode_length;
}

// AES encryption using EVP
void aes_encrypt(
    const unsigned char *plaintext,
    int plaintext_length,
    unsigned char *encrypted_data,
    const unsigned char *key,
    const unsigned char *iv
) {
    printf("AES Encrypt Key: ");
    printUnsignedCharArray(key, 16);
    printf("AES Encrypt IV: ");
    printUnsignedCharArray(iv, 16);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        fprintf(stderr, "Error: Failed to create EVP_CIPHER_CTX in aes_encrypt.\n");
        exit(EXIT_FAILURE);
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv) != 1) {
        fprintf(stderr, "Error: EVP_EncryptInit_ex failed in aes_encrypt.\n");
        EVP_CIPHER_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }

    int len = 0;
    int ciphertext_len = 0;

    if (EVP_EncryptUpdate(ctx, encrypted_data, &len, plaintext, plaintext_length) != 1) {
        fprintf(stderr, "Error: EVP_EncryptUpdate failed in aes_encrypt.\n");
        EVP_CIPHER_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }
    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, encrypted_data + len, &len) != 1) {
        fprintf(stderr, "Error: EVP_EncryptFinal_ex failed in aes_encrypt.\n");
        EVP_CIPHER_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);
}

// AES decryption using EVP
void aes_decrypt(
    const unsigned char *encrypted_data,
    int encrypted_length,
    unsigned char *decrypted_data,
    const unsigned char *key,
    const unsigned char *iv
) {
    printf("AES Decrypt Key: ");
    printUnsignedCharArray(key, 16);
    printf("AES Decrypt IV: ");
    printUnsignedCharArray(iv, 16);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        fprintf(stderr, "Error: Failed to create EVP_CIPHER_CTX in aes_decrypt.\n");
        exit(EXIT_FAILURE);
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv) != 1) {
        fprintf(stderr, "Error: EVP_DecryptInit_ex failed in aes_decrypt.\n");
        EVP_CIPHER_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }

    int len = 0;
    int plaintext_len = 0;

    if (EVP_DecryptUpdate(ctx, decrypted_data, &len, encrypted_data, encrypted_length) != 1) {
        fprintf(stderr, "Error: EVP_DecryptUpdate failed in aes_decrypt.\n");
        EVP_CIPHER_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }
    plaintext_len = len;

    if (EVP_DecryptFinal_ex(ctx, decrypted_data + plaintext_len, &len) != 1) {
        fprintf(stderr, "Error: EVP_DecryptFinal_ex failed in aes_decrypt. Possible padding issue.\n");
        ERR_print_errors_fp(stderr);
        EVP_CIPHER_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }
    plaintext_len += len;

    decrypted_data[plaintext_len] = '\0';
    EVP_CIPHER_CTX_free(ctx);
}

// HMAC calculation
void calculate_hmac(const unsigned char *data, int data_length, unsigned char *output_buffer,
                    const unsigned char *key, int key_length) {
    unsigned int hmac_length;
    unsigned char *result = HMAC(EVP_sha256(), key, key_length, data, data_length, NULL, &hmac_length);

    if (result) {
        memcpy(output_buffer, result, hmac_length);
    } else {
        fprintf(stderr, "Error: HMAC calculation failed in calculate_hmac.\n");
        exit(EXIT_FAILURE);
    }
}

// Decrypt and verify function
void decrypt_ciphertext(
    const unsigned char *encrypted_data,
    int data_length,
    unsigned char *decrypted_data
) {
    // Requirement 6.2.1.3
    // TODO - DH
    unsigned char key[16] = {
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
        0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36
    };
    unsigned char iv[16] = { 0 };
    // Requirement 6.2.1.1
    unsigned char hmac_key[16] = {
        0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
        0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50
    };

    printf("Step 1: Raw Encrypted Data (Base64 Encoded):\n");
    printUnsignedCharArray(encrypted_data, data_length);

    // Step 2: Base64 Decode
    unsigned char *decoded_data = NULL;
    int decoded_length = base64_decode((char *)encrypted_data, &decoded_data);
    if (decoded_length <= 0 || !decoded_data) {
        fprintf(stderr, "Error: Base64 decoding failed in decrypt_ciphertext.\n");
        free(decoded_data);
        return;
    }
    printf("Step 2: Decoded Data (Hex):\n");
    printUnsignedCharArray(decoded_data, decoded_length);

    // Step 3: Split HMAC and Ciphertext
    int hmac_length = SHA256_DIGEST_LENGTH;
    if (decoded_length < hmac_length) {
        fprintf(stderr, "Error: Data length is too short to contain HMAC in decrypt_ciphertext.\n");
        free(decoded_data);
        return;
    }

    unsigned char *received_hmac = decoded_data + (decoded_length - hmac_length);
    int ciphertext_length = decoded_length - hmac_length;

    printf("Step 3: Data Input to HMAC (Hex):\n");
    printUnsignedCharArray(decoded_data, ciphertext_length);

    // Step 4: Calculate HMAC
    unsigned char computed_hmac[SHA256_DIGEST_LENGTH];
    calculate_hmac(decoded_data, ciphertext_length, computed_hmac, hmac_key, sizeof(hmac_key));

    printf("Step 4: Received HMAC (Hex):\n");
    printUnsignedCharArray(received_hmac, hmac_length);
    printf("Step 4: Computed HMAC (Hex):\n");
    printUnsignedCharArray(computed_hmac, hmac_length);

    // Step 5: Verify HMAC
    if (CRYPTO_memcmp(received_hmac, computed_hmac, hmac_length) != 0) {
        fprintf(stderr, "Error: HMAC verification failed in decrypt_ciphertext.\n");
        free(decoded_data);
        return;
    }

    printf("Step 5: HMAC verification successful.\n");

    // Step 6: Decrypt Ciphertext
    memset(decrypted_data, 0, ciphertext_length);
    aes_decrypt(decoded_data, ciphertext_length, decrypted_data, key, iv);

    printf("Step 6: Decrypted Data (String): %s\n", decrypted_data);

    free(decoded_data);
}
