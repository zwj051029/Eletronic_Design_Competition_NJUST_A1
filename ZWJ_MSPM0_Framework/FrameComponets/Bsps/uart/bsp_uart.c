#include "bsp_uart.h"
#include "string.h"

/// @brief 记录所有BSP_UART实例，便于管理和查找（static化避免外部调用）
static BspUart_Instance *bspuart_insts[BSPUART_MAX_CANINSTS] = {NULL}; // BSP_UART实例注册表
static uint8_t bspuart_insts_count = 0;                                // 已注册的BSP_UART实例数量

void BspUart_InstRegister(BspUart_Instance *inst, UART_Regs *uart_regs, BspUart_Type rx_type, BspUart_Type tx_type,
                          uint16_t rx_setlen, BspUart_InstRxCallback rx_callback) {
    if (inst == NULL || uart_regs == NULL)
        return;

    inst->uart_regs = uart_regs;
    inst->rx_type = rx_type;
    inst->tx_type = tx_type;
    inst->rx_callback = rx_callback;

    memset(inst->rx_buf, 0, sizeof(inst->rx_buf));
    inst->rx_setlen = rx_setlen;
    inst->rx_len = 0;

    if (bspuart_insts_count < BSPUART_MAX_CANINSTS) {
        for (uint8_t i = 0; i < bspuart_insts_count; i++) {
            if (bspuart_insts[i]->uart_regs == inst->uart_regs) {
                while (1) {
                }
            }
        }

        bspuart_insts[bspuart_insts_count++] = inst;
    }
}

void BspUart_ConfigDMA(BspUart_Instance *inst, DMA_Regs *dma_regs, uint8_t rx_dma_channel_id, uint8_t tx_dma_channel_id,
                       uint32_t src_addr, uint32_t dest_addr) {
    if (inst == NULL || dma_regs == NULL) {
        return;
    }

    inst->dma_regs = dma_regs;
    inst->rx_dma_channel_id = rx_dma_channel_id;
    inst->tx_dma_channel_id = tx_dma_channel_id;
    inst->src_addr = src_addr;
    inst->dest_addr = dest_addr;

    switch (inst->rx_type) {
    case BspUart_Type_Normal:
        // HAL_UART_Receive(inst->huart, inst->rx_buf, inst->rx_setlen, HAL_MAX_DELAY);
        break;
    case BspUart_Type_IT:
        // HAL_UART_Receive_IT(inst->huart, &inst->rx_byte, 1);
        break;
    case BspUart_Type_DMA:
        DL_DMA_setSrcAddr(inst->dma_regs, inst->rx_dma_channel_id, inst->src_addr);
        DL_DMA_setDestAddr(inst->dma_regs, inst->rx_dma_channel_id, inst->dest_addr);
        // DL_DMA_setTransferSize(inst->dma_regs, inst->rx_dma_channel_id, inst->rx_setlen);
        // 一个一个发，注意发十六进制数
        DL_DMA_setTransferSize(inst->dma_regs, inst->rx_dma_channel_id, 1);
        DL_DMA_enableChannel(inst->dma_regs, inst->rx_dma_channel_id);
        break;
    default:
        break;
    }
}

void BspUart_Transmit(BspUart_Instance inst, uint8_t *tx_data, uint16_t tx_len) {
    if (inst.uart_regs == NULL || tx_data == NULL || tx_len == 0)
        return;

    switch (inst.tx_type) {
    case BspUart_Type_Normal:
        break;
    case BspUart_Type_IT:
        break;
    case BspUart_Type_DMA:
        inst.src_addr = (uint32_t) tx_data;
        inst.dest_addr = (uint32_t) (&inst.uart_regs->TXDATA);

        DL_DMA_setSrcAddr(inst.dma_regs, inst.tx_dma_channel_id, inst.src_addr);
        DL_DMA_setDestAddr(inst.dma_regs, inst.tx_dma_channel_id, inst.dest_addr);
        DL_DMA_setTransferSize(inst.dma_regs, inst.tx_dma_channel_id, tx_len);
        DL_DMA_enableChannel(inst.dma_regs, inst.tx_dma_channel_id);
        break;
    default:
        break;
    }
}