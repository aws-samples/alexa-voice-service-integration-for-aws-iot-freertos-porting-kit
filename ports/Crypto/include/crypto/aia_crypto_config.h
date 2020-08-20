/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef AIA_CRYPTO_CONFIG_H_
#ifdef __cplusplus
extern "C" {
#endif
#define AIA_CRYPTO_CONFIG_H_

/* AIA headers */
#include <aiacore/aia_crypto_mbedtls.h>
#include <aiacore/aia_encryption_algorithm.h>
#include <aiacore/aia_random_mbedtls.h>
#include <aiacore/aia_secret_derivation_algorithm.h>
#include <aiacore/mbedtls/aia_base64_mbedtls.h>

static const AiaSecretDerivationAlgorithm_t SECRET_DERIVATION_ALGORITHM =
    AIA_ECDH_CURVE_25519_32_BYTE;

/**
 * Seeds the random number generator.
 * @note Implementations of this function must be thread safe.
 *
 * @param salt Changes the starting point for the random generator (may
 * be NULL and does not need to be '\0' terminated).
 * @param saltLength The length of @c salt.
 * @return @c true if random number generator is successfully seeded, else @c
 * false.
 */
static inline bool AiaRandom_Seed( const char* salt, size_t saltLength )
{
    return AiaRandomMbedtls_Seed( salt, saltLength );
}

/**
 * Generates a cryptographically secure random number.
 * @note Implementations of this function must be thread safe.
 *
 * @param [out] buffer The buffer to fill with random data
 * @param bufferLength The length of @c buffer.
 * @return @c true if random number is generated successfully, else
 * @c false.
 */
static inline bool AiaRandom_Rand( unsigned char* buffer, size_t bufferLength )
{
    return AiaRandomMbedtls_Rand( buffer, bufferLength );
}

/**
 * Sets the key for encryption/decryption functions.
 *
 * @param encryptKey The encryption key to use.
 * @param encryptKeySize The size of @c encryptKey (in bytes).
 * @param encryptAlgorithm The encryption algorithm to use.
 *
 * @return @c true if the key is set successfully, else @c false.
 */
bool AiaCrypto_SetKey( const uint8_t* encryptKey, size_t encryptKeySize,
                       const AiaEncryptionAlgorithm_t encryptAlgorithm );

/**
 * Encrypts given input data. This function is thread-safe.
 *
 * @param inputData The buffer that holds the data to encrypt. Has to be at
 * least @c inputLen bytes in size.
 * @param inputLen The size of the buffer that holds the input data in bytes.
 * Also equal to the size of the @c outputData.
 * @param [out] outputData The buffer that holds the ciphertext. Has to be at
 * least @c inputLen bytes in size. For encryption, the outputData buffer can be
 * the same as the inputData buffer.
 * @param [out] iv The initialization vector will be generated with a size of
 * @c ivLen bytes for the user.
 * @param ivLen The length of the initialization vector.
 * @param [out] tag The buffer for holding the tag generated during encryption.
 * Has to be at least @c tagLen bytes in size.
 * @param tagLen The length of the tag generated during encryption.
 *
 * @return @c true if encryption is successful, else @c false.
 */
static inline bool AiaCrypto_Encrypt( const uint8_t* inputData,
                                      const size_t inputLen,
                                      uint8_t* outputData, uint8_t* iv,
                                      size_t ivLen, uint8_t* tag,
                                      const size_t tagLen )
{
    return AiaCryptoMbedtls_Encrypt( inputData, inputLen, outputData, iv, ivLen,
                                     tag, tagLen );
}

/**
 * Decrypts given encrypted data. This function is thread-safe.
 *
 * @param inputData The buffer holding the ciphertext.
 * @param inputLen The length of the ciphertext to decrypt. Also equal to the
 * length of the decrypted plaintext.
 * @param [out] outputData The buffer that holds the plaintext. Has to be at
 * least @c inputLen bytes in size. For decryption, the outputData buffer cannot
 * be the same as inputData buffer. If the buffers overlap, the outputData
 * buffer must trail at least 8 Bytes behind the inputData buffer.
 * @param iv The initialization vector of at least @c ivLen bytes.
 * @param ivLen The length of the initialization vector.
 * @param tag The buffer for holding the tag to verify. Has to be at least @c
 * tagLen bytes in size.
 * @param tagLen The length of the tag generated during encryption.
 *
 * @return @c true if decryption is successful, else @c false.
 */
static inline bool AiaCrypto_Decrypt( const uint8_t* inputData,
                                      const size_t inputLen,
                                      uint8_t* outputData, const uint8_t* iv,
                                      const size_t ivLen, const uint8_t* tag,
                                      const size_t tagLen )
{
    return AiaCryptoMbedtls_Decrypt( inputData, inputLen, outputData, iv, ivLen,
                                     tag, tagLen );
}

/**
 * Generates a key pair intended for the specified shared secret calculation.
 *
 * @param secretDerivationAlgorithm The secret derivation algorithm to generate
 * a key pair for.
 * @param[out] privateKey The buffer to write binary private key data into.
 * @param privateKeyLen The length of @c privateKey.
 * @param[out] publicKey The buffer to write binary public key data into.
 * @param publicKeyLen The length of @c publicKey.
 * @return @c true if key pair generated successfully, else @c false.
 */
static inline bool AiaCrypto_GenerateKeyPair(
    AiaSecretDerivationAlgorithm_t secretDerivationAlgorithm,
    uint8_t* privateKey, size_t privateKeyLen, uint8_t* publicKey,
    size_t publicKeyLen )
{
    return AiaCryptoMbedtls_GenerateKeyPair( secretDerivationAlgorithm,
                                             privateKey, privateKeyLen,
                                             publicKey, publicKeyLen );
}

/**
 * Calculates the shared secret between client and service
 *
 * @param clientPrivateKey The buffer holding binary client private key data.
 * @param clientPrivateKeyLen The length of @c clientPrivateKey.
 * @param servicePublicKey The buffer holding binary service public key data.
 * @param servicePublicKeyLen The length of @c servicePublicKey.
 * @param secretDerivationAlgorithm The secret derivation algorithm to use.
 * @param[out] sharedSecret The buffer to write binary shared secret data into.
 * @param sharedSecretLen The length of @c sharedSecret
 * @return @c true if shared secret is calculated successfully, else @c false.
 */
static inline bool AiaCrypto_CalculateSharedSecret(
    const uint8_t* clientPrivateKey, size_t clientPrivateKeyLen,
    const uint8_t* servicePublicKey, size_t servicePublicKeyLen,
    AiaSecretDerivationAlgorithm_t secretDerivationAlgorithm,
    uint8_t* sharedSecret, size_t sharedSecretLen )
{
    return AiaCryptoMbedtls_CalculateSharedSecret(
        clientPrivateKey, clientPrivateKeyLen, servicePublicKey,
        servicePublicKeyLen, secretDerivationAlgorithm, sharedSecret,
        sharedSecretLen );
}

/**
 * Returns the size required for an output buffer for encoding to base64.
 *
 * @param input The buffer to encode/decode.
 * @param inputLen The length of @c input.
 * @return The required buffer size for encoding to or decoding from base64. @c
 * 0 will be returned on failures.
 */
static inline size_t Aia_Base64GetEncodeSize( const uint8_t* input,
                                              size_t inputLen )
{
    return AiaBase64MbedTls_GetEncodeSize( input, inputLen );
}

/**
 * Returns the size required for an output buffer for decoding from base64.
 *
 * @param input The buffer to encode/decode.
 * @param inputLen The length of @c input.
 * @return The required buffer size for encoding to or decoding from base64. @c
 * 0 will be returned on failures.
 */
static inline size_t Aia_Base64GetDecodeSize( const uint8_t* input,
                                              size_t inputLen )
{
    return AiaBase64MbedTls_GetDecodeSize( input, inputLen );
}

/**
 * Encodes the provided buffer using base64 encoding.
 *
 * @param input The buffer to encode.
 * @param inputLen The length of @c input.
 * @param[out] output The output buffer to write encoded bytes into. This cannot
 * be the same as @c input.
 * @param outputLen The length of @c output.
 * @return @c true if successful or @c false otherwise.
 * @note Users should use @c Aia_Base64GetEncodeSize to obtain the required size
 * of @c output.
 */
static inline bool Aia_Base64Encode( const uint8_t* input, size_t inputLen,
                                     uint8_t* output, size_t outputLen )
{
    return AiaBase64MbedTls_Encode( input, inputLen, output, outputLen );
}

/**
 * Decodes a base64 encoded buffer.
 *
 * @param input The base64 encoded buffer to decode.
 * @param inputLen The length of @c input.
 * @param[out] output The output buffer to write decoded bytes into. This cannot
 * be the same as @c input.
 * @param outputLen The length of @c output.
 * @return @c true if successful or @c false otherwise.
 * @note Users should use @c Aia_Base64GetDecodeSize to obtain the required size
 * of @c output.
 */
static inline bool Aia_Base64Decode( const uint8_t* input, size_t inputLen,
                                     uint8_t* output, size_t outputLen )
{
    return AiaBase64MbedTls_Decode( input, inputLen, output, outputLen );
}

#ifdef __cplusplus
}
#endif
#endif /* ifndef AIA_CRYPTO_CONFIG_H_ */
