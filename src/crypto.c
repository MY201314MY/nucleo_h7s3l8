#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

#include "cryptoauthlib.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(crypto, LOG_LEVEL_DBG);

ATCAIfaceCfg iface_config = {
    .iface_type            = ATCA_I2C_IFACE,
    .cfg_data              = (void *)"i2c@40005400",
    .devtype               = ATECC608B,
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

int _read_config()
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

