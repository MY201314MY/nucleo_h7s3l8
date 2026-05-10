#include <stdlib.h>
#include <zephyr/syscalls/time_units.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>
#include <zephyr/sys/util.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(nucleo, LOG_LEVEL_DBG);

static const struct device *const hts221 = DEVICE_DT_GET(DT_NODELABEL(hts221_x_nucleo_iks01a1));
static const struct device *const lsm6ds0 = DEVICE_DT_GET(DT_NODELABEL(lsm6ds0_x_nucleo_iks01a1));

static int example_operations(const struct shell *sh, size_t argc, char *argv[])
{
    int ret = -1;
    int operation = atoi(argv[1]);
    
    LOG_DBG("operation %d selected", operation);

    if(operation == 0)
    {
        struct sensor_value temp = {0}, hum = {0};
        if (!device_is_ready(hts221)) 
        {
		    LOG_ERR("%s: device not ready.", hts221->name);
		    return 0;
	    }

        ret = sensor_sample_fetch(hts221);
        if (ret < 0) {
			LOG_ERR("hts221 sensor featch failed with ret:%d", ret);
			return 0;
		}

        sensor_channel_get(hts221, SENSOR_CHAN_AMBIENT_TEMP, &temp);
		sensor_channel_get(hts221, SENSOR_CHAN_HUMIDITY, &hum);

        LOG_INF("hts221: temperature: %.1f ℃", sensor_value_to_double(&temp));
        LOG_INF("hts221: humidity: %.1f %%", sensor_value_to_double(&hum));
    }
    else if(operation == 1)
    {
        struct sensor_value accel_xyz[3] = {0}, gyro_xyz[3] = {0};
        if (!device_is_ready(lsm6ds0)) 
        {
		    LOG_ERR("%s: device not ready.", lsm6ds0->name);
		    return 0;
	    }

        ret = sensor_sample_fetch(lsm6ds0);
        if (ret < 0) {
			LOG_ERR("lsm6ds0 sensor featch failed with ret:%d", ret);
			return 0;
		}
        sensor_channel_get(lsm6ds0, SENSOR_CHAN_ACCEL_XYZ, accel_xyz);
		sensor_channel_get(lsm6ds0, SENSOR_CHAN_GYRO_XYZ, gyro_xyz);

        LOG_INF("lsm6ds0: accel_x: %.3f g, accel_y: %.3f g, accel_z: %.3f g",
			sensor_value_to_double(&accel_xyz[0]),
			sensor_value_to_double(&accel_xyz[1]),
			sensor_value_to_double(&accel_xyz[2]));
        LOG_INF("lsm6ds0: gyro_x: %.3f dps, gyro_y: %.3f dps, gyro_z: %.3f dps",
			sensor_value_to_double(&gyro_xyz[0]),
			sensor_value_to_double(&gyro_xyz[1]),
			sensor_value_to_double(&gyro_xyz[2]));
    }
    else
    {
        LOG_WRN("unknown operation");
    }

    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(nucleo_commands,
	SHELL_CMD(num, NULL,
		"nucleo_iks01a1",
		example_operations),
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(nucleo, &nucleo_commands,
		   "example for nucleo_iks01a1", NULL);



