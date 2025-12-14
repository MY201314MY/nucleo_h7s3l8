#include <zephyr/shell/shell.h>

extern int example_sntp_request(const struct shell *sh, size_t argc, char *argv[]);

SHELL_STATIC_SUBCMD_SET_CREATE(protocol_commands,
	SHELL_CMD(sntp, NULL,
		"sntp request",
		example_sntp_request),
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(protocol, &protocol_commands,
		   "example for protocol", NULL);