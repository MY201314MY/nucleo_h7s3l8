#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(hash_sw, LOG_LEVEL_DBG);

/* mbedtls 软件哈希头文件 */
#include <mbedtls/sha1.h>
#include <mbedtls/sha256.h>
#include <mbedtls/sha512.h>

static int crypto_hash(uint32_t type, const uint8_t *text, size_t size, uint8_t *digest)
{
    int ret = -1;

    switch(type)
    {
        case HASH_ALGOSELECTION_SHA1: {
            mbedtls_sha1_context ctx;
            mbedtls_sha1_init(&ctx);
            mbedtls_sha1_starts(&ctx);
            mbedtls_sha1_update(&ctx, text, size);
            mbedtls_sha1_finish(&ctx, digest);
            mbedtls_sha1_free(&ctx);
            ret = 0;
            break;
        }

        case HASH_ALGOSELECTION_SHA224: {
            mbedtls_sha256_context ctx;
            mbedtls_sha256_init(&ctx);
            mbedtls_sha256_starts(&ctx, 1); /* 1 = SHA224 */
            mbedtls_sha256_update(&ctx, text, size);
            mbedtls_sha256_finish(&ctx, digest);
            mbedtls_sha256_free(&ctx);
            ret = 0;
            break;
        }

        case HASH_ALGOSELECTION_SHA256: {
            mbedtls_sha256_context ctx;
            mbedtls_sha256_init(&ctx);
            mbedtls_sha256_starts(&ctx, 0); /* 0 = SHA256 */
            mbedtls_sha256_update(&ctx, text, size);
            mbedtls_sha256_finish(&ctx, digest);
            mbedtls_sha256_free(&ctx);
            ret = 0;
            break;
        }

        case HASH_ALGOSELECTION_SHA384: {
            mbedtls_sha512_context ctx;
            mbedtls_sha512_init(&ctx);
            mbedtls_sha512_starts(&ctx, 1); /* 1 = SHA384 */
            mbedtls_sha512_update(&ctx, text, size);
            mbedtls_sha512_finish(&ctx, digest);
            mbedtls_sha512_free(&ctx);
            ret = 0;
            break;
        }

        case HASH_ALGOSELECTION_SHA512: {
            mbedtls_sha512_context ctx;
            mbedtls_sha512_init(&ctx);
            mbedtls_sha512_starts(&ctx, 0); /* 0 = SHA512 */
            mbedtls_sha512_update(&ctx, text, size);
            mbedtls_sha512_finish(&ctx, digest);
            mbedtls_sha512_free(&ctx);
            ret = 0;
            break;
        }

        default:
            return -EAGAIN;
    }

    return ret;
}

int soft_crypto_hash_test(void)
{
    uint8_t digest[64] = {0};
    const uint8_t text[] = "STM32H7 is a high-performance microcontrollers family based on Arm Cortex-M7 32-bit RISC core. It offers native and installable security services.";

    crypto_hash(HASH_ALGOSELECTION_SHA1, text, strlen(text), digest);
    LOG_HEXDUMP_INF(digest, 20, "SW SHA1");

    crypto_hash(HASH_ALGOSELECTION_SHA224, text, strlen(text), digest);
    LOG_HEXDUMP_INF(digest, 28, "SW SHA224");

    crypto_hash(HASH_ALGOSELECTION_SHA256, text, strlen(text), digest);
    LOG_HEXDUMP_INF(digest, 32, "SW SHA256");

    crypto_hash(HASH_ALGOSELECTION_SHA384, text, strlen(text), digest);
    LOG_HEXDUMP_INF(digest, 48, "SW SHA384");

    crypto_hash(HASH_ALGOSELECTION_SHA512, text, strlen(text), digest);
    LOG_HEXDUMP_INF(digest, 64, "SW SHA512");

    return 0;
}
