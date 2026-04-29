#ifndef __BSP_DELAY_H__
#define __BSP_DELAY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"
#include "stdint.h"

/******************************************************************************
 * 配置选项
 *****************************************************************************/
// 是否在FreeRTOS运行时允许短时间忙等（≤100μs）
// 1 = 允许（推荐，保证超声波传感器精度）
// 0 = 不允许（用硬件定时器中断，精度稍差但不占用CPU）
#define DELAY_ALLOW_BUSYWAIT_IN_FREERTOS 1

// 短时间忙等的最大阈值（单位：μs）
#define DELAY_BUSYWAIT_MAX_US 100

/******************************************************************************
 * 函数声明
 *****************************************************************************/
/**
 * @brief 微秒级延时（核心函数）
 * @param us: 延时时间，单位微秒
 * @note - ≤100μs：强制忙等（保证精度）
 *       - >100μs：根据配置选择
 */
void BspDelay_us(uint32_t us);

/**
 * @brief 毫秒级延时（自动适配FreeRTOS）
 * @param ms: 延时时间，单位毫秒
 */
void BspDelay_ms(uint32_t ms);

/**
 * @brief 非阻塞微秒级延时（用硬件定时器中断，可选）
 * @param us: 延时时间，单位微秒
 * @note 需要在SysConfig里开启TIMA0的比较中断
 */
void BspDelay_us_NonBlocking(uint32_t us);

/**
 * @brief 检查非阻塞延时是否完成
 * @return true: 延时完成，false: 延时中
 */
bool BspDelay_us_IsDone(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_DELAY_H__ */