#ifndef __BSP_TIMER_QEI_H__
#define __BSP_TIMER_QEI_H__

#ifdef __cplusplus
extern "C" 
{
#endif

#include <stdint.h>
#include "ti_msp_dl_config.h"
#include <stddef.h>

    // QEI实例结构体
    typedef struct
    {
        GPTIMER_Regs *htim;     // QEI定时器句柄
        int64_t count;          // 当前计数
        int64_t last_count;     // 上一次计数
        float speed;            // 当前速度
        uint16_t encoder_lines; // 编码器线数
        uint16_t gear_ratio;    // 减速比
        uint8_t enabled;        // QEI使能标志
    } BspTIMQEI_TypeDef;

    ///@brief  注册QEI
    void BspTimerQEI_InstRegist(BspTIMQEI_TypeDef *qei,
                                GPTIMER_Regs *htim,
                                uint16_t encoder_lines,
                                uint16_t gear_ratio);

    ///@brief 获取计数
    int32_t BspTimerQEI_GetCount(BspTIMQEI_TypeDef *qei);

    ///@brief 清零
    void BspTimerQEI_Reset(BspTIMQEI_TypeDef *qei);

    ///@brief 更新速度
    float BspTimerQEI_UpdateSpeed(BspTIMQEI_TypeDef *qei, float dt);

    // 使能 QEI（启动定时器计数）
    void BspTimerQEI_Enable(BspTIMQEI_TypeDef *qei);

    ///@brief 失能 QEI（停止定时器计数，并清除状态）
    void BspTimerQEI_Disable(BspTIMQEI_TypeDef *qei);

#ifdef __cplusplus
}
#endif

#endif