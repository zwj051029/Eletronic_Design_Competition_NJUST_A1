#include "RtosCpp.hpp"
#include "FreeRTOS.h"
#include "MainFrame.hpp"
#include "System.hpp"
#include "bsp_delay.h"
#include "bsp_dwt.h"
#include "motor_at8236.hpp"
#include "bsp_uart.h"
#include "std_cpp.h"
#include "task.h"
#include "bluetooth.hpp"

BlueTooth test_bt;

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

    motor_left.Init(GPIOA, DL_GPIO_PIN_15, GPIOA, DL_GPIO_PIN_16, 1, 1, TIMG8, GPIO_Motor_left_BIN1_PWM_C0_IDX,
                    GPIOA, DL_GPIO_PIN_22, 330, 1); //编码器两个参数和速度全部置一
    motor_right.Init(GPIOA, DL_GPIO_PIN_12, GPIOA, DL_GPIO_PIN_13, 1, 1, TIMG7, GPIO_Motor_right_AIN2_PWM_C1_IDX,
                  GPIOA, DL_GPIO_PIN_26, 330, 1); //编码器两个参数和速度全部置一

    motor_left.Enable();
    motor_right.Enable();

    motor_left.SetPIDSpeedLoop(50);
    motor_right.SetPIDSpeedLoop(50);
    BspDelay_ms(3000);

    motor_left.SetPIDSpeedLoop(150);
    motor_right.SetPIDSpeedLoop(150);
    BspDelay_ms(3000);

    motor_left.SetPIDSpeedLoop(-50);
    motor_right.SetPIDSpeedLoop(-50);
    BspDelay_ms(3000);

    motor_left.SetPIDSpeedLoop(-150);
    motor_right.SetPIDSpeedLoop(-150);
    BspDelay_ms(3000);

    motor_left.Disable();
    motor_right.Disable();
    
                             
    test_bt.Init(BlueTooth_INST);
    // //开启UART中断
    // NVIC_EnableIRQ(BlueTooth_INST_INT_IRQN);
}

/******      RTOS任务相关的函数      ******/
/**
 * @brief 机器人高频控制任务（1000Hz）
 * @note 该任务负责机器人的实时控制逻辑
 */
void ControlCpp() {

    while (1) {
        // MotorAT8236::ControlAllMotors();
        test_bt.SendMsg((uint8_t *)"hello world", 11);
        BspDelay_ms(1000);
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
        // // 运行状态机核心
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

/**
 * @brief 机器人定位模块任务（500Hz）
 * @note 该任务负责机器人的定位数据更新
 */
void PositionerCpp() {
    TickType_t appTick = xTaskGetTickCount();

    while (1) {
        /***     最大循环频率：500Hz     ***/
        vTaskDelayUntil(&appTick, pdMS_TO_TICKS(2));
    }
}
