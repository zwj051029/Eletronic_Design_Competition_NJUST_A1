#include "motor_at8236.hpp"
#include "bluetooth.hpp"
#include "stdio.h"

// 限制注册实例变量
#define Motor_MAX_CANINSTS 2

Motor *motor_insts[Motor_MAX_CANINSTS] = {NULL};
uint8_t motor_insts_count = 0;

Motor motor_left;
Motor motor_right;

void Motor::Init(GPIO_Regs *encoderA_port, uint32_t encoderA_pin, GPIO_Regs *encoderB_port, uint32_t encoderB_pin,
                 GPTIMER_Regs *htim, DL_TIMER_CC_INDEX channel, GPIO_Regs *direction_control_port,
                 uint32_t direction_control_pin) {

    BspGpio_InstRegister(&this->encoderA_inst, encoderA_port, encoderA_pin);
    BspGpio_InstRegister(&this->encoderB_inst, encoderB_port, encoderB_pin);

    BspTIMPWM_InstRegist(&this->PWM, htim, channel);
    BspGpio_InstRegister(&this->direction_control, direction_control_port, direction_control_pin);

    // IRQRegister(&motor_inst, EncoderISR, SpeedUpdateISR);

    // PID初始化，后续可以自行调用修改函数进行修改
    pid_speed.Init(0.0f, 0.0f, 0.0f, false);
    pid_speed.IncreLize(true);            // 启用增量式PID
    pid_speed.SetLimit(0.3f, 0.8f, 0.6f); // 积分限幅30%, 输出限幅1.0, 微分滤波0.9

    pid_position.Init(0.0f, 0.0f, 0.0f, false); // false = 位置式PID
    pid_position.SetLimit(0.3f, 1.0f, 0.9f);          // 积分限幅30%，输出限幅100%，微分滤波

    // 限制注册实例变量
    motor_insts[motor_insts_count++] = this;
}

void Motor::SetSpeed(float target_speed) {
    this->target_speed = target_speed;
}

void Motor::SpeedLoop() {
    float duty;
    // duty = target_speed / max_speed;

    if (target_speed > 0.0f) {
        duty = pid_speed.Calc(target_speed, current_speed, 0.85f);
        BspTIMPWM_SetDuty(&this->PWM, duty);
        BspGpio_SetState(&this->direction_control, BSPGPIO_HIGH_STATE);
    } else {
        duty = pid_speed.Calc(-target_speed, -current_speed, 0.85f);
        BspTIMPWM_SetDuty(&this->PWM, 1.0f - duty);
        BspGpio_SetState(&this->direction_control, BSPGPIO_LOW_STATE);
    }
}

void Motor::SetPosition(float target_postion) {
    this->total_pulse_count = 0;
    this->current_position = 0;
    this->pulse_count = 0;
    this->last_pulse_count = 0;

    this->target_position = target_position;
    this->pid_position.Reset();
    this->mode = Pos_Control_Mode;
}

void Motor::PositionLoop() {
    float pos_error = target_position - current_position;
    float target_speed_from_pos = pid_position.Calc(0.0f, pos_error, 0.01f);

    //等等修改target_speed_from_pos = clamp(target_speed_from_pos, -this->max_speed, this->max_speed);

    this->target_speed = target_speed_from_pos;
    SpeedLoop();
}

void Motor::SetSpeedPIDCoeffienct(float kp, float ki, float kd) {
    this->pid_speed.kp = kp;
    this->pid_speed.ki = ki;
    this->pid_speed.kd = kd;
}

void Motor::SetPositionPIDCoeffienct(float kp, float ki, float kd) {
    this->pid_speed.kp = kp;
    this->pid_speed.ki = ki;
    this->pid_speed.kd = kd;
}

void Motor::Enable() {

    BspTIMPWM_Enable(&this->PWM);
}

void Motor::Disable() {

    BspTIMPWM_Disable(&this->PWM);
}

void Motor::Control() {

    switch (mode) {
    case Speed_Control_Mode:
        Motor::SpeedLoop();
        break;
    case Pos_Control_Mode:
        Motor::PositionLoop();
        break;
    case No_Control_Mode:
        break;
    default:
        break;
    }
}

/**
 * @brief 电机的总控制函数
 */
void Motor::ControlAllMotors() {
    for (uint8_t i = 0; i < motor_insts_count; i++) {
        Motor *motor = motor_insts[i];
        if (motor != NULL) {
            motor->Control();
        }
    }
}

/**
 * @brief 计算电机当前转速（使用固定测速周期，即M法）
 * @return float 当前转速，单位 RPM（正值正转，负值反转）
 * @note  该函数应在定时器中断（例如 10ms 定时器中断）中被调用
 *        倍频：1
 */
void Motor::SpeedCalculation() {
    // 1. 计算脉冲增量（速度环一直需要，所以放在外面）
    this->delta = pulse_count - last_pulse_count;

    // 2. 脉冲增量 → 输出轴转数（速度环一直需要）
    float rev = (float) delta / (encoder_lines * 1.0f * gear_ratio);

    // 3. 转数 → 转速 (RPM)（速度环一直需要）
    this->current_speed = (rev / speed_calculation_period) * 60.0f;

    // // ==========================================
    // // ✅ 核心修改：只有在位置环模式下，才计算位置相关参数
    // // ==========================================
    // if (mode == Pos_Control_Mode) {
    //     this->total_pulse_count += this->delta; // 只在位置模式下累加脉冲
    //     const float PULSE_PER_DEGREE = 5.22f;
    //     this->current_position = (float)total_pulse_count / PULSE_PER_DEGREE; // 只在位置模式下计算角度
    // }

    // 4. 更新 last_pulse_count（无论什么模式都需要）
    last_pulse_count = pulse_count;
}

extern "C" void EncoderISR() {
    // 直接读取 PA15 和 PA12 的中断状态
    uint32_t status = DL_GPIO_getEnabledInterruptStatus(
        GPIOA,
        motor_left.encoderA_inst.pin | motor_right.encoderA_inst.pin); // 或 DL_GPIO_PIN_15 | DL_GPIO_PIN_12

    // 左电机 (PA15)
    if (status & motor_left.encoderA_inst.pin) {
        DL_GPIO_writePins(LED_PORT, LED_LED_PIN_PIN);
        if (DL_GPIO_readPins(GPIOA, motor_left.encoderA_inst.pin)) // PA16
            motor_left.pulse_count++;
        else
            motor_left.pulse_count--;
        DL_GPIO_clearInterruptStatus(GPIOA, motor_left.encoderA_inst.pin);
    }

    // 右电机 (PA12)
    if (status & motor_right.encoderA_inst.pin) {
        // DL_GPIO_clearPins(LED_PORT, LED_LED_PIN_PIN);
        if (DL_GPIO_readPins(GPIOA, motor_right.encoderB_inst.pin)) // PA13
            motor_right.pulse_count--;
        else
            motor_right.pulse_count++;
        DL_GPIO_clearInterruptStatus(GPIOA, motor_right.encoderA_inst.pin);
    }
}

/**
 * @brief 实际速度计算函数
 */
extern "C" void SpeedUpdateISR() {
    for (uint8_t i = 0; i < motor_insts_count; i++) {
        Motor *m = motor_insts[i];
        m->SpeedCalculation();
    }
}