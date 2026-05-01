#include "bluetooth.hpp"

void BlueTooth::Init(UART_Regs *uart_regs) {
    if (uart_regs == NULL) {
        return;
    }

    BspUart_InstRegister(&this->uart_inst, uart_regs, BspUart_Type_DMA, BspUart_Type_DMA, 64, NULL);
    BspUart_ConfigDMA(&this->uart_inst, DMA, BlueTooth_RX_DMA_CH_CHAN_ID, BlueTooth_TX_DMA_CH_CHAN_ID);
    this->initialize = true;
}

void BlueTooth::SendMsg(uint8_t *data, uint16_t data_len) {
    if (!initialize || data == NULL || data_len == 0 || tx_occupied) {
        return;
    }

    this->tx_occupied = true;
    BspUart_Transmit(uart_inst, data, data_len);
    this->tx_occupied = false;
}

void BlueTooth_INST_IRQHandler(void) {
    switch (DL_UART_Main_getPendingInterrupt(BlueTooth_INST)) {
    // DMA搬运完成
    case DL_UART_MAIN_IIDX_DMA_DONE_TX:
        break;
    default:
        break;
    }
}
