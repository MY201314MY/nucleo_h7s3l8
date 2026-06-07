/*
 * Copyright (c) 2022 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/linker/linker-defs.h>
#include <zephyr/drivers/gpio.h>

#include "bsp/button.h"
#include "bsp/uart.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);


#define LED_DELAY_MS 100

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

__stm32_backup_sram_section int backup_boot_count;

int main(void)
{
	backup_boot_count++;
	
	LOG_INF("app on: %p", (void *)__rom_region_start);
	LOG_INF("arch : %s", CONFIG_ARCH);
	LOG_INF("soc : %s", CONFIG_BOARD_QUALIFIERS);
	LOG_INF("board : %s", CONFIG_BOARD);
	LOG_INF("frequency : %d MHz (Cortex-M7)", sys_clock_hw_cycles_per_sec()/1000000);
	LOG_INF("boot count : %d", backup_boot_count);

	gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);

	bsp_button_init();
	bsp_uart_init();
	int bsp_imu_init(void);
	bsp_imu_init();

	while(1) 
	{
		gpio_pin_toggle_dt(&led0);
		k_sleep(K_MSEC(LED_DELAY_MS));
	}
	
	return 0;
}
