#ifndef M0_COMM_IRQ_H
#define M0_COMM_IRQ_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 供 C 中断服务调用，将接收到的字节传入 M0Comm 解析
void M0Comm_ProcessByte(uint8_t byte);

#ifdef __cplusplus
}
#endif

#endif