#include <stdlib.h>
#include <zephyr/syscalls/time_units.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/can.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/rtc.h>
#include <zephyr/arch/arm/mpu/arm_mpu.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/sys/sys_heap.h>
#include <stm32h7rsxx.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(operation, LOG_LEVEL_DBG);

int example_operations(const struct shell *sh, size_t argc, char *argv[])
{
    int ret = -1;
    int operation = atoi(argv[1]);
    
    LOG_DBG("operation %d selected", operation);

    if(operation == 0) {
        LOG_WRN("reboot at once.");
        LOG_PANIC();
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

        ret = spi_transceive_dt(&spim, &tx_spi_buf_set, &rx_spi_buf_set);

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
        ret = rtc_get_time(dev, &time);
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
        uint32_t type = MPU->TYPE;
        uint8_t region_count = (uint8_t)((type & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos);
        
        LOG_INF("--- Hard-Reading MPU Registers (%d slots) ---", region_count);
        for (uint8_t i = 0; i < region_count; i++) {
            MPU->RNR = i;

            uint32_t rbar = MPU->RBAR;
            uint32_t rasr = MPU->RASR;

            if (rasr & MPU_RASR_ENABLE_Msk) {
                uint32_t addr = rbar & MPU_RBAR_ADDR_Msk;
                
                uint8_t size_code = (uint8_t)((rasr & MPU_RASR_SIZE_Msk) >> MPU_RASR_SIZE_Pos);
                uint32_t size_bytes = 1U << (size_code + 1);

                if (size_code == 31) 
                {
                    LOG_INF("region %d: addr: 0x%08X | size: 4GB", i, addr);
                }
                else
                {
                    LOG_INF("region %d: addr: 0x%08X | size: 0x%08X bytes", i, addr, size_bytes);
                }
            }
        }
    }
    else if(operation == 6)
    {
        LOG_INF("num regions : %d", mpu_config.num_regions);
        for(uint8_t i=0;i<mpu_config.num_regions;i++)
        {
            LOG_INF("address : 0x%08X", mpu_config.mpu_regions[i].base);
        }
    }
    else if(operation == 7)
    {

    }
    else if(operation == 8)
    {
        LOG_INF("HAL tick : %d", HAL_GetTick(   ));
    }
    else if(operation == 9)
    {
        extern struct k_heap _system_heap;
        struct sys_memory_stats stats;

        sys_heap_runtime_stats_get(&_system_heap.heap, &stats);
        LOG_INF("heap allocated=%d, free=%d, max allocated:%d", stats.allocated_bytes, stats.free_bytes, stats.max_allocated_bytes);
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

