#include <zephyr/kernel.h>
#include <stdlib.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/can.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(operation, LOG_LEVEL_DBG);

static char *can_bus_state_to_str(enum can_state state)
{
	switch (state) {
	case CAN_STATE_ERROR_ACTIVE:
		return "error-active";
	case CAN_STATE_ERROR_WARNING:
		return "error-warning";
	case CAN_STATE_ERROR_PASSIVE:
		return "error-passive";
	case CAN_STATE_BUS_OFF:
		return "bus-off";
	case CAN_STATE_STOPPED:
		return "stopped";
	default:
		return "unknown";
	}
}

int example_operations(const struct shell *sh, size_t argc, char *argv[])
{
    int operation = atoi(argv[1]);
    
    LOG_DBG("operation %d selected", operation);

    if(operation == 0) {
        LOG_WRN("reboot at once.");

        k_sleep(K_MSEC(200));
        sys_reboot(SYS_REBOOT_COLD);
    }
    else if(operation == 1) {
        const struct device *dev = DEVICE_DT_GET_ONE(st_stm32_temp_cal);
        struct sensor_value temp;

        if (!device_is_ready(dev)) {
            LOG_ERR("temperature sensor device not ready");
            return -ENODEV;
        }
        if (sensor_sample_fetch(dev) < 0) {
			LOG_ERR("temperature sensor sample update error");
			return -EAGAIN;
		}
        sensor_channel_get(dev, SENSOR_CHAN_DIE_TEMP, &temp);
        LOG_INF("temperature: %.1f C", sensor_value_to_double(&temp));

    } else if(operation == 2) {
        const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));
        struct can_bus_err_cnt err_cnt = {0, 0};
        enum can_state state;

        int ret = can_get_state(dev, &state, &err_cnt);
		if (ret != 0) {
			LOG_ERR("failed to get can controller state: %d", ret);
		}
        else
        {
            LOG_INF("can bus state : %s", can_bus_state_to_str(state));
        }
    } else {
        LOG_DBG("unknown operation");
    }

    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(operation_commands,
	SHELL_CMD(num, NULL,
		"operation",
		example_operations),
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(operation, &operation_commands,
		   "example for operation", NULL);
