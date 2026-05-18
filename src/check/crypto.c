#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

#include <mbedtls/ssl.h>
#include <mbedtls/cipher.h>
#include <mbedtls/version.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(crypto, LOG_LEVEL_DBG);

static void _ecdsa_test(void) {
    extern int _ecdsa(int type);
    _ecdsa(0);
    _ecdsa(1);
    _ecdsa(2);
}

static int _ecdhe_test()
{
    extern int _ecdhe(int type);
    _ecdhe(0);
    _ecdhe(1);
    _ecdhe(2);

    return 0;
}

static int _rsa_test()
{
    int _rsa(int type);
    _rsa(0);

    return 0;
}

static int _hash_test()
{
    int crypto_hash_test();

    crypto_hash_test();
    
    return 0;
}

static void debug_supported_ciphersuites(void)
{
    const int *ciphersuites = mbedtls_ssl_list_ciphersuites();
    int i = 0;

    LOG_INF("Supported TLS ciphersuites:");
    while (ciphersuites[i] != 0) {
        const char *name = mbedtls_ssl_get_ciphersuite_name(ciphersuites[i]);
        if (name) {
            LOG_INF("%s", name);
        }
        i++;
    }
}

static int example_crypto_operations(const struct shell *sh, size_t argc, char *argv[])
{
    int operation = atoi(argv[1]);
    
    LOG_DBG("operation %d selected", operation);

    if(operation == 0)
    {
        _ecdsa_test();
    }
    else if(operation == 1)
    {
        _ecdhe_test();
    }
    else if(operation == 2)
    {
        _rsa_test();
    }
    else if(operation == 3)
    {
        _hash_test();
    }
    else if(operation == 4)
    {
        int soft_crypto_hash_test(void);
        soft_crypto_hash_test();
    }
    else if(operation == 5)
    {
        debug_supported_ciphersuites();
    }
    else if(operation == 6)
    {
        int crypto_aes_hw_test(void);
        crypto_aes_hw_test();
    }
    else if(operation == 7)
    {
        int crypto_aes_mbedtls_test(void);
        crypto_aes_mbedtls_test();
    }
    else 
    {
        LOG_WRN("unknown operation");
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

