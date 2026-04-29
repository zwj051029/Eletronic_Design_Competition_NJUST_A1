#ifndef __BSP_TIM_PWM_H__
#define __BSP_TIM_PWM_H__

#ifdef __cplusplus
extern "C"
{
#endif

//#include "stm32f4xx_hal.h"
//#include "tim.h"
#include <stdint.h>
#include "ti/devices/msp/msp.h"


    // PWM实例结构体
    typedef struct BspTIMPWM_t
    {
        GPTIMER_Regs *htim;      // PWM定时器句柄
        uint32_t channel;        // PWM通道
        float freq;              // PWM频率
        uint8_t enabled;         // PWM使能标志

        uint32_t auto_reload_value; // 自动重装载寄存器的值（ARR）
        uint32_t compare_value;     // 比较寄存器的值（CCR）

        float duty;                                    // PWM占空比，duty = CCR / ARR
        float (*GetFreq)(struct BspTIMPWM_t pwm_inst); // 获取PWM频率的函数指针
    } BspTIMPWM_TypeDef;

    /// @brief 注册PWM实例
    void BspTIMPWM_InstRegist(BspTIMPWM_TypeDef *pwm_inst, GPTIMER_Regs *htim, uint32_t channel);

    /// @brief 设置PWM占空比
    void BspTIMPWM_SetDuty(BspTIMPWM_TypeDef *pwm_inst, float duty);

    /// @brief 启用PWM输出
    void BspTIMPWM_Enable(BspTIMPWM_TypeDef *pwm_inst);

    /// @brief 禁用PWM输出
    void BspTIMPWM_Disable(BspTIMPWM_TypeDef *pwm_inst);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_TIM_PWM_H__ */
