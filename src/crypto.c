#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(crypto, LOG_LEVEL_DBG);

static void _ecdsa_software_test(void) {
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

static int example_crypto_operations(const struct shell *sh, size_t argc, char *argv[])
{
    int operation = atoi(argv[1]);
    
    LOG_DBG("operation %d selected", operation);

    if(operation == 7)
    {
        _ecdsa_software_test();
    }
    else if(operation == 8)
    {
        _ecdhe_test();
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

