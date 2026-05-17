#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <stm32h7rsxx_hal.h>

#include <stdlib.h>
#include <zephyr/syscalls/time_units.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <stm32h7rsxx_hal.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(hash, LOG_LEVEL_DBG);

static HASH_HandleTypeDef hhash = {0};

int crypto_hash_init(void)
{
    __HAL_RCC_HASH_CLK_ENABLE();
    
    return 0;
}

static int crypto_hash(uint32_t type, const uint8_t *text, size_t size, uint8_t *digest)
{
    size_t _size = 0;
    uint8_t _digest[64] = {0};

    switch(type)
    {
        case HASH_ALGOSELECTION_SHA1:
            _size = 20;
            hhash.Init.Algorithm = HASH_ALGOSELECTION_SHA1;
        break;

        case HASH_ALGOSELECTION_SHA224:
            _size = 28;
            hhash.Init.Algorithm = HASH_ALGOSELECTION_SHA224;
        break;

        case HASH_ALGOSELECTION_SHA256:
            _size = 32;
            hhash.Init.Algorithm = HASH_ALGOSELECTION_SHA256;
        break;

        case HASH_ALGOSELECTION_SHA384:
            _size = 48;
            hhash.Init.Algorithm = HASH_ALGOSELECTION_SHA384;
        break;

        case HASH_ALGOSELECTION_SHA512:
            _size = 64;
            hhash.Init.Algorithm = HASH_ALGOSELECTION_SHA512;
        break;

        default:
        return -EAGAIN;
    }

    hhash.Instance = HASH;
    hhash.Init.DataType = HASH_BYTE_SWAP;
    HAL_HASH_Init(&hhash);

    if (HAL_HASH_Start(&hhash, text, size, _digest, 1000) != HAL_OK)
    {
        LOG_ERR("HAL_HASH_Start failed with ret %d", HAL_HASH_GetError(&hhash));
    }
    memcpy(digest, _digest, _size);
    HAL_HASH_DeInit(&hhash);

    return 0;
}

int crypto_hash_test()
{
    uint8_t digest[64] = {0};
    const uint8_t text[] = "STM32H7 is a high-performance microcontrollers family based on Arm Cortex-M7 32-bit RISC core. It offers native and installable security services.";

    crypto_hash(HASH_ALGOSELECTION_SHA1, text, strlen(text), digest);
    LOG_HEXDUMP_INF(digest, 20, "SHA1 Digest");

    crypto_hash(HASH_ALGOSELECTION_SHA224, text, strlen(text), digest);
    LOG_HEXDUMP_INF(digest, 28, "SHA224 Digest");

    crypto_hash(HASH_ALGOSELECTION_SHA256, text, strlen(text), digest);
    LOG_HEXDUMP_INF(digest, 32, "SHA256 Digest");

    crypto_hash(HASH_ALGOSELECTION_SHA384, text, strlen(text), digest);
    LOG_HEXDUMP_INF(digest, 48, "SHA384 Digest");

    crypto_hash(HASH_ALGOSELECTION_SHA512, text, strlen(text), digest);
    LOG_HEXDUMP_INF(digest, 64, "SHA512 Digest");

    return 0;
}

SYS_INIT(crypto_hash_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

