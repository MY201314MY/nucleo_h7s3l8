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

