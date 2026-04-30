#pragma once

#include "ti_msp_dl_config.h"
#include "bsp_tim_pwm.h"
#include "pid.hpp"
#include "bsp_gpio.h"
#include <ti/driverlib/dl_gpio.h>

#define  GROUP1_IRQHandler GROUP1_IRQHandler      //中断函数根据实际选择对应编码器引脚的中断函数

class MotorAT8236{
    private:
        ///@brief 获取指定引脚的双边沿触发极性宏
        uint32_t GetEdgeRiseFallMacro(uint32_t pin);
        ///@brief 配置编码器 A 相的双边沿触发并打开中断
        
        /// 编码器相关参数
        void SetupEncoderInterrupt();
        BspGpio_Instance encoderB_inst;           // 霍尔编码器的B相
        int64_t pulse_count = 0;                  // 带符号累计脉冲数，正转加，反转减
        uint16_t encoder_lines = 1;               // 编码器线数，单相每圈脉冲数
        int64_t last_pulse_count = 0;             // 上一次速度更新时的脉冲计数
        uint16_t gear_ratio = 1;                  // 减速比，电机轴转数 / 输出轴转数
        
        /// AT8236相关参数
        BspTIMPWM_t PWMA;                         // AT8236的PWMA的A相
        BspTIMPWM_t PWMB;                         // AT8236的PWMA的B相

        // 电机的相关参数
        float current_speed = 0.0f;               // 当前实际速度
        float target_speed = 0;                   // 电机的目标速度
        float max_speed;                          // 电机的最大速度
        bool initialized = false;                 // 电机是否初始化
        bool enabled = false;                     // 电机是否使能

    public:
        BspGpio_Instance encoderA_inst;           // 霍尔编码器的A相
        
        // 用于中断回调
        static MotorAT8236* instance;
        /// @brief 电机初始化函数
        void Init(GPIO_Regs *encoderA_port, uint32_t encoderA_pin, 
                  GPIO_Regs *encoderB_port, uint32_t encoderB_pin,
                  uint16_t encoder_lines, uint16_t gear_ratio,
                  GPTIMER_Regs *PWMA_htim, uint32_t PWMA_channel,
                  GPTIMER_Regs *PWMB_htim, uint32_t PWMB_channel,
                  float max_speed);
                  
        /// @brief 电机使能函数
        void Enable();
        /// @brief 电机失能函数
        void Disable();    
         
        /// @brief 电机目标速度设置函数  
        void SetTargetSpeed(float target_speed);
        
        /// @brief 电机目标速度获取函数
        float GetTargetSpeed();        
        /// @brief 电机当前速度获取函数
        float GetCurrentSpeed();

        /// @brief 编码器脉冲更新函数
        static void EncoderPluse();
        /// @brief 
        static MotorAT8236* GetInstance();
        /// @brief 利用PID更新速度函数
        void UpdateSpeed(float dt);
};



