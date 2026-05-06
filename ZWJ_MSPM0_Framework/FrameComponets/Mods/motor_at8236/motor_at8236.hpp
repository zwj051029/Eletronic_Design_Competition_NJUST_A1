#pragma once
#include "bsp_gpio.h"
#include "bsp_tim_pwm.h"
#include "motor_at8236_IRQ.h"
#include "pid.hpp"
#include "ti_msp_dl_config.h"

typedef enum {
    Speed_Control_Mode,
    Pos_Control_Mode,
    No_Control_Mode,
} MotorMode;

void SpeedUpdateISR();
void EncoderISR();

class Motor {
    friend void SpeedUpdateISR();
    friend void EncoderISR();
    friend class Navigation;

private:
    // pwm
    BspTIMPWM_TypeDef PWM;
    BspGpio_Instance direction_control;
    float max_speed = 330;
    float target_speed;
    float current_speed = 0;
    float target_position;
    float current_position = 0;
    float total_pulse_count = 0;

    MotorMode mode = Speed_Control_Mode;

    /// 编码器相关参数
    BspGpio_Instance encoderA_inst;         // 霍尔编码器的A相
    BspGpio_Instance encoderB_inst;         // 霍尔编码器的B相
    int64_t pulse_count = 0;                // 带符号累计脉冲数，正转加，反转减
    int64_t last_pulse_count = 0;           // 上一次速度更新时的脉冲计数
    uint16_t encoder_lines = 13;            // 编码器线数，单相每圈脉冲数
    uint16_t gear_ratio = 30;               // 减速比，电机轴转数 / 输出轴转数
    float speed_calculation_period = 0.01f; // M法计算周期
    int64_t delta;                          // 脉冲·增量

    // 电机相关参数
    float wheel_base = 156.5 ; //轮距，单位为mm
    

    // Motor_Instance motor_inst;

    // PID相关
    Pids pid_speed;    // 速度环PID
    Pids pid_position; // 位置环PID

public:
    void Init(GPIO_Regs *encoderA_port, uint32_t encoderA_pin, GPIO_Regs *encoderB_port, uint32_t encoderB_pin,
              GPTIMER_Regs *htim, DL_TIMER_CC_INDEX channel, GPIO_Regs *direction_control_port,
              uint32_t direction_control_pin);

    void SetSpeed(float target_speed);
    void SpeedLoop();

    void SetPosition(float target_postion);
    void PositionLoop();

    void Enable();
    void Disable();

    /// @brief 电机的单个控制
    void Control();
    /// @brief 电机的总体控制
    static void ControlAllMotors();

    /// @brief 计算速度函数
    void SpeedCalculation();

    void SetSpeedPIDCoeffienct(float kp, float ki, float kd);
    void SetPositionPIDCoeffienct(float kp, float ki, float kd);
};

// 请先注册
extern Motor motor_left;
extern Motor motor_right;

// NVIC_EnableIRQ(GPIOA_INT_IRQn);
// NVIC_EnableIRQ(TIMG6_INT_IRQn);

// // 先左后右
// motor_left.Init(GPIOA, DL_GPIO_PIN_15, GPIOA, DL_GPIO_PIN_16, TIMG8, DL_TIMER_CC_1_INDEX, GPIOA, DL_GPIO_PIN_23);
// motor_left.Enable();

// motor_right.Init(GPIOA, DL_GPIO_PIN_12, GPIOA, DL_GPIO_PIN_13, TIMG7, DL_TIMER_CC_0_INDEX, GPIOA, DL_GPIO_PIN_27);
// motor_right.Enable();

// motor_left.SetPIDCoeffienct(0.0008f, 0.01f, 0.000001f);
// motor_right.SetPIDCoeffienct(0.009f, 0.02f, 0.0000012f);