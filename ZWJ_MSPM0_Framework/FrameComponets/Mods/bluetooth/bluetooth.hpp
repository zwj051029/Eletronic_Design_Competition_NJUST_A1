#pragma once

#include "ti_msp_dl_config.h"
#include "bsp_uart.h"

class BlueTooth {
private:
    BspUart_Instance uart_inst;

    bool initialize = false;
    bool tx_occupied = false;

public:
    void Init(UART_Regs *uart_regs);

    void SendMsg(uint8_t *data, uint16_t data_len);
};
