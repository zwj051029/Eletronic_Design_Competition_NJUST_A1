#include "motor_at8236.hpp"

// 限制注册实例变量
#define MotorAT8236_MAX_CANINSTS 2

// 限制注册实例变量
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
 * @param PWMB_htim :驱动电机的另一个通道的GPIOx
 * @param PWMB_channel :驱动电机的PWMA的另一个通道的GPIO引脚
 * @param max_speed ：电机的最大速度
 */
void MotorAT8236::Init(GPIO_Regs *encoderA_port, uint32_t encoderA_pin, GPIO_Regs *encoderB_port, uint32_t encoderB_pin,
                       uint16_t encoder_lines, uint16_t gear_ratio, GPTIMER_Regs *PWMA_htim, uint32_t PWMA_channel,
                       GPIO_Regs *PWMB_port, uint32_t PWMB_pin, float max_speed, float min_speed) {
    // 初始化编码器的GPIO与电机的PWM
    BspGpio_InstRegister(&this->encoderA_inst, encoderA_port, encoderA_pin);
    BspGpio_InstRegister(&this->encoderB_inst, encoderB_port, encoderB_pin);
    BspTIMPWM_InstRegist(&this->PWMA, PWMA_htim, PWMA_channel);
    BspGpio_InstRegister(&this->PWMB, PWMB_port, PWMB_pin);

    // 初始化基本参数
    this->encoder_lines = encoder_lines;
    this->gear_ratio = gear_ratio;
    this->max_speed = max_speed;
    this->min_speed = min_speed;
    this->initialized = true;

    // PID初始化，后续可以自行调用修改函数进行修改
    pid_speed.Init(2.0f, 0.5f, 0.1f, false); // kp=2.0, ki=0.5, kd=0.1, 不反向
    pid_speed.IncreLize(true);               // 启用增量式PID
    pid_speed.SetLimit(0.3f, 1.0f, 0.9f);    // 积分限幅30%, 输出限幅1.0, 微分滤波0.9

    // 限制注册实例变量
    motor_at8236_insts[motor_at8236_insts_count++] = this;
}

/**
 * @brief 使能电机
 */
void MotorAT8236::Enable() {
    if (!this->initialized || this->enabled)
        return;

    BspTIMPWM_Enable(&this->PWMA);
    BspGpio_SetState(&this->PWMB, BSPGPIO_LOW_STATE); // 默认低电平

    this->enabled = true;
}

/**
 * @brief 失能电机
 */
void MotorAT8236::Disable() {
    if (!this->initialized || !this->enabled)
        return;

    BspTIMPWM_Disable(&this->PWMA);
    BspGpio_SetState(&this->PWMB, BSPGPIO_LOW_STATE);

    this->enabled = false;
}

void MotorAT8236::ControlAllMotors(float target_speed[]) {
    if (!this->initialized || !this->enabled)
        return;

    for (uint8_t i = 0; i < motor_at8236_insts_count; i++) {
        MotorAT8236 *motor_at8236 = motor_at8236_insts[i];
        if (motor_at8236 != NULL) {
            motor_at8236->Control(target_speed[i]);
        }
    }
}

/**
 * @brief 电机控制主循环，根据当前模式执行相应的控制策略
 */
void MotorAT8236::Control(float target_speed) {
    switch (mode) {
    case Speed_Control_Mode:
        SetPIDSpeedLoop(target_speed);
        break;
    case Pos_Control_Mode:
        break;
    case No_Control_Mode:
        break;
    default:
        break;
    }
}

/**
 * @brief 计算电机当前转速（使用固定测速周期，即M法）
 * @return float 当前转速，单位 RPM（正值正转，负值反转）
 * @note  该函数应在定时器中断（例如 10ms 定时器中断）中被调用
 *        倍频：二
 */
void MotorAT8236::SpeedCalculation() {
    // 1. 计算脉冲增量
    int64_t delta = pulse_count - last_pulse_count;
    last_pulse_count = pulse_count;

    // 2. 脉冲增量 → 输出轴转数（2倍频 + 减速比）
    float rev = (float) delta / (encoder_lines * 2.0f * gear_ratio);

    // 3. 转数 → 转速 (RPM)
    this->current_speed = rev / speed_calculation_period * 60.0f;
}

/****
AT8236真值表
IN1	IN2	电机状态	功能说明
PWM	0	正转	正向 PWM 驱动，采用快衰减模式
PWM	1	反转	反向 PWM 驱动，采用慢衰减模式
1	PWM	正转	正向 PWM 驱动，采用慢衰减模式
0	PWM	反转	反向 PWM 驱动，采用快衰减模式
0   0   停止
注：主要使用“快衰减模式”
但是考虑到实际的配置，无法让两个引脚同时输入PWM，所有一只引脚配置为PWM、一只为GPIO
****/

/**
 * @brief 设置目标速度，并执行一次增量式 PID 控制（非阻塞）
 */
void MotorAT8236::SetPIDSpeedLoop(float target_speed) {
    if (!this->initialized || !this->enabled)
        return;

    this->mode = Speed_Control_Mode;
    this->target_speed = target_speed;

    // 增量式 PID 计算：Calc 内部会根据当前配置调用 CalcIncAuto
    // 返回的是经过内部累加和限幅后的绝对控制量（占空比范围 -1.0 ~ 1.0）
    // float duty = pid_speed.Calc(target_speed, current_speed, 1.0f);
    float duty = this->target_speed / this->max_speed;

    // 输出到电机,UpdatePWM 会处理正负方向
    UpdatePWM(duty);
}

/**
 * @brief 根据输入值更新电机输出（PWM+方向模式）
 * @param duty 目标占空比（-1.0 ~ 1.0，正值正转，负值反转，0 停止）
 * @note  PWMA 输出 PWM 信号，PWMB 作为方向控制（高电平反转，低电平正转/停止）
 */
void MotorAT8236::UpdatePWM(float duty) {
    if (!this->initialized || !this->enabled)
        return;

    float absDuty = fabsf(duty);
    if (absDuty > 1.0f)
        absDuty = 1.0f;

    // PWMA 始终输出 PWM 信号（绝对值占空比）
    BspTIMPWM_SetDuty(&PWMA, absDuty);

    // PWMB 作为方向信号（GPIO）
    if (duty > 0.0f) {
        // 正转：PWMB 输出低电平
        BspGpio_SetState(&PWMB, BSPGPIO_LOW_STATE);
    } else if (duty < 0.0f) {
        // 反转：PWMB 输出高电平
        BspGpio_SetState(&PWMB, BSPGPIO_HIGH_STATE);
    }
}

/**
 * @brief 使用二倍频的中断函数
 * @note 需要使能
 */
void GROUP0_IRQHandler(void) {
    for (uint8_t i = 0; i < motor_at8236_insts_count; i++) {
        MotorAT8236 *m = motor_at8236_insts[i];

        if (DL_GPIO_getEnabledInterruptStatus(m->encoderA_inst.port, m->encoderA_inst.pin)) {
            uint8_t A = BspGpio_GetState(&m->encoderA_inst) ? 1 : 0;
            uint8_t B = BspGpio_GetState(&m->encoderB_inst) ? 1 : 0;

            if (A == B)
                m->pulse_count--;
            else
                m->pulse_count++;

            DL_GPIO_clearInterruptStatus(m->encoderA_inst.port, m->encoderA_inst.pin);
        }
    }
}

/**
 * @brief 定时器归零中断，设置10ms自动触发来计算两个轮子的速度
 * @note  请填写TIMER_0_INST，即对应的定时器
 */
void TIMER_0_INST_IRQHandler(void) {
    motor_left.SpeedCalculation();
    motor_right.SpeedCalculation();

    // 定时器需要指定
    DL_Timer_clearInterruptStatus(TIMER_0_INST, DL_TIMER_IIDX_ZERO);
}