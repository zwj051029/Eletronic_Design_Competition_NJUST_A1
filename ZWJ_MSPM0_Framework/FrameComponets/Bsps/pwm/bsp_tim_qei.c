#include "bsp_tim_qei.h"

/**
 * @brief  注册QEI实例
 * @param  qei             QEI实例
 * @param  htim            定时器句柄
 * @param  encoder_lines   编码器线数，单相每圈脉冲数
 * @param  gear_ratio      减速比
 */
void BspTimerQEI_InstRegist(BspTIMQEI_TypeDef *qei,
                           GPTIMER_Regs *htim,
                           uint16_t encoder_lines,
                           uint16_t gear_ratio)
{
    // 检验参数有效性
    if (qei == NULL || htim == NULL)
        return;

    // 赋值：定时器、编码器线数、减速比
    qei->htim = htim;
    qei->encoder_lines = encoder_lines;
    qei->gear_ratio = gear_ratio;
    
    qei->count = 0;
    qei->last_count = 0;
    qei->speed = 0.0f;
}

/**
 * @brief 获取QEI计数值
 * @param qei      QEI实例 
 * @return int32_t QEI计数值
 */
int32_t BspTimerQEI_GetCount(BspTIMQEI_TypeDef *qei)
{
    if (qei == NULL || qei->htim == NULL)
        return 0;

    return (int32_t)DL_Timer_getTimerCount(qei->htim);
}

/**
 * @brief 清零QEI计数值
 * @param qei       QEI实例
 */
void BspTimerQEI_Reset(BspTIMQEI_TypeDef *qei)
{
    if (qei == NULL || qei->htim == NULL)
        return;

    DL_Timer_setTimerCount(qei->htim, 0);

    qei->count = 0;
    qei->last_count = 0;
}

/**
 * @brief 
 * @param qei QEI实例
 * @param dt  M法计数值
 * @return float 当前转速，单位为 r/min
 * @note 
 */
float BspTimerQEI_UpdateSpeed(BspTIMQEI_TypeDef *qei, float dt )
{
    if (qei == NULL || qei->htim == NULL || dt <= 0)
        return 0.0f;

    int32_t now = (int32_t)DL_Timer_getTimerCount(qei->htim);

    // 间隔值自动处理溢出
    int32_t delta = now - qei->last_count;
    // 数据更新
    qei->last_count = now;

    // 0保护
    if (qei->encoder_lines == 0 || qei->gear_ratio == 0)
        return 0.0f;
    
    // 使用M法测量速度，单位为 r/min，其公式为：
    // 输出轴转速RPM= (这次读到的脉冲数−上次读到的脉冲数) / (编码器线数×4×减速比) / 间隔时间 (秒)×60
    // X4模式 → 每圈脉冲 = encoder_lines × 4
    float rev = (float)delta / (qei->encoder_lines * 4 * qei->gear_ratio);
    
    // 输出轴转速RPM
    qei->speed = rev / dt * 60.0f;

    return qei->speed;
}

/**
 * @brief 使能QEI并且启动
 * @param qei QEI实例
 */
void BspTimerQEI_Enable(BspTIMQEI_TypeDef *qei)
{
    if (qei == NULL || qei->htim == NULL) 
        return;

    if (!qei->enabled) {
        //开始计数
        DL_Timer_startCounter(qei->htim);
        qei->enabled = 1;
    }
}

/**
 * @brief 使能QEI并且清除计数值
 * @param qei QEI实例
 */
void BspTimerQEI_Disable(BspTIMQEI_TypeDef *qei)
{
    if (qei == NULL || qei->htim == NULL) 
        return;

    if (qei->enabled) {
        // 停止计数
        DL_Timer_stopCounter(qei->htim);
        qei->enabled = 0;

        // 清零相关状态
        qei->last_count = 0;
        qei->speed = 0.0f;
    }
}