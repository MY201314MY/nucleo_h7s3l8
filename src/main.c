/*
 * Copyright (c) 2022 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
commit d02cdc734e6e4ad83960a7b97787729509c848b2 (HEAD -> nucleo_h7s3l8)
Author: Martin Hoff <martin.hoff@silabs.com>
Date:   Wed Nov 26 18:00:32 2025 +0100

    drivers: i2s: siwx91x: ensure device runtime is released
    
    Removed conditional checks for device runtime put in DMA RX and
    TX callbacks, ensuring that device runtime is always released
    asynchronously.
    
    Signed-off-by: Martin Hoff <martin.hoff@silabs.com>
*/
#include <zephyr/kernel.h>
#include <zephyr/linker/linker-defs.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#define LED_DELAY_MS 100

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

int main(void)
{
	LOG_INF("app start address: 0x%p", (void *)__rom_region_start);
	LOG_INF("soc : %s", CONFIG_BOARD_QUALIFIERS);
	LOG_INF("board : %s", CONFIG_BOARD);
	LOG_INF("frequency : %d MHz (Cortex-M7)", sys_clock_hw_cycles_per_sec()/1000000);

	gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);

	while (1) {
		gpio_pin_toggle_dt(&led0);
		k_sleep(K_MSEC(LED_DELAY_MS));
		gpio_pin_toggle_dt(&led0);
		k_sleep(K_MSEC(LED_DELAY_MS));

		gpio_pin_toggle_dt(&led1);
		k_sleep(K_MSEC(LED_DELAY_MS));
		gpio_pin_toggle_dt(&led1);
		k_sleep(K_MSEC(LED_DELAY_MS));

		gpio_pin_toggle_dt(&led2);
		k_sleep(K_MSEC(LED_DELAY_MS));
		gpio_pin_toggle_dt(&led2);
		k_sleep(K_MSEC(LED_DELAY_MS));
	}
	
	return 0;
}
