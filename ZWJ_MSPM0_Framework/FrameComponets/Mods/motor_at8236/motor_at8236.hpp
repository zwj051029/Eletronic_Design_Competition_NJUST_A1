#pragma once

#include "ti_msp_dl_config.h"
#include "bsp_tim_pwm.h"
#include "pid.hpp"
#include "bsp_gpio.h"

class MotorAT8236{
    private:
        // 霍尔编码器的A相与B相
        BspGpio_Instance encoderA__inst;
        BspGpio_Instance encoderB__inst;
        
        // AT8236的PWMA的A相与B相
        BspTIMPWM_t PWM_A;
        BspTIMPWM_t PWM_B;

        // AT8236的基本参数
        float current_speed;                      //电机的当前速度
        float target_speed;                       //电机的当前速度
        float max_speed;                          //电机的最大速度
        bool initialized = false;                 //电机是否初始化
        bool enabled = false;                     //电机是否使能

    public:
        /// @brief 电机初始化函数
        void Init(BspGpio_Instance encoderA__inst, BspGpio_Instance encoderB__inst,
                  BspTIMPWM_t PWM_A, BspTIMPWM_t PWM_B,
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
        void EncoderISR();
        /// @brief 速度更新函数
        void UpdateSpeed(float dt);
            

    

        
    
};

