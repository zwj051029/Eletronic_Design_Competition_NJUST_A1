#pragma once

#include "ti_msp_dl_config.h"
#include "bsp_tim_pwm.h"
#include "pid.hpp"
#include "bsp_gpio.h"

class MotorAT8236{
    private:
        /// 编码器相关参数
        BspGpio_Instance encoderA_inst;          // 霍尔编码器的A相
        BspGpio_Instance encoderB_inst;          // 霍尔编码器的B相
        volatile int64_t pulseCount = 0;          // 带符号累计脉冲数（正转加，反转减）
        uint16_t encoder_lines = 1;               // 编码器线数（单相每圈脉冲数）
        volatile int64_t last_pulse_count = 0;    // 上一次速度更新时的脉冲计数
        uint16_t gear_ratio = 1;                  // 减速比（电机轴转数 / 输出轴转数）
        volatile float current_speed = 0.0f;      // 当前实际速度（RPM）
        
        /// AT8236相关参数
        BspTIMPWM_t PWMA;                        // AT8236的PWMA的A相
        BspTIMPWM_t PWMB;                        // AT8236的PWMA的B相

        // 电机的相关参数
        float target_speed;                       //电机的当前速度
        float max_speed;                          //电机的最大速度
        bool initialized = false;                 //电机是否初始化
        bool enabled = false;                     //电机是否使能

    public:
        /// @brief 电机初始化函数
        void Init(GPIO_Regs *encoderA_port, uint32_t encoderA_pin, 
                  GPIO_Regs *encoderB_port, uint32_t encoderB_pin,
                  uint16_t encoderLines = 1, uint16_t gearRatio = 1,
                  BspTIMPWM_TypeDef *PWMA_inst, GPTIMER_Regs *PWMA_htim, uint32_t PWMA_channel,
                  BspTIMPWM_TypeDef *PWMB_inst, GPTIMER_Regs *PWMB_htim, uint32_t PWMB_channel,
                  float max_speed);
                  
        /// @brief 电机使能函数
        void Enable();
        /// @brief 电机失能函数
        void Disable();    
         
        /// @brief 电机目标速度设置函数  
        void SetTargetSpeed(float target_speed);
        
        /// @brief 电机当前速度获取函数
        float GetCurrentSpeed();
        /// @brief 电机目标速度获取函数
        float GetTargetSpeed();
        
        /// @brief 编码器中断处理函数
        static void EncoderISR();
        /// @brief 速度更新函数
        void UpdateSpeed(float dt);
        
};

