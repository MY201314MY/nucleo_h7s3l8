#ifndef _SRC_BSP_UART_H_
#define _SRC_BSP_UART_H_

int bsp_uart_init(void);
int bsp_uart_transmit(uint8_t *txbuffer, size_t size);

#endif
