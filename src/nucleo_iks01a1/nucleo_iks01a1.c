#include <stdlib.h>
#include <zephyr/syscalls/time_units.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>
#include <zephyr/sys/util.h>

#include "../bsp/uart.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(nucleo, LOG_LEVEL_INF);

#define IMU_FETCH_TIMEOUT 10

static const struct device *const hts221 = DEVICE_DT_GET(DT_NODELABEL(hts221_x_nucleo_iks01a1));
static struct k_timer tx_timer = {0};
static struct k_sem tx_sem = {0};

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
    else
    {
        LOG_WRN("unknown operation");
    }

    return 0;
}

static int bsp_imu_thread_entry(void)
{
    uint8_t buffer[512] = {0};
    const struct device *const imu = DEVICE_DT_GET(DT_NODELABEL(lsm6ds0_x_nucleo_iks01a1));
    struct sensor_value accel_xyz[3] = {0}, gyro_xyz[3] = {0};
    int ret = 0;

    if (!device_is_ready(imu)) 
    {
        LOG_ERR("%s: device not ready.", imu->name);
        return 0;
    }

    k_timer_start(&tx_timer, K_MSEC(IMU_FETCH_TIMEOUT), K_NO_WAIT);
    while(1)
	{
        k_sem_take(&tx_sem, K_FOREVER);
        ret = sensor_sample_fetch(imu);
        if (ret < 0) {
			LOG_ERR("lsm6ds0 sensor featch failed with ret:%d", ret);
		}

        sensor_channel_get(imu, SENSOR_CHAN_ACCEL_XYZ, accel_xyz);
		sensor_channel_get(imu, SENSOR_CHAN_GYRO_XYZ, gyro_xyz);
        sprintf(buffer, "%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf\r\n", ((double)k_uptime_get())/1000,
            sensor_value_to_double(&accel_xyz[0]),
			sensor_value_to_double(&accel_xyz[1]),
			sensor_value_to_double(&accel_xyz[2]),

            sensor_value_to_double(&gyro_xyz[0]),
			sensor_value_to_double(&gyro_xyz[1]),
			sensor_value_to_double(&gyro_xyz[2]));

        bsp_uart_transmit(buffer, strlen(buffer));

        LOG_DBG("lsm6ds0: accel_x: %.3f g, accel_y: %.3f g, accel_z: %.3f g",
			sensor_value_to_double(&accel_xyz[0]),
			sensor_value_to_double(&accel_xyz[1]),
			sensor_value_to_double(&accel_xyz[2]));
        LOG_DBG("lsm6ds0: gyro_x: %.3f dps, gyro_y: %.3f dps, gyro_z: %.3f dps",
			sensor_value_to_double(&gyro_xyz[0]),
			sensor_value_to_double(&gyro_xyz[1]),
			sensor_value_to_double(&gyro_xyz[2]));
	}

	return 0;
}

K_THREAD_STACK_DEFINE(i_thread_stack, 4096);
static struct k_thread i_thread;
static k_tid_t i_thread_id = NULL;

static void tx_timer_handler(struct k_timer *timer_id)
{
    k_sem_give(&tx_sem);
    k_timer_start(&tx_timer, K_MSEC(IMU_FETCH_TIMEOUT), K_NO_WAIT);
}

int bsp_imu_init(void)
{
    k_sem_init(&tx_sem, 0, 1);
    k_timer_init(&tx_timer, tx_timer_handler, NULL);

	if(i_thread_id != NULL)
	{
		LOG_WRN("thread has been created.");
		return 0;
	}
    i_thread_id = k_thread_create(&i_thread,
										i_thread_stack,
                     					4096,
                     					(k_thread_entry_t)bsp_imu_thread_entry,
                     					NULL, NULL, NULL,
                     					K_PRIO_PREEMPT(9), 0, K_NO_WAIT);

	if(i_thread_id != NULL)
  	{
    	k_thread_name_set(i_thread_id, "imu");
  	}
  	else
  	{
		LOG_ERR("imu thread create failed.");
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



