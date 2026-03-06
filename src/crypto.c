#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

#include "cryptoauthlib.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(crypto, LOG_LEVEL_DBG);

static ATCAIfaceCfg iface_config = {
    .iface_type            = ATCA_I2C_IFACE,
    .cfg_data              = (void *)"i2c@40005400",
    .devtype               = ATECC608A,
    {
        .atcai2c           = {
        .address       = 0xC0,
        .bus           = 0,
        .baud          = 400000,
        },
    },
    .wake_delay            = 2000,
    .rx_retries            = 20
};

int _crypto_atecc608_init()
{
    ATCA_STATUS status = atcab_init(&iface_config);
    if (status != ATCA_SUCCESS)
    {
        LOG_ERR("atcab_init() failed: %02x", status);
        return -ENODEV;
    }
    
    return 0;
}

static int _read_config()
{
    ATCA_STATUS status;
    uint8_t config[200];
    size_t config_size = 0;
    uint8_t zone = ATCA_ZONE_CONFIG;

    do
    {
        status = atcab_get_zone_size(zone, 0, &config_size);
        if (status != ATCA_SUCCESS)
        {
            LOG_ERR("atcab_get_zone_size() failed: %02x\r\n", status);
            break;
        }

        status = atcab_read_config_zone(config);
        if (status != ATCA_SUCCESS)
        {
            LOG_ERR("atcab_read_config_zone() failed: %02x\r\n", status);
            break;
        }

        LOG_HEXDUMP_INF(config, config_size, "config");
    }
    while (0);
    
    LOG_HEXDUMP_INF(config + 21, 32, "slot config");
    LOG_HEXDUMP_INF(config + 96, 32, "key config");

    return 0;
}

static int _read_data()
{
    ATCA_STATUS status;
    size_t slot_size = 0;
    uint8_t buffer[512] = {0};

    for(uint16_t slot=8; slot<15;slot++)
    {
        status = atcab_get_zone_size(ATCA_ZONE_DATA, slot, &slot_size);
        if (status != ATCA_SUCCESS) {
            LOG_ERR("atcab_get_zone_size failed with ret=%d", status);
            continue;
        }
        LOG_INF("slot:%d --- size:%d", slot, slot_size);
        status = atcab_read_zone(ATCA_ZONE_DATA, slot, 0, 0, buffer, 32);

        if (status == ATCA_SUCCESS) {
            LOG_HEXDUMP_INF(buffer, 32, "RX");
        } else if (status == ATCA_NOT_LOCKED) {
            LOG_WRN("slot not locked\n");
        } else {
            LOG_ERR("atcab_read_zone failed with ret=%d", status);
        }
    }

    return 0;
}

static int _write_data()
{
    uint8_t key[32] = {0};
    ATCA_STATUS status;

    for(int i=0; i<32; i++) 
    {
        key[i] = i;
    }
    status = atcab_write_zone(ATCA_ZONE_DATA, 1, 0, 0, key, 32);

    if (status == ATCA_SUCCESS) {
        LOG_INF("slot 9 write success.");
    } else {
        LOG_ERR("slot 9 write failed with ret=%d", status);
    }

    return 0;
}

static int _lock_config_crypto_atecc608()
{
    ATCA_STATUS status;
    bool is_locked = false;

    if ((status = atcab_is_config_locked(&is_locked)) != ATCA_SUCCESS)
    {
        LOG_ERR("atcab_is_config_locked() failed with ret=0x%08X", status);
    }
    else
    {
        LOG_INF("config zone: %s", is_locked ? "locked" : "unlocked");
    }

    if(is_locked == true)
    {
        return 0;
    }

    status = atcab_lock_config_zone();
    if(status == ATCA_SUCCESS)
    {
        atcab_wakeup();
        LOG_INF("ATECC608 locked success.");
        return 0;
    }
    else{
        
        LOG_ERR("ATECC608 locked failed with ret=%d", status);
        return -EAGAIN;
    }
}

static int _lock_data_crypto_atecc608()
{
    ATCA_STATUS status;
    bool is_locked = false;

    if ((status = atcab_is_data_locked(&is_locked)) != ATCA_SUCCESS)
    {
        LOG_ERR("atcab_is_data_locked() failed with ret=0x%08X", status);
    }
    else
    {
        LOG_INF("data zone: %s", is_locked ? "locked" : "unlocked");
    }

    if(is_locked == true)
    {
        return 0;
    }

    status = atcab_lock_data_zone();
    if(status == ATCA_SUCCESS)
    {
        atcab_wakeup();
        LOG_INF("ATECC608 data zone locked success.");
        return 0;
    }
    else{
        
        LOG_ERR("ATECC608 data zone locked failed with ret=%d", status);
        return -EAGAIN;
    }
}

static int _verify_aes(void) {
    ATCA_STATUS status;
    uint8_t plaintext[16] = {0};
    uint8_t ciphertext[16] = {0};
    for(uint8_t slot=0;slot<15;slot++)
    {
        status = atcab_aes_encrypt(slot, 0, plaintext, ciphertext);

        if (status == ATCA_SUCCESS) {
            LOG_INF("AES crypto success.");
            LOG_HEXDUMP_INF(ciphertext, 16, "AES Ciphertext");
        } else {
            LOG_ERR("AES crypto failed with ret=%d", status);
        }
    }

    return 0;
}

static int _generate_ecc_key()
{
    ATCA_STATUS status;
    uint8_t buffer[64];

    status = atcab_genkey(0, buffer);
    if (status == ATCA_SUCCESS) {
        LOG_INF("ECC genkey on slot 0 success.");
        LOG_HEXDUMP_INF(buffer, sizeof(buffer), "public key");
    }
    else{
        LOG_ERR("atcab_genkey failed with ret=%d", status);
    }

    status = atcab_get_pubkey(0, buffer);
    if(status == ATCA_SUCCESS)
    {
        LOG_HEXDUMP_INF(buffer, sizeof(buffer), "public key check");
    }
    else
    {
        LOG_ERR("atcab_get_pubkey failed with ret=%d", status);
    }

    return 0;
}

static int _ecc_hardware_test(void) {
    ATCA_STATUS status;
    uint8_t pubkey[64];
    uint8_t msg_hash[32];
    uint8_t signature[64];
    bool is_verified = false;

    memset(msg_hash, 0xAA, 32);

    status = atcab_genkey(1, pubkey);
    
    if(status == ATCA_SUCCESS)
    {
        LOG_INF("key generate success.");
        status = atcab_sign(1, msg_hash, signature);

        if (status == ATCA_SUCCESS) {
            LOG_INF("sign success");
            status = atcab_verify_extern(msg_hash, signature, pubkey, &is_verified);
            if(status == ATCA_SUCCESS)
            {
                LOG_INF("verify status : %s", is_verified == true?"true":"false");
            }
            else
            {
                LOG_ERR("atcab_verify_extern failed with ret=%d", status);
            }
        }
        else{
            LOG_ERR("atcab_sign failed with ret=%d", status);
        }
    }

    return 0;
}

static void _ecdsa_software_test(void) {
    extern int _ecdsa();
    _ecdsa();
}

static int _ecdhe_software_test()
{
    extern int _ecdhe();
    _ecdhe();

    return 0;
}

static void _crypto_ecdhe()
{
    uint8_t slot_num = 1;
    uint8_t public_key[ATCA_PUB_KEY_SIZE];
    uint8_t shared_key[ATCA_KEY_SIZE];
    uint8_t buffer[ATCA_PUB_KEY_SIZE] = {0};
    ATCA_STATUS status;

    /* only slot 1 is valid. */

    for(slot_num=1;slot_num<2;slot_num++)
    {
        LOG_INF("slot number : %d", slot_num);
        status = atcab_genkey(slot_num, public_key);
        LOG_INF("ret : %d", status);

        LOG_HEXDUMP_INF(public_key, sizeof(public_key), "public");

        memcpy(buffer, public_key, ATCA_PUB_KEY_SIZE);
        
        status = atcab_ecdh(slot_num, public_key, shared_key);
        LOG_INF("ret : %d", status);
        if(status == ATCA_SUCCESS)
        {
            LOG_INF("ECDH success! shared secret generated.");
            LOG_HEXDUMP_INF(shared_key, sizeof(shared_key), "key");
        }
        else
        {
            LOG_ERR("atcab_ecdh() failed with ret=0x%08X", status);
        }
    }
}

static int example_crypto_operations(const struct shell *sh, size_t argc, char *argv[])
{
    int operation = atoi(argv[1]);
    
    LOG_DBG("operation %d selected", operation);

    if(operation == 0)
    {
        ATCA_STATUS status;
        bool is_locked = false;

        {
            if ((status = atcab_is_config_locked(&is_locked)) != ATCA_SUCCESS)
            {
                LOG_ERR("atcab_is_config_locked() failed with ret=0x%08X", status);
            }
            else
            {
                LOG_INF("config zone: %s", is_locked ? "locked" : "unlocked");
            }
        }

        {
            if ((status = atcab_is_data_locked(&is_locked)) != ATCA_SUCCESS)
            {
                LOG_ERR("atcab_is_data_locked() failed with ret=0x%08X", status);
            }
            else
            {
                LOG_INF("data zone: %s", is_locked ? "locked" : "unlocked");
            }
        }

        {
            uint8_t revision[4] = {0};

            status = atcab_info(revision);
            if (status != ATCA_SUCCESS)
            {
                LOG_ERR("atcab_info() failed with ret=0x%08X", status);
            }
            LOG_HEXDUMP_INF(revision, sizeof(revision), "version");
        }

        {
            uint8_t sernum[9] = {0};

            status = atcab_read_serial_number(sernum);
            if (status != ATCA_SUCCESS)
            {
                LOG_ERR("atcab_read_serial_number() failed with ret=0x%08X", status);
            }

            LOG_HEXDUMP_INF(sernum, sizeof(sernum), "serial number");
        }

        LOG_INF("ATECC608A init success.");
    }
    else if(operation == 1)
    {
        _read_config();
    }
    else if(operation == 2)
    {
        _read_data();
    }
    else if(operation == 3)
    {
        _write_data();
    }
    else if(operation == 4)
    {
        _verify_aes();
    }
    else if(operation == 5)
    {
        _generate_ecc_key();
    }
    else if(operation == 6)
    {
        _ecc_hardware_test();
    }
    else if(operation == 7)
    {
        _ecdsa_software_test();
    }
    else if(operation == 8)
    {
        _ecdhe_software_test();
    }
    else if(operation == 9)
    {
        _crypto_ecdhe();
    }
    else if(operation == 10086)
    {
    /* 
        [00:00:09.335,000] <dbg> crypto: example_crypto_operations: operation 10086 selected
        [00:00:09.349,000] <inf> crypto: config zone: unlocked
        [00:00:09.380,000] <inf> crypto: ATECC608 locked success.
    */
        _lock_config_crypto_atecc608();
    }
    else if(operation == 1008611)
    {
        _lock_data_crypto_atecc608();
    }
    else 
    {
        LOG_DBG("unknown operation");
    }

    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(crypto_commands,
	SHELL_CMD(num, NULL,
		"operation",
		example_crypto_operations),
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(crypto, &crypto_commands,
		   "example for crypto", NULL);

