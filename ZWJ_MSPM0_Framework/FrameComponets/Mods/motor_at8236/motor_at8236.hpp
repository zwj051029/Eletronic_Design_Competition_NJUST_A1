#pragma once

#include "bsp_gpio.h"
#include "bsp_tim_pwm.h"
#include "pid.hpp"
#include "ti_msp_dl_config.h"

typedef enum {
    Speed_Control_Mode,
    Pos_Control_Mode,
    No_Control_Mode,
} MotorMode;

class MotorAT8236 {
    friend void GROUP0_IRQHandler();

private:
    /// 编码器相关参数
    BspGpio_Instance encoderA_inst;         // 霍尔编码器的A相
    BspGpio_Instance encoderB_inst;         // 霍尔编码器的B相
    int64_t pulse_count = 0;                // 带符号累计脉冲数，正转加，反转减
    int64_t last_pulse_count = 0;           // 上一次速度更新时的脉冲计数
    uint16_t encoder_lines = 1;             // 编码器线数，单相每圈脉冲数
    uint16_t gear_ratio = 1;                // 减速比，电机轴转数 / 输出轴转数
    float speed_calculation_period = 0.01f; // M法计算周期

    /// AT8236相关参数
    BspTIMPWM_t PWMA;      // AT8236的PWM的A相
    BspGpio_Instance PWMB; // AT8236的PWM的B相，实际还是要配置为普通GPIO

    // 电机的相关参数
    float current_speed = 0.0f; // 当前实际速度
    float target_speed = 0.0f;  // 电机的目标速度
    float max_speed;            // 电机的最大速度
    float min_speed;            // 电机的最小速度

    bool initialized = false; // 电机是否初始化
    bool enabled = false;     // 电机是否使能

    MotorMode mode = No_Control_Mode; // 电机模式

    // PID相关
    PidGeneral pid_speed; // 速度环PID

public:
    /// @brief 电机初始化函数
    void Init(GPIO_Regs *encoderA_port, uint32_t encoderA_pin, GPIO_Regs *encoderB_port, uint32_t encoderB_pin,
              uint16_t encoder_lines, uint16_t gear_ratio, GPTIMER_Regs *PWMA_htim, uint32_t PWMA_channel,
              GPTIMER_Regs *PWMB_htim, uint32_t PWMB_channel, float max_speed, float min_speed);

    /// @brief 电机使能函数
    void Enable();
    /// @brief 电机失能函数
    void Disable();

    /// @brief 电机目标速度设置函数
    void SetPIDSpeedLoop(float target_speed);
    /// @brief 计算速度函数
    void SpeedCalculation();
    /// @brief PWM更新函数
    void UpdatePWM(float duty);

    /// @brief 电机的单个控制
    void Control(float target_speed);
    /// @brief 电机的总体控制
    void ControlAllMotors(float target_speed[]);
};

extern MotorAT8236 motor_left();
extern MotorAT8236 motor_right;
