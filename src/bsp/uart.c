#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/shell/shell.h>

#include "uart.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(uart, LOG_LEVEL_INF);

static uint8_t uart_rxbuffer[1024] = {0};
static struct ring_buf rx_rb;

static uint8_t uart_txbuffer[1024] = {0};
static struct ring_buf tx_rb;

static struct k_mutex rx_mutex;
static struct k_mutex tx_mutex;

static struct k_timer rx_timer = {0};

#define UART_RX_TIMEOUT 20

static struct k_sem uart_rx_sem = {0};
static uint8_t bsp_rx_buffer[1024] = {0};

static const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(usart1));

static void cb_handler_rx(const struct device *dev)
{
    uint8_t rxbuffer[32] = {0};
    int size             = uart_fifo_read(dev, rxbuffer, sizeof(rxbuffer));

    if(size <= 0)
    {
        LOG_ERR("failed to read: %d", size);
        return;
    }

    ring_buf_put(&rx_rb, rxbuffer, size);
    k_timer_start(&rx_timer, K_MSEC(UART_RX_TIMEOUT), K_NO_WAIT);

    LOG_HEXDUMP_DBG(rxbuffer, size, "uart received");
}

static void cb_handler_tx(const struct device *dev)
{
    uint8_t *txbuffer = NULL;

    if(ring_buf_is_empty(&tx_rb) == true)
    {
        uart_irq_tx_disable(dev);
    }

    uint32_t size = ring_buf_get_claim(&tx_rb, &txbuffer, UINT32_MAX);
    int ret = uart_fifo_fill(dev, txbuffer, size);
    if(ret < 0)
    {
        ring_buf_get_finish(&tx_rb, 0);
    }
    else
    {
        ring_buf_get_finish(&tx_rb, (uint32_t)ret);
    }
}

static void uart_cb_handler(const struct device *dev, void *user_data)
{
    if(uart_irq_update(dev) && uart_irq_is_pending(dev))
    {
        if(uart_irq_rx_ready(dev))
        {
            cb_handler_rx(dev);
        }

        if(uart_irq_tx_ready(dev))
        {
            cb_handler_tx(dev);
        }
    }
}

static void rx_timer_handler(struct k_timer *timer_id)
{
    int ret = ring_buf_get(&rx_rb, bsp_rx_buffer, sizeof(bsp_rx_buffer));
    LOG_INF("rx_timer_handler: got %d bytes from ring buffer", ret);

    k_sem_give(&uart_rx_sem);
}

int bsp_uart_init(void)
{
    k_mutex_init(&rx_mutex);
    k_mutex_init(&tx_mutex);

    k_sem_init(&uart_rx_sem, 0, 1);

    ring_buf_init(&rx_rb, sizeof(uart_rxbuffer), uart_rxbuffer);
    ring_buf_init(&tx_rb, sizeof(uart_txbuffer), uart_txbuffer);
    uart_irq_callback_user_data_set(uart, uart_cb_handler, NULL);
    uart_irq_rx_enable(uart);
    uart_irq_tx_disable(uart);

    k_timer_init(&rx_timer, rx_timer_handler, NULL);

    return 0;
}

int bsp_uart_transmit(uint8_t *txbuffer, size_t size)
{
    uint32_t count = ring_buf_put(&tx_rb, txbuffer, size);

    uart_irq_tx_enable(uart);

    return count;
}

