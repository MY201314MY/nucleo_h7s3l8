#include <stdlib.h>
#include <zephyr/syscalls/time_units.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/can.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/rtc.h>

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

    }
    else if(operation == 2) {
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
    }
    else if(operation == 3)
    {
        const struct spi_dt_spec spim = SPI_DT_SPEC_GET(DT_NODELABEL(spim_dt), SPI_OP_MODE_MASTER | SPI_WORD_SET(8));
        uint8_t tx_buffer[1] = {0x9F};
        uint8_t rx_buffer[4] = {0x00, 0x00, 0x00, 0x00};

        struct spi_buf tx_spi_bufs = {.buf = tx_buffer, .len = sizeof(tx_buffer)};
        struct spi_buf rx_spi_bufs = {.buf = rx_buffer, .len = sizeof(rx_buffer)};
        struct spi_buf_set tx_spi_buf_set = {.buffers = &tx_spi_bufs, .count = 1};
        struct spi_buf_set rx_spi_buf_set = {.buffers = &rx_spi_bufs, .count = 1};

        bool status = spi_is_ready_dt(&spim);
        if(status != true)
        {
            LOG_ERR("spi device is not ready");
        }

        int ret = spi_transceive_dt(&spim, &tx_spi_buf_set, &rx_spi_buf_set);

        if (ret < 0) {
            LOG_ERR("spi_transceive returned %d", ret);
            return ret;
        }
        ret = sizeof(tx_buffer);
        LOG_HEXDUMP_INF(((uint8_t *)rx_spi_bufs.buf) + 1, rx_spi_bufs.len - 1, "w25q64fv id");
    }
    else if(operation == 4)
    {
        const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(rtc));
        struct rtc_time time;
        int ret = rtc_get_time(dev, &time);
        if (ret < 0) {
            LOG_ERR("rtc_get_time failed: %d", ret);
            return ret;
        }

        LOG_INF("time : %04d-%02d-%02d %02d:%02d:%02d.%03d",
            time.tm_year+1900, time.tm_mon+1, time.tm_mday,
            time.tm_hour, time.tm_min, time.tm_sec, time.tm_nsec/USEC_PER_SEC);
    }
    else if(operation == 5)
    {
        
    }
    else {
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
