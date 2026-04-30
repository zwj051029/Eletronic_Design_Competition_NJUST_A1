#include "motor_at8236.hpp"

#define MotorAT8236_MAX_CANINSTS 2

static MotorAT8236 *motor_at8236_insts[MotorAT8236_MAX_CANINSTS] = {NULL};
static uint8_t motor_at8236_insts_count = 0;

MotorAT8236 motor_left;
MotorAT8236 motor_right;

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
void MotorAT8236::Init(GPIO_Regs *encoderA_port, uint32_t encoderA_pin, GPIO_Regs *encoderB_port, uint32_t encoderB_pin,
                       uint16_t encoder_lines, uint16_t gear_ratio, GPTIMER_Regs *PWMA_htim, uint32_t PWMA_channel,
                       GPTIMER_Regs *PWMB_htim, uint32_t PWMB_channel, float max_speed) {
    // 初始化编码器的GPIO与电机的PWM
    BspGpio_InstRegister(&this->encoderA_inst, encoderA_port, encoderA_pin);
    BspGpio_InstRegister(&this->encoderB_inst, encoderB_port, encoderB_pin);
    BspTIMPWM_InstRegist(&this->PWMA, PWMA_htim, PWMA_channel);
    BspTIMPWM_InstRegist(&this->PWMB, PWMB_htim, PWMB_channel);

    // 初始化基本参数
    this->encoder_lines = encoder_lines;
    this->gear_ratio = gear_ratio;
    this->max_speed = max_speed;
    this->initialized = true;

    // 用于中断函数中调用类的静态方法，以读取编码器脉冲信号
    motor_at8236_insts[motor_at8236_insts_count++] = this;
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
void MotorAT8236::Enable() {
    if (!this->initialized)
        return;

    BspTIMPWM_Enable(&this->PWMA);
    BspTIMPWM_Enable(&this->PWMB);

    this->enabled = true;
}

/**
 * @brief 失能电机
 */
void MotorAT8236::Disable() {
    if (!this->initialized)
        return;

    BspTIMPWM_Disable(&this->PWMA);
    BspTIMPWM_Disable(&this->PWMB);

    this->enabled = false;
}

/**
 * @brief 设置电机的目标速度
 * @param target_speed：电机的目标速度
 */
void MotorAT8236::SetSpeed(float target_speed) {
    this->target_speed = target_speed;
}

/**
 * @brief 获得电机的当前速度
 * @return float：电机的当前速度
 */
float MotorAT8236::GetCurrentSpeed() {
    return this->current_speed;
}

void MotorAT8236::ControlAllMotors()
{
    // 
    
    for (uint8_t i = 0; i < motor_at8236_insts_count; i++)
    {
        MotorAT8236 *motor_at8236 = motor_at8236_insts[i];
        if (motor_at8236 != NULL)
        {
            motor_at8236->Control();
        }
    }
}

void MotorAT8236::Control()
{
    switch (mode) {
        case Speed_Control_Mode:
    }
}

void GROUP1_IRQHandler(void) {
}