/*
 * Copyright (c) 2022 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
commit bbff45f1c4cf7d3baa068294a30fc490d692bdb4 (HEAD -> main, origin/main, origin/HEAD)
Author: Kyle Bonnici <kylebonnici@hotmail.com>
Date:   Thu Dec 11 15:06:39 2025 +0100

    DTS: format files using dts-linter 0.3.7-hotfix2
    
    - Ensure that properties have 2 new lines when node is above it.
    - Enures that 1 new line is required between a node and #if/#ifdef...
    - Enures that 2 new line are required between #endif and node.
    - Wraps property values that exceed 100 characters in length.
    
    Signed-off-by: Kyle Bonnici <kylebonnici@hotmail.com>
*/
#include <zephyr/kernel.h>
#include <zephyr/linker/linker-defs.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#define LED_DELAY_MS 100

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

int main(void)
{
	//backup_boot_count++;
	
	LOG_INF("app start address: 0x%p", (void *)__rom_region_start);
	LOG_INF(" arch : %s", CONFIG_ARCH);
	LOG_INF("  soc : %s", CONFIG_BOARD_QUALIFIERS);
	LOG_INF("board : %s", CONFIG_BOARD);
	LOG_INF("frequency : %d MHz (Cortex-M7)", sys_clock_hw_cycles_per_sec()/1000000);
	//LOG_INF("boot count : %d", backup_boot_count);

	gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);

	while (1) {
		gpio_pin_toggle_dt(&led0);
		k_sleep(K_MSEC(LED_DELAY_MS));
		gpio_pin_toggle_dt(&led0);
		k_sleep(K_MSEC(LED_DELAY_MS));
	}
	
	return 0;
}
