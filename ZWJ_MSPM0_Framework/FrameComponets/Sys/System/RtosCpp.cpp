#include "RtosCpp.hpp"
#include "Follow.hpp"
#include "FreeRTOS.h"
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
    System.Init();
    MainFrameCpp();

    NVIC_EnableIRQ(GPIOA_INT_IRQn);
    NVIC_EnableIRQ(TIMG6_INT_IRQn);

    // 先左后右
    motor_left.Init(GPIOA, DL_GPIO_PIN_15, GPIOA, DL_GPIO_PIN_16, TIMG8, DL_TIMER_CC_1_INDEX, GPIOA, DL_GPIO_PIN_23);
    motor_left.Enable();

    motor_right.Init(GPIOA, DL_GPIO_PIN_12, GPIOA, DL_GPIO_PIN_13, TIMG7, DL_TIMER_CC_0_INDEX, GPIOA, DL_GPIO_PIN_27);
    motor_right.Enable();

    motor_left.SetPIDCoeffienct(0.0008f, 0.01f, 0.000001f);
    motor_right.SetPIDCoeffienct(0.009f, 0.02f, 0.0000012f);
}

/******      RTOS任务相关的函数      ******/
/**
 * @brief 机器人高频控制任务（1000Hz）
 * @note 该任务负责机器人的实时控制逻辑
 */
void ControlCpp() {

    while (1) {
        right_motor_speed = speed_mixer.GetFinalRightSpeed();
        left_motor_speed = speed_mixer.GetFinalLeftSpeed() + SPEED_DIFF;

        if (MainStateMachine::cond_return_start) {
            right_motor_speed = 0.0f;
            left_motor_speed = 0.0f;
        }

        motor_right.SetSpeed(right_motor_speed);
        motor_left.SetSpeed(left_motor_speed);

        // motor_right.SetSpeed(-20.0f);
        // motor_left.SetSpeed(0.0f);

        Motor::ControlAllMotors();
        /***     最大循环频率：1000Hz     ***/
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

/**
 * @brief 机器人状态更新任务（250Hz）
 * @note 该任务负责机器人的状态监测和更新
 */
void StateCoreCpp() {
    TickType_t appTick = xTaskGetTickCount();
    StateCore &core = StateCore::GetInstance();

    while (1) {
        // 运行状态机核心
        core.Run();
        /***     最大循环频率：250Hz     ***/
        vTaskDelayUntil(&appTick, pdMS_TO_TICKS(4));
    }
}

/**
 * @brief 机器人应用管理任务（200Hz）
 * @note 该任务负责机器人的应用逻辑管理
 */
void ApplicationCpp() {
    TickType_t appTick = xTaskGetTickCount();

    while (1) {
        // 更新所有应用
        System._Update_Applications();

        /***     最大循环频率：200Hz     ***/
        vTaskDelayUntil(&appTick, pdMS_TO_TICKS(5));
    }
}

/**
 * @brief 机器人系统主任务（200Hz）
 * @note 该任务负责机器人的系统管理和协调
 */
void RobotSystemCpp() {
    TickType_t appTick = xTaskGetTickCount();

    while (1) {
        // 运行系统主进程
        System.Run();

        /***    最大循环频率：200Hz     ***/
        vTaskDelayUntil(&appTick, pdMS_TO_TICKS(5));
    }
}
