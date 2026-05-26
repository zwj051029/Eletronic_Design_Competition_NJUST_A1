#include "M0_Comm.hpp"
#include "ti_msp_dl_config.h" // MSPM0 DriverLib 头文件
#include <cstring>

M0Comm m0_comm;

void M0Comm::Init() {
    // 使能 UART1 接收中断（RX Not Empty）
    DL_UART_enableInterrupt(UART_1_INST, DL_UART_INTERRUPT_RX);
}

void M0Comm::ProcessRxByte(uint8_t byte) {
    if (state_ == WAIT_HEADER) {
        if (byte == 0xBB) {
            state_ = READING;
            buf_idx_ = 0;
        }
        return;
    }

    frame_buf_[buf_idx_++] = byte;
    if (buf_idx_ >= 9) {
        // 校验：整个帧（包括帧头）异或结果应为 0
        uint8_t calc = 0xBB;
        for (int i = 0; i < 9; i++) {
            calc ^= frame_buf_[i];
        }
        if (calc == 0) {
            // 小端字节序提取 float
            memcpy((void *) &target_left_, &frame_buf_[0], 4);  // 左速度
            memcpy((void *) &target_right_, &frame_buf_[4], 4); // 右速度
            new_frame_ = true;
        }
        state_ = WAIT_HEADER;
    }
}

// C 兼容函数，供中断调用
extern "C" void M0Comm_ProcessByte(uint8_t byte) {
    m0_comm.ProcessRxByte(byte);
}