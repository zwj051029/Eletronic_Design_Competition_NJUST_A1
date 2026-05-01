#include "bsp_tim_pwm.h"
#include "ti_msp_dl_config.h"




/**
 * @brief  获取PWM频率
 * @param  pwm_inst PWM实例
 * @retval PWM频率（Hz）
 */
static float GetFreq(struct BspTIMPWM_t pwm_inst)
{
    // 检验定时器句柄有效性
    if (pwm_inst.htim == NULL)
        return 0.0f; // 定时器句柄无效

    // 获取时钟主频
    uint32_t timer_clock_freq = CPUCLK_FREQ / 2;
    
    // 获取主频时钟的ARR值
    uint32_t arr = pwm_inst.htim->COUNTERREGS.LOAD;
    
    // 获取时钟配置，结果存入结构体clockConfig（包含分频系数与PRC）
    DL_Timer_ClockConfig clockConfig;
    DL_Timer_getClockConfig(pwm_inst.htim, &clockConfig);

    // Timer Clock Divider（DIV）和 Timer Clock Prescaler（PRC）
    uint8_t prescale = clockConfig.prescale;
    // 将 DIV 枚举转换为实际分频系数（1 ~ 8）
    uint32_t divideRatio = 1; // 默认 1 分频
    switch (clockConfig.divideRatio) {
        case DL_TIMER_CLOCK_DIVIDE_1: divideRatio = 1; break;
        case DL_TIMER_CLOCK_DIVIDE_2: divideRatio = 2; break;
        case DL_TIMER_CLOCK_DIVIDE_3: divideRatio = 3; break;
        case DL_TIMER_CLOCK_DIVIDE_4: divideRatio = 4; break;
        case DL_TIMER_CLOCK_DIVIDE_5: divideRatio = 5; break;
        case DL_TIMER_CLOCK_DIVIDE_6: divideRatio = 6; break;
        case DL_TIMER_CLOCK_DIVIDE_7: divideRatio = 7; break;
        case DL_TIMER_CLOCK_DIVIDE_8: divideRatio = 8; break;
        default: divideRatio = 1; 
    }
    
    // 计算PWM频率 = 时钟主频 / ((ARR + 1) * (prescale + 1) * DIV)
    float pwm_freq = (float)timer_clock_freq / ( (arr+1) * (prescale + 1) * divideRatio ) ;

    return pwm_freq;
}

/**
 * @brief  注册PWM实例
 * @param  pwm_inst PWM实例
 * @param  htim     定时器句柄
 * @param  channel  PWM通道
 */
void BspTIMPWM_InstRegist(BspTIMPWM_TypeDef *pwm_inst, GPTIMER_Regs *htim, uint32_t channel)
{
    // 检验参数有效性
    if (pwm_inst == NULL || htim == NULL)
        return; // 参数无效

    // 配置PWM实例的相关参数
    pwm_inst->htim = htim;       // 定时器句柄
    pwm_inst->channel = channel; // PWM通道

    // 获取ARR寄存器的值
    pwm_inst->auto_reload_value = pwm_inst->htim->COUNTERREGS.LOAD;
    // 获取CCR寄存器的值
    pwm_inst->compare_value = 200 - 1;
    // 给函数指针赋值
    pwm_inst->GetFreq = GetFreq;
    // 计算PWM频率
    pwm_inst->freq = pwm_inst->GetFreq(*pwm_inst);

    // 初始化PWM的占空比为0
    BspTIMPWM_SetDuty(pwm_inst, 0.0f);
}

/**
 * @brief  设置PWM占空比
 * @param  pwm_inst PWM实例
 * @param  duty     占空比（0.0 - 1.0）
 */
void BspTIMPWM_SetDuty(BspTIMPWM_TypeDef *pwm_inst, float duty)
{
    // 检查输入的占空比范围
    if (duty < 0.0f)
        duty = 0.0f;
    if (duty > 1.0f)
        duty = 1.0f;

    // 更新PWM实例的占空比
    pwm_inst->duty = duty;

    // 计算CCR的对应值
    pwm_inst->compare_value = (uint32_t)(pwm_inst->auto_reload_value * (1.0f - duty));
    

    // 更新定时器的比较寄存器
    DL_Timer_setCaptureCompareValue(pwm_inst->htim, pwm_inst->compare_value, pwm_inst->channel);
}

/**
 * @brief  启用PWM输出
 * @param  pwm_inst PWM实例
 */
void BspTIMPWM_Enable(BspTIMPWM_TypeDef *pwm_inst)
{
    // 检查参数有效性
    if (pwm_inst == NULL)
        return; // 参数无效

    if (!pwm_inst->enabled)
    {
        // 启动PWM输出
        DL_Timer_startCounter(pwm_inst->htim);
        pwm_inst->enabled = 1; // 标记为已启用
    }
}

/**
 * @brief  禁用PWM输出
 * @param  pwm_inst PWM实例
 */
void BspTIMPWM_Disable(BspTIMPWM_TypeDef *pwm_inst)
{
    // 检查参数有效性
    if (pwm_inst == NULL)
        return; // 参数无效

    if (pwm_inst->enabled)
    {
        // 停止PWM输出
        DL_Timer_stopCounter(pwm_inst->htim);
        pwm_inst->enabled = 0; // 标记为已禁用
    }
}
