#ifndef __BSP_UART_H__
#define __BSP_UART_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ti_msp_dl_config.h"
#include <stdbool.h>
#include <stdint.h>

typedef void (*BspUart_InstRxCallback)(uint32_t uart_base, uint8_t *rx_data, uint16_t rx_size);

typedef enum {
    BspUart_Type_Normal,
    BspUart_Type_IT,
    BspUart_Type_DMA,
} BspUart_Type;

typedef struct {
    UART_Regs *uart_regs;
    BspUart_Type rx_type;               // 接收模式
    BspUart_Type tx_type;               // 发送模式
    BspUart_InstRxCallback rx_callback; // 接收回调函数

    // 接收相关
    uint8_t rx_buf[64]; // 接收缓冲区
    uint8_t rx_byte;    // 中断模式单次接收字节
    uint16_t rx_setlen; // 期望接收长度
    uint16_t rx_len;    // 实际接收长度

    // DMA 相关
    DMA_Regs *dma_regs;        // 一般直接填 DMA
    uint8_t rx_dma_channel_id; // 通道ID
    uint8_t tx_dma_channel_id; // 通道ID
    uint32_t src_addr;         // 源地址
    uint32_t dest_addr;        // 目标地址
} BspUart_Instance;

void BspUart_InstRegister(BspUart_Instance *inst, UART_Regs *uart_regs, BspUart_Type rx_type, BspUart_Type tx_type,
                          uint16_t rx_setlen, BspUart_InstRxCallback rx_callback);

void BspUart_ConfigDMA(BspUart_Instance *inst, DMA_Regs *dma_regs, uint8_t rx_dma_channel_id,
                       uint8_t tx_dma_channel_id);

void BspUart_Transmit(BspUart_Instance inst, uint8_t *tx_data, uint16_t tx_len);

#ifdef __cplusplus
}
#endif

#endif
