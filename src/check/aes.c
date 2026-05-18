#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <string.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(aes_sw, LOG_LEVEL_INF);

#include <mbedtls/aes.h>
#include <mbedtls/gcm.h>
#include "../mbedtls/mbedtls-aes.h"

#define PLAINTEXT_SIZE    48  /* 12 * 4 */
static uint8_t Plaintext[48] = {
    0x10,0x0F,0x00,0x08, 0x14,0x13,0x12,0x11, 0x18,0x17,0x16,0x15,
    0x1C,0x1B,0x1A,0x19, 0x20,0x1F,0x1E,0x1D, 0x24,0x23,0x22,0x21,
    0x28,0x27,0x26,0x25, 0x2C,0x2B,0x2A,0x29, 0x30,0x2F,0x2E,0x2D,
    0x34,0x33,0x32,0x31, 0x38,0x37,0x36,0x35, 0x02,0x00,0x3A,0x39
};

uint8_t EncryptedText[48]= {0};
uint8_t DecryptedText[48]= {0};
uint8_t TAG[16]          = {0};

int crypto_aes_mbedtls_test(void)
{
    int ret = -1;

    LOG_HEXDUMP_INF((uint8_t*)Plaintext, sizeof(Plaintext), "text");

    mbedtls_gcm_context gcm_ctx;
    mbedtls_gcm_init(&gcm_ctx);

    mbedtls_gcm_setkey(&gcm_ctx, MBEDTLS_CIPHER_ID_AES, key, 256);

    ret = mbedtls_gcm_crypt_and_tag(&gcm_ctx,
                    MBEDTLS_GCM_ENCRYPT,
                    PLAINTEXT_SIZE,
                    iv, 16,
                    header, 28,
                    Plaintext,
                    EncryptedText,
                    16, TAG);

    if (ret == 0) {
        LOG_HEXDUMP_INF(EncryptedText, PLAINTEXT_SIZE, "SW encrypt");
        LOG_HEXDUMP_INF(TAG, 16, "SW tag");
    }

    ret = mbedtls_gcm_crypt_and_tag(&gcm_ctx,
                    MBEDTLS_GCM_DECRYPT,
                    PLAINTEXT_SIZE,
                    iv, 16,
                    header, 28,
                    EncryptedText,
                    DecryptedText,
                    16, TAG);

    if (ret == 0) {
        LOG_HEXDUMP_INF(DecryptedText, PLAINTEXT_SIZE, "SW decrypt");
    }

    mbedtls_gcm_free(&gcm_ctx);

    return 0;
}
