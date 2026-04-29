#include "bsp_delay.h"
#include "FreeRTOS.h"
#include "bsp_dwt.h"
#include "task.h"
#include "ti_msp_dl_config.h"

// 复用你已有的TIMA0定时器
#define DELAY_TIM_INST TIMA0
#define TIM_16BIT_MAX 0xFFFF

// MSPM0主频80MHz，1us=80个时钟周期
#define US_TO_CYCLES(us) ((us) * 80UL)
#define MS_TO_CYCLES(ms) ((ms) * 80000UL)

// 非阻塞延时用的静态变量
static volatile bool delay_nonblocking_done = false;
static volatile uint32_t delay_target_cycles = 0;
static volatile uint32_t delay_start_cnt = 0;

/**
 * @brief 读取当前定时器计数值（内联函数，最快速度）
 */
static inline uint32_t GetTimCounter(void) {
    return DELAY_TIM_INST->COUNTERREGS.CTR;
}

/**
 * @brief 核心忙等函数（仅内部使用）
 */
static void DelayUs_BusyWait(uint32_t us) {
    if (us == 0)
        return;

    uint32_t total_cycles = US_TO_CYCLES(us);
    uint32_t start_cnt = GetTimCounter();
    uint32_t elapsed_cycles = 0;
    uint32_t last_cnt = start_cnt;

    while (elapsed_cycles < total_cycles) {
        uint32_t current_cnt = GetTimCounter();

        if (current_cnt >= last_cnt) {
            elapsed_cycles += (current_cnt - last_cnt);
        } else {
            elapsed_cycles += ((TIM_16BIT_MAX - last_cnt) + current_cnt + 1);
        }

        last_cnt = current_cnt;
    }
}

/**
 * @brief 微秒级延时（对外接口）
 */
void BspDelay_us(uint32_t us) {
    if (us == 0)
        return;

    // 情况1：延时很短，或者FreeRTOS未启动 → 强制忙等
    if (us <= DELAY_BUSYWAIT_MAX_US || xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        DelayUs_BusyWait(us);
        return;
    }

    // 情况2：FreeRTOS已启动，且延时较长
#if DELAY_ALLOW_BUSYWAIT_IN_FREERTOS
    // 配置允许忙等 → 直接忙等（推荐，保证精度）
    DelayUs_BusyWait(us);
#else
    // 配置不允许忙等 → 用非阻塞延时（精度稍差）
    Bsp_DelayUs_NonBlocking(us);
    while (!Bsp_DelayUs_IsDone()) {
        taskYIELD(); // 释放CPU，让其他任务运行
    }
#endif
}

/**
 * @brief 毫秒级延时（自动适配FreeRTOS）
 */
void BspDelay_ms(uint32_t ms) {
    if (ms == 0)
        return;

    BaseType_t scheduler_state = xTaskGetSchedulerState();

    if (scheduler_state == taskSCHEDULER_NOT_STARTED) {
        // FreeRTOS未启动：用硬件定时器忙等
        uint32_t total_cycles = MS_TO_CYCLES(ms);
        uint32_t start_cnt = GetTimCounter();
        uint32_t elapsed_cycles = 0;
        uint32_t last_cnt = start_cnt;

        while (elapsed_cycles < total_cycles) {
            uint32_t current_cnt = GetTimCounter();

            if (current_cnt >= last_cnt) {
                elapsed_cycles += (current_cnt - last_cnt);
            } else {
                elapsed_cycles += ((TIM_16BIT_MAX - last_cnt) + current_cnt + 1);
            }

            last_cnt = current_cnt;
        }
    } else {
        // FreeRTOS已启动
        if (ms <= 10) {
            // ≤10ms：用忙等（保证精度）
            BspDelay_us(ms * 1000);
        } else {
            // >10ms：用FreeRTOS非阻塞延时
            vTaskDelay(pdMS_TO_TICKS(ms));
        }
    }
}

/**
 * @brief 非阻塞微秒级延时（用硬件定时器比较）
 */
void BspDelay_us_NonBlocking(uint32_t us) {
    delay_nonblocking_done = false;
    delay_start_cnt = GetTimCounter();
    delay_target_cycles = US_TO_CYCLES(us);
}

/**
 * @brief 检查非阻塞延时是否完成
 */
bool BspDelay_us_IsDone(void) {
    if (delay_nonblocking_done) {
        return true;
    }

    uint32_t current_cnt = GetTimCounter();
    uint32_t elapsed_cycles;

    if (current_cnt >= delay_start_cnt) {
        elapsed_cycles = current_cnt - delay_start_cnt;
    } else {
        elapsed_cycles = (TIM_16BIT_MAX - delay_start_cnt) + current_cnt + 1;
    }

    if (elapsed_cycles >= delay_target_cycles) {
        delay_nonblocking_done = true;
        return true;
    }

    return false;
}