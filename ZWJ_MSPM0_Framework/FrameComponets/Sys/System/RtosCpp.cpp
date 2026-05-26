#include "RtosCpp.hpp"
#include "Follow.hpp"
#include "FreeRTOS.h"
#include "M0_Comm.hpp"
#include "MainFrame.hpp"
#include "MainStateMachine.hpp"
#include "SpeedMixer.hpp"
#include "System.hpp"
#include "bsp_delay.h"
#include "bsp_dwt.h"
#include "bsp_uart.h"
#include "motor_at8236.hpp"
#include "std_cpp.h"
#include "task.h"

#define SPEED_DIFF 15.0f

float right_motor_speed = 0.0f;
float left_motor_speed = 0.0f;

/******      主初始化函数      ******/
/**
 * @brief 机器人主初始化函数
 * @note 该函数调用各模块的初始化函数，确保系统各部分正确配置
 * @warning 为什么要搞一个这个，而不是在RTOS启动的线程初始化呢
 * 主要是因为怕线程爆栈，主函数的栈深基本上摸不到底的
 */
void MainInitCpp() {
    NVIC_EnableIRQ(GPIOA_INT_IRQn);
    NVIC_EnableIRQ(TIMG6_INT_IRQn);

    // 先左后右
    motor_left.Init(GPIOA, DL_GPIO_PIN_15, GPIOA, DL_GPIO_PIN_16, TIMG8, DL_TIMER_CC_1_INDEX, GPIOA, DL_GPIO_PIN_23);
    motor_left.Enable();

    motor_right.Init(GPIOA, DL_GPIO_PIN_12, GPIOA, DL_GPIO_PIN_13, TIMG7, DL_TIMER_CC_0_INDEX, GPIOA, DL_GPIO_PIN_27);
    motor_right.Enable();

    motor_left.SetPIDCoeffienct(0.0008f, 0.01f, 0.000001f);
    motor_right.SetPIDCoeffienct(0.009f, 0.02f, 0.0000012f);

    m0_comm.Init();
}

/******      RTOS任务相关的函数      ******/
/**
 * @brief 机器人高频控制任务（1000Hz）
 * @note 该任务负责机器人的实时控制逻辑
 */
void ControlCpp() {

    while (1) {
        if (m0_comm.IsNewFrame()) {
            motor_left.SetSpeed(m0_comm.GetTargetLeft());
            motor_right.SetSpeed(m0_comm.GetTargetRight());
            m0_comm.ClearNewFrame();
        }
        
        // 执行电机速度闭环控制（内部读取编码器并调节 PWM）
        Motor::ControlAllMotors();

        /***     最大循环频率：1000Hz     ***/
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
