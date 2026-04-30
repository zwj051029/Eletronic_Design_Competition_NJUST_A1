#include "bsp_delay.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ti_msp_dl_config.h"

/* 80MHz 主频：1 微秒 = 80 个时钟周期 */
#ifndef CPU_FREQ_MHZ
#define CPU_FREQ_MHZ 80
#endif
#define CYCLES_PER_US (CPU_FREQ_MHZ)

/**
 * @brief 微秒级忙等延时（直接使用内核周期延迟指令）
 * @param us 延时的微秒数
 */
void BspDelay_us(uint32_t us) {
    if (us == 0)
        return;

    uint64_t total_cycles = (uint64_t) us * CYCLES_PER_US;

    /* 分次调用 __delay_cycles，避免单次参数超过 32 位上限 */
    while (total_cycles > 0UL) {
        uint32_t cycles = (total_cycles > 0xFFFFFFFFUL) ? 0xFFFFFFFFUL : (uint32_t) total_cycles;
        delay_cycles(cycles);
        total_cycles -= cycles;
    }
}

/**
 * @brief 毫秒级延时（自动适配 FreeRTOS）
 * @param ms 延时的毫秒数
 */
void BspDelay_ms(uint32_t ms) {
    if (ms == 0)
        return;

    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        vTaskDelay(pdMS_TO_TICKS(ms));
    } else {
        BspDelay_us(ms * 1000UL);
    }
}