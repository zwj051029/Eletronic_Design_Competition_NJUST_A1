#include "motor_at8236.hpp"

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
}
                  