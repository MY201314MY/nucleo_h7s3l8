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
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_core.h>
#include <zephyr/net/net_context.h>
#include <zephyr/net/net_mgmt.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#define LED_DELAY_MS 100

static struct net_mgmt_event_callback net_mgmt_cb;
static struct net_mgmt_event_callback iface_mgmt_cb;

__stm32_backup_sram_section int backup_boot_count;

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static void dhcpv4_client(struct net_if *iface, void *user_data)
{
	ARG_UNUSED(user_data);

	LOG_INF("dhcpv4 on %s: index=%d", net_if_get_device(iface)->name,
		net_if_get_by_iface(iface));
	net_dhcpv4_start(iface);
}

static void net_handler(struct net_mgmt_event_callback *cb,
		    uint64_t mgmt_event,
		    struct net_if *iface)
{
	int i = 0;

	if (mgmt_event == NET_EVENT_IPV4_ADDR_ADD) {
		LOG_INF("net ipv4 added.");
		for (i = 0; i < NET_IF_MAX_IPV4_ADDR; i++) {
			char buf[NET_IPV4_ADDR_LEN];

			if (iface->config.ip.ipv4->unicast[i].ipv4.addr_type !=
								NET_ADDR_DHCP) {
				continue;
			}

			LOG_INF("IP[%d]: %s", net_if_get_by_iface(iface),
				net_addr_ntop(NET_AF_INET,
					&iface->config.ip.ipv4->unicast[i].ipv4.address.in_addr,
							buf, sizeof(buf)));
			LOG_INF("mask[%d]: %s", net_if_get_by_iface(iface),
				net_addr_ntop(NET_AF_INET,
						&iface->config.ip.ipv4->unicast[i].netmask,
						buf, sizeof(buf)));
			LOG_INF("router[%d]: %s", net_if_get_by_iface(iface),
				net_addr_ntop(NET_AF_INET,
							&iface->config.ip.ipv4->gw,
							buf, sizeof(buf)));
			LOG_INF("lease time[%d]: %u seconds", net_if_get_by_iface(iface),
				iface->config.dhcpv4.lease_time);
		}
	}
	else if (mgmt_event == NET_EVENT_IPV4_ADDR_DEL)
	{
		LOG_INF("net ipv4 deleted.");
	}
	else if(mgmt_event == NET_EVENT_IF_UP)
	{
		LOG_INF("net if up.");
	}
	else if(mgmt_event == NET_EVENT_IF_DOWN)
	{
		LOG_INF("net if down.");
	}
}

static void iface_handler(struct net_mgmt_event_callback *cb,
		    uint64_t mgmt_event,
		    struct net_if *iface)
{
	if(mgmt_event == NET_EVENT_IF_UP)
	{
		LOG_INF("net if up.");
	}
	else if(mgmt_event == NET_EVENT_IF_DOWN)
	{
		LOG_INF("net if down.");
	}
}

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
	net_mgmt_init_event_callback(&net_mgmt_cb, net_handler,
				     NET_EVENT_IPV4_ADDR_ADD | NET_EVENT_IPV4_ADDR_DEL);
	net_mgmt_add_event_callback(&net_mgmt_cb);
	
	net_mgmt_init_event_callback(&iface_mgmt_cb, iface_handler,
				     NET_EVENT_IF_UP | NET_EVENT_IF_DOWN);
	net_mgmt_add_event_callback(&iface_mgmt_cb);

	net_if_foreach(dhcpv4_client, NULL);

	while (1) {
		gpio_pin_toggle_dt(&led0);
		k_sleep(K_MSEC(LED_DELAY_MS));
		gpio_pin_toggle_dt(&led0);
		k_sleep(K_MSEC(LED_DELAY_MS));
	}
	
	return 0;
}
