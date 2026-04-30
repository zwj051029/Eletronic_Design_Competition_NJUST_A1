#include "motor_at8236.hpp"

MotorAT8236* MotorAT8236::instance = nullptr;

/**
 * @brief  
 * @param encoderA_port ：编码器A相的GPIOx
 * @param encoderA_pin ：编码器A相的GPIO引脚
 * @param encoderB_port ：编码器B相的GPIOx
 * @param encoderB_pin ：编码器B相的GPIO引脚
 * @param encoder_lines ：编码器的码器线数，单相每圈脉冲数
 * @param gear_ratio ：编码器的减速比，电机轴转数 / 输出轴转数
 * @param PWMA_htim ：驱动电机的PWMA的定时器Timerx
 * @param PWMA_channel :驱动电机的PWMA的定时器的通道
 * @param PWMB_htim :驱动电机的PWMB的定时器Timerx
 * @param PWMB_channel :驱动电机的PWMA的定时器的通道
 * @param max_speed ：电机的最大速度
 */
void MotorAT8236::Init(GPIO_Regs *encoderA_port, uint32_t encoderA_pin, 
                       GPIO_Regs *encoderB_port, uint32_t encoderB_pin,  
                       uint16_t encoder_lines , uint16_t gear_ratio,                    
                       GPTIMER_Regs *PWMA_htim, uint32_t PWMA_channel,
                       GPTIMER_Regs *PWMB_htim, uint32_t PWMB_channel,
                       float max_speed)
{
    // 初始化编码器的GPIO与电机的PWM
    BspGpio_InstRegister(&this->encoderA_inst, encoderA_port, encoderA_pin);
    BspGpio_InstRegister(&this->encoderB_inst, encoderB_port, encoderB_pin);
    BspTIMPWM_InstRegist(&this->PWMA, PWMA_htim, PWMA_channel);
    BspTIMPWM_InstRegist(&this->PWMB, PWMB_htim, PWMB_channel);
    
    //初始化基本参数
    this->encoder_lines = encoder_lines;
    this->gear_ratio = gear_ratio;
    this->max_speed = max_speed;
    this->initialized = true; 
    
    // 用于中断函数中调用类的静态方法，以读取编码器脉冲信号
    instance = this; 
}

/**
 * @brief 获取指定引脚的双边沿触发极性宏
 * @param pin 引脚掩码，如 DL_GPIO_PIN_0
 * @return 对应的双边沿极性宏；无效引脚返回0
 */
uint32_t GetEdgeRiseFallMacro(uint32_t pin)
{
    switch (pin) {
        case DL_GPIO_PIN_0:  return DL_GPIO_PIN_0_EDGE_RISE_FALL;
        case DL_GPIO_PIN_1:  return DL_GPIO_PIN_1_EDGE_RISE_FALL;
        case DL_GPIO_PIN_2:  return DL_GPIO_PIN_2_EDGE_RISE_FALL;
        case DL_GPIO_PIN_3:  return DL_GPIO_PIN_3_EDGE_RISE_FALL;
        case DL_GPIO_PIN_4:  return DL_GPIO_PIN_4_EDGE_RISE_FALL;
        case DL_GPIO_PIN_5:  return DL_GPIO_PIN_5_EDGE_RISE_FALL;
        case DL_GPIO_PIN_6:  return DL_GPIO_PIN_6_EDGE_RISE_FALL;
        case DL_GPIO_PIN_7:  return DL_GPIO_PIN_7_EDGE_RISE_FALL;
        case DL_GPIO_PIN_8:  return DL_GPIO_PIN_8_EDGE_RISE_FALL;
        case DL_GPIO_PIN_9:  return DL_GPIO_PIN_9_EDGE_RISE_FALL;
        case DL_GPIO_PIN_10: return DL_GPIO_PIN_10_EDGE_RISE_FALL;
        case DL_GPIO_PIN_11: return DL_GPIO_PIN_11_EDGE_RISE_FALL;
        case DL_GPIO_PIN_12: return DL_GPIO_PIN_12_EDGE_RISE_FALL;
        case DL_GPIO_PIN_13: return DL_GPIO_PIN_13_EDGE_RISE_FALL;
        case DL_GPIO_PIN_14: return DL_GPIO_PIN_14_EDGE_RISE_FALL;
        case DL_GPIO_PIN_15: return DL_GPIO_PIN_15_EDGE_RISE_FALL;
        case DL_GPIO_PIN_16: return DL_GPIO_PIN_16_EDGE_RISE_FALL;
        case DL_GPIO_PIN_17: return DL_GPIO_PIN_17_EDGE_RISE_FALL;
        case DL_GPIO_PIN_18: return DL_GPIO_PIN_18_EDGE_RISE_FALL;
        case DL_GPIO_PIN_19: return DL_GPIO_PIN_19_EDGE_RISE_FALL;
        case DL_GPIO_PIN_20: return DL_GPIO_PIN_20_EDGE_RISE_FALL;
        case DL_GPIO_PIN_21: return DL_GPIO_PIN_21_EDGE_RISE_FALL;
        case DL_GPIO_PIN_22: return DL_GPIO_PIN_22_EDGE_RISE_FALL;
        case DL_GPIO_PIN_23: return DL_GPIO_PIN_23_EDGE_RISE_FALL;
        case DL_GPIO_PIN_24: return DL_GPIO_PIN_24_EDGE_RISE_FALL;
        case DL_GPIO_PIN_25: return DL_GPIO_PIN_25_EDGE_RISE_FALL;
        case DL_GPIO_PIN_26: return DL_GPIO_PIN_26_EDGE_RISE_FALL;
        case DL_GPIO_PIN_27: return DL_GPIO_PIN_27_EDGE_RISE_FALL;
        case DL_GPIO_PIN_28: return DL_GPIO_PIN_28_EDGE_RISE_FALL;
        case DL_GPIO_PIN_29: return DL_GPIO_PIN_29_EDGE_RISE_FALL;
        case DL_GPIO_PIN_30: return DL_GPIO_PIN_30_EDGE_RISE_FALL;
        case DL_GPIO_PIN_31: return DL_GPIO_PIN_31_EDGE_RISE_FALL;
        default:            return 0;
    }
}

/**
 * @brief 配置编码器 A 相的双边沿触发并打开中断
 * @note  使用的端口和引脚来自 encoderA_inst 成员，需先通过 BspGpio_InstRegister 注册好。
 */
void MotorAT8236::SetupEncoderInterrupt()
{
    uint32_t pin  = this->encoderA_inst.pin;
    GPIO_Regs* port = this->encoderA_inst.port;

    uint32_t macro = MotorAT8236::GetEdgeRiseFallMacro(pin);
    if (macro == 0) {
        return;
    }

    if (pin <= DL_GPIO_PIN_15) {
        uint32_t polarity = DL_GPIO_getLowerPinsPolarity(port);
        polarity &= ~macro;
        polarity |= macro;
        DL_GPIO_setLowerPinsPolarity(port, polarity);
    } else {
        uint32_t polarity = DL_GPIO_getUpperPinsPolarity(port);
        polarity &= ~macro;
        polarity |= macro;
        DL_GPIO_setUpperPinsPolarity(port, polarity);
    }

    DL_GPIO_enableInterrupt(port, pin);
}


/****
AT8236真值表
IN1	IN2	电机状态	功能说明
PWM	0	正转	正向 PWM 驱动，采用快衰减模式
1	PWM	正转	正向 PWM 驱动，采用慢衰减模式
0	PWM	反转	反向 PWM 驱动，采用快衰减模式
PWM	1	反转	反向 PWM 驱动，采用慢衰减模式
0   0   停止
注：主要使用“快衰减模式”
****/

/**
 * @brief 使能电机 
 */
void MotorAT8236::Enable()
{
    if(!this->initialized)
        return;

    BspTIMPWM_Enable(&this->PWMA);
    BspTIMPWM_Enable(&this->PWMB);

    this->enabled = true;
}     

/**
 * @brief 失能电机 
 */
void MotorAT8236::Disable()
{
    if(!this->initialized)
        return;

    BspTIMPWM_Disable(&this->PWMA);
    BspTIMPWM_Disable(&this->PWMB);

    this->enabled = false;
}   

/**
 * @brief 设置电机的目标速度
 * @param target_speed：电机的目标速度
 */
void MotorAT8236::SetTargetSpeed(float target_speed)
{
    this->target_speed = target_speed;
}

/**
 * @brief 获得电机的目标速度
 * @return float：电机的目标速度
 */
float MotorAT8236::GetTargetSpeed()
{
    return this->target_speed;
}

/**
 * @brief 获得电机的当前速度
 * @return float：电机的当前速度 
 */
float MotorAT8236::GetCurrentSpeed()
{
    return this->current_speed ;
}

/**
 * @brief 根据霍尔传感器的工作原理，记录脉冲信号
 * @note  1. 该方法为静态方法，可在全局中断里被直接调用，因为静态方法属于类本身，不需要 this 指针。
          2. instance 是静态成员指针，在Init时指向this指针（instance = this），让静态方法能通过它访问普通成员。
 */
void MotorAT8236::EncoderPluse()
{
    if (instance == nullptr) 
        return;

    // 读取编码器A相与B相电平
    bool A = BspGpio_GetState(&instance->encoderA_inst);
    bool B = BspGpio_GetState(&instance->encoderB_inst);

    // A触发的正交解码，其逻辑为当AB同相时
    if (A == B)
        instance->pulse_count++;   // 正转
    else
        instance->pulse_count--;   // 反转
}

/**
* @brief 获取当前电机实例的静态指针
 * @return MotorAT8236* 成功初始化后返回对象指针，使得外部中断可以调用；若尚未初始化则返回 nullptr
 */
MotorAT8236* MotorAT8236::GetInstance() {
    return instance;   
}

/**
 * @brief GPIOA 端口中断服务函数，用于处理电机编码器 A 相边沿触发事件
 * @note  通过 MotorAT8236::GetInstance() 获取当前电机实例，读取 A 相中断状态，
 *         调用 EncoderPluse() 更新脉冲计数，并清除中断标志。
 *         如果需要调整，需要更换函数名，在.hpp中修改宏定义
 */
void GROUP1_IRQHandler()
{
    // 通过静态指针访问当前电机对象的编码器端口和引脚
    MotorAT8236* motor = MotorAT8236::GetInstance();  
    
    uint32_t status = DL_GPIO_getEnabledInterruptStatus(motor->encoderA_inst.port,
                                                         motor->encoderA_inst.pin);
    if (status & motor->encoderA_inst.pin) 
    {
        MotorAT8236::EncoderPluse();
        DL_GPIO_clearInterruptStatus(motor->encoderA_inst.port, motor->encoderA_inst.pin);
    }
}