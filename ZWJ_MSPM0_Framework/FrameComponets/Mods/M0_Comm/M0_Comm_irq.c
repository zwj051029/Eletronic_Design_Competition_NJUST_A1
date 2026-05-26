#include "M0_Comm_Irq.h"
#include "ti_msp_dl_config.h" // 如果需要 UART_1_INST 寄存器

// 直接操作寄存器以避免 DL 库宏兼容问题
void UART1_IRQHandler(void) {
    uint8_t data = DL_UART_receiveData(UART_1_INST);
    // 调用 C 兼容函数，传入字节解析
    M0Comm_ProcessByte(data);

    // 以下为可选：清除中断挂起（一般读取后自动清除，但某些 MCU 需要手动清）
    // NVIC->ICPR[0] = (1 << (UART1_IRQn % 32));
}