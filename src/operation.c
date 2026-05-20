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
#include <zephyr/drivers/entropy.h>
#include <stm32h7rsxx.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(operation, LOG_LEVEL_DBG);

#define GPIO_BUTTON_PORT DT_NODELABEL(gpioc)
#define GPIO_BUTTON_PIN  13

static int example_operations(const struct shell *sh, size_t argc, char *argv[])
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
    else if(operation == 2) 
    {
        if (__HAL_PWR_GET_FLAG(PWR_FLAG_SBF) != RESET)
        {
            __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SBF);
            if (__HAL_PWR_GET_FLAG(PWR_WAKEUP_FLAG3) != RESET)
            {
                __HAL_PWR_CLEAR_FLAG(PWR_WAKEUP_FLAG3);
            }
        }

        {
            PWREx_WakeupPinTypeDef PinParams = {0};

            HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN3);
            __HAL_PWR_CLEAR_FLAG(PWR_WAKEUP_FLAG3);

            PinParams.WakeUpPin    = PWR_WAKEUP_PIN3;
            PinParams.PinPolarity  = PWR_PIN_POLARITY_LOW;
            PinParams.PinPull      = PWR_PIN_PULL_UP;
            HAL_PWREx_EnableWakeUpPin(&PinParams);

            HAL_PWR_EnterSTANDBYMode();
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
                uint64_t size_bytes = 1ULL << (size_code + 1);
                uint8_t xn = (uint8_t)((rasr & MPU_RASR_XN_Msk) >> MPU_RASR_XN_Pos);
                uint8_t ap = (uint8_t)((rasr & MPU_RASR_AP_Msk) >> MPU_RASR_AP_Pos);

                if(size_code == 31)
                {
                    continue;
                }

                LOG_INF("region %02d: addr: 0x%08X --- xn : %d --- ap : %d --- size: 0x%08llX bytes", i, addr, xn, ap, size_bytes);
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
        ret = gpio_pin_configure(DEVICE_DT_GET(GPIO_BUTTON_PORT), GPIO_BUTTON_PIN, GPIO_INPUT | GPIO_PULL_UP);
        ret = gpio_pin_get_raw(DEVICE_DT_GET(GPIO_BUTTON_PORT), GPIO_BUTTON_PIN);
        LOG_INF("pin status : %d", ret);
    }
    else if(operation == 8)
    {
        LOG_INF("HAL tick : %d", HAL_GetTick());
    }
    else if(operation == 9)
    {
        extern struct k_heap _system_heap;
        struct sys_memory_stats stats;

        sys_heap_runtime_stats_get(&_system_heap.heap, &stats);
        LOG_INF("heap allocated=%d, free=%d, max allocated:%d", stats.allocated_bytes, stats.free_bytes, stats.max_allocated_bytes);
    }
    else if(operation == 10)
    {
        const struct device *const vbat_dev = DEVICE_DT_GET(DT_NODELABEL(vbat));
        struct sensor_value voltage;

        if (!device_is_ready(vbat_dev)) {
            LOG_ERR("device not ready.");
            return -ENODEV;
        }
        ret = sensor_sample_fetch(vbat_dev);
        if (ret == 0) {
            sensor_channel_get(vbat_dev, SENSOR_CHAN_VOLTAGE, &voltage);
            LOG_INF("voltage of battery : %d.%06d V", voltage.val1, voltage.val2);
        } else {
            LOG_ERR("sensor_sample_fetch failed with ret : %d", ret);
        }
    }
    else if(operation == 11)
    {
        const struct device *const vref_dev = DEVICE_DT_GET(DT_NODELABEL(vref));
        struct sensor_value voltage;

        if (!device_is_ready(vref_dev)) {
            LOG_ERR("device not ready.");
            return -ENODEV;
        }
        ret = sensor_sample_fetch(vref_dev);
        if (ret == 0) {
            sensor_channel_get(vref_dev, SENSOR_CHAN_VOLTAGE, &voltage);
            LOG_INF("voltage of reference : %d.%06d V", voltage.val1, voltage.val2);
        } else {
            LOG_ERR("sensor_sample_fetch failed with ret : %d", ret);
        }
    }
    else if(operation == 12)
    {
        const struct device *rng_dev = DEVICE_DT_GET(DT_NODELABEL(rng));
        uint8_t random[32] = {0};

        ret = entropy_get_entropy(rng_dev, random, sizeof(random));
        if(ret == 0)
        {
            LOG_HEXDUMP_INF(random, sizeof(random), "random");
        }

    }
    else if(operation == 13)
    {
        if(__HAL_PWR_GET_FLAG(PWR_FLAG_SBF) != RESET)
        {
            __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SBF);
            LOG_INF("reset from : %s", "standby");
            if (__HAL_PWR_GET_FLAG(PWR_WAKEUP_FLAG3) != RESET)
            {
                __HAL_PWR_CLEAR_FLAG(PWR_WAKEUP_FLAG3);
                LOG_INF("wake up by pin : %d", 3);
            }
        }
        else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST) != RESET)
        {
            LOG_INF("reset from : %s", "power on");
            __HAL_RCC_CLEAR_RESET_FLAGS();
        }
        else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) != RESET)
        {
            LOG_INF("reset from : %s", "hard reset");
            __HAL_RCC_CLEAR_RESET_FLAGS();
        }
    }
    else if(operation == 14)
    {
        RTC_HandleTypeDef hrtc;
        if(__HAL_PWR_GET_FLAG(PWR_FLAG_SBF) != RESET)
        {
            __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SBF);
        }
        HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
        __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF_ALL);

        if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 0x10, RTC_WAKEUPCLOCK_CK_SPRE_16BITS, 0) != HAL_OK)
        {
            LOG_ERR("L:%d", __LINE__);
        }
        HAL_PWR_EnterSTANDBYMode();
    }
    else {
        LOG_WRN("unknown operation");
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

