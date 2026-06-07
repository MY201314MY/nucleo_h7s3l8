#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(btn, LOG_LEVEL_INF);

#define GPIO_BUTTON_PORT DT_NODELABEL(gpioc)
#define GPIO_BUTTON_PIN  13

#define BUTTON_DETECT_PERIOD         20
#define BUTTON_SHORT_PRESS_TIMEOUT   40
#define BUTTON_LONG_PRESS_TIMEOUT    3000
#define BUTTON_PRESS_TIME_MAX        BUTTON_LONG_PRESS_TIMEOUT
#define BUTTON_SHORT_PRESS_COUNT_MAX 15

typedef struct 
{
	uint8_t status_backup;
	uint32_t count;

	uint8_t change_1:2;
	uint8_t change_2:2;
	uint8_t press_count:4;
} button_status_t;

static button_status_t button[1] = {0};

static int button_hander()
{
	button_status_t *p = button;

	int ret = gpio_pin_get_raw(DEVICE_DT_GET(GPIO_BUTTON_PORT), GPIO_BUTTON_PIN);
	if(ret >= 0)
	{
		if(ret == 0)
		{
			if(!p->count)
			{
				LOG_DBG("button status change");
			}

			if((p->count*BUTTON_DETECT_PERIOD) >= BUTTON_PRESS_TIME_MAX)
			{
				LOG_DBG("cnt stop to increase");
			}
			else
			{
				p->count++;
			}

			if((p->count*BUTTON_DETECT_PERIOD) >= BUTTON_SHORT_PRESS_TIMEOUT)
			{
				/* "change_1 == 1" is enough, but we want to print some logs. */
				if(p->change_1 == 0)
				{
					p->change_1 = 1;
					if(p->press_count < BUTTON_SHORT_PRESS_COUNT_MAX)
					{
						p->press_count++;
					}
					else{
						p->press_count = 0;
					}
				}
				if(p->change_1 == 1)
				{
					p->change_1 = 2;
					p->status_backup = ( ret == 0 ) ? 0 : 1;
					LOG_INF("button status change 1");
				}
			}
			if((p->count*BUTTON_DETECT_PERIOD) >= BUTTON_LONG_PRESS_TIMEOUT)
			{
				if(p->change_2 == 0)
				{
					p->change_2 = 1;
				}

				if(p->change_2 == 1)
				{
					p->change_2 = 2;
					p->status_backup = ( ret == 0 ) ? 0 : 1;
					LOG_INF("button_status_change_2");
				}
			}
		}
		else if(ret == 1){
			p->count = 0;
			p->status_backup = 1;

			if(p->change_2 != 0)
			{
				p->press_count = 0;
				p->change_2 = 0;
				LOG_INF("L:%d -- event : %s", __LINE__, "long press");
			}
			else if(p->change_1 != 0)
			{
				p->change_1 = 0;
				LOG_INF("L:%d -- event : %s", __LINE__, "short press");
				LOG_INF("count:%d", p->press_count);
			}

			p->change_1 = 0;
			p->change_2 = 0;
		}
	}

	return 0;

}

int bsp_button_thread_entry(void)
{
	int ret = 0;
	ret = gpio_pin_configure(DEVICE_DT_GET(GPIO_BUTTON_PORT), GPIO_BUTTON_PIN, GPIO_INPUT | GPIO_PULL_UP);
	LOG_INF("button init : %d", ret);

    while(1)
	{
		button_hander();
		ret = gpio_pin_get_raw(DEVICE_DT_GET(GPIO_BUTTON_PORT), GPIO_BUTTON_PIN);
		LOG_DBG("ret:%d", ret);
		k_sleep(K_MSEC(BUTTON_DETECT_PERIOD));
	}

	return 0;
}

K_THREAD_STACK_DEFINE(b_thread_stack, 4096);
static struct k_thread b_thread;
static k_tid_t b_thread_id = NULL;

int bsp_button_init()
{
	if(b_thread_id != NULL)
	{
		LOG_WRN("thread has been created.");
		return 0;
	}
    b_thread_id = k_thread_create(&b_thread,
										b_thread_stack,
                     					4096,
                     					(k_thread_entry_t)bsp_button_thread_entry,
                     					NULL, NULL, NULL,
                     					K_PRIO_PREEMPT( 8 ), 0, K_NO_WAIT );

	if( b_thread_id != NULL )
  	{
    	k_thread_name_set(b_thread_id, "button");
  	}
  	else
  	{
		LOG_ERR("button thread create failed.");
  	}

	return 0;
}



