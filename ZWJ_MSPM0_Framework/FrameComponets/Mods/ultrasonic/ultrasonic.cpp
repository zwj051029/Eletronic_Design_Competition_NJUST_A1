#include "ultrasonic.hpp"
#include "bsp_delay.h"
#include "task.h"

// #define ULTRASONIC_TIM_INST TIMA1

// 80MHz主频 → 1个时钟周期=12.5ns → 1us=80个周期
#define CYCLES_TO_US(cycles) ((float) (cycles) / 80.0f)

void Ultrasonic_Gpio::Init(GPIO_Regs *echo_port, uint32_t echo_pin, GPIO_Regs *trig_port, uint32_t trig_pin) {
    BspGpio_InstRegister(&echo_inst, echo_port, echo_pin);
    BspGpio_InstRegister(&trig_inst, trig_port, trig_pin);
    BspGpio_SetState(&trig_inst, BSPGPIO_LOW_STATE);

    // timer_handle = xTimerCreate("UltrasonicTimer", // 定时器名称
    //                             pdMS_TO_TICKS(1),  // 周期1ms
    //                             pdTRUE,            // 自动重载
    //                             this,              // 定时器ID：传递this指针
    //                             TimerCallback      // 回调函数
    // );

    this->state = ULTRASONIC_IDLE;
    this->initialized = true;
}

float Ultrasonic_Gpio::GetDistance() {
    if (!(this->initialized))
        return 0.0f;

    return this->distance;
}

void Ultrasonic_Gpio::Enable() {
    if (!initialized || timer_handle == NULL)
        return;
    enabled = true;
    state = ULTRASONIC_IDLE;

    // 启动软件定时器
    if (xTimerIsTimerActive(timer_handle) == pdFALSE) {
        xTimerStart(timer_handle, 0);
    }
}

void Ultrasonic_Gpio::Disable() {
    if (!initialized || timer_handle == NULL)
        return;
    enabled = false;
    state = ULTRASONIC_IDLE;

    // 停止软件定时器
    if (xTimerIsTimerActive(timer_handle) == pdTRUE) {
        xTimerStop(timer_handle, 0);
    }
}

void Ultrasonic_Gpio::SendTrigger(void) {
    BspGpio_SetState(&trig_inst, BSPGPIO_LOW_STATE);
    BspDelay_us(2);
    BspGpio_SetState(&trig_inst, BSPGPIO_HIGH_STATE);
    BspDelay_us(15);
    BspGpio_SetState(&trig_inst, BSPGPIO_LOW_STATE);
}

// // 软件定时器回调函数（静态，由FreeRTOS自动调用）
// void Ultrasonic_Gpio::TimerCallback(TimerHandle_t xTimer) {
//     Ultrasonic_Gpio *instance = static_cast<Ultrasonic_Gpio *>(pvTimerGetTimerID(xTimer));

//     if (instance == NULL)
//         return;

//     if (!instance->initialized || !instance->enabled) {
//         instance->state = ULTRASONIC_IDLE;
//         return;
//     }

//     switch (instance->state) {
//     // ------------------------------
//     // 步骤1：发送触发脉冲
//     // ------------------------------
//     case ULTRASONIC_IDLE:
//         instance->SendTrigger();

//         // 清定时器计数器+清中断标志
//         DL_Timer_setTimerCount(ULTRASONIC_TIM_INST, 0);
//         DL_Timer_clearInterruptStatus(ULTRASONIC_TIM_INST, DL_TIMER_INTERRUPT_LOAD_EVENT);

//         // 记录超时开始时间
//         instance->timeout_start_tick = xTaskGetTickCount();
//         instance->state = ULTRASONIC_WAIT_RISE;
//         break;

//     // ------------------------------
//     // 步骤2：等待ECHO变高
//     // ------------------------------
//     case ULTRASONIC_WAIT_RISE:
//         // 检测到ECHO变高
//         if (BspGpio_GetState(&instance->echo_inst) == BSPGPIO_HIGH_STATE) {
//             // 启动定时器
//             DL_Timer_startCounter(ULTRASONIC_TIM_INST);
//             instance->state = ULTRASONIC_WAIT_FALL;
//         }
//         // 超时处理
//         else if (xTaskGetTickCount() - instance->timeout_start_tick >= instance->timeout_ms) {
//             instance->distance = -1.0f;
//             instance->state = ULTRASONIC_IDLE;
//         }
//         break;

//     // ------------------------------
//     // 步骤3：等待ECHO变低/定时器溢出
//     // ------------------------------
//     case ULTRASONIC_WAIT_FALL:
//         // 检测到ECHO变低（测量成功）
//         if (BspGpio_GetState(&instance->echo_inst) == BSPGPIO_LOW_STATE) {
//             // 停止定时器
//             DL_Timer_stopCounter(ULTRASONIC_TIM_INST);

//             // 计数值*0.17=厘米
//             uint32_t count = DL_Timer_getTimerCount(ULTRASONIC_TIM_INST);
//             instance->distance = (float) count * 0.017f;
//             instance->state = ULTRASONIC_DONE;
//         }
//         // 检测定时器溢出（40ms到了）
//         else if (DL_Timer_getRawInterruptStatus(ULTRASONIC_TIM_INST, DL_TIMER_INTERRUPT_LOAD_EVENT)) {
//             DL_Timer_stopCounter(ULTRASONIC_TIM_INST);
//             DL_Timer_clearInterruptStatus(ULTRASONIC_TIM_INST, DL_TIMER_INTERRUPT_LOAD_EVENT);
//             instance->distance = -1.0f;
//             instance->state = ULTRASONIC_IDLE;
//         }
//         break;

//     // ------------------------------
//     // 步骤4：测量完成，准备下一次
//     // ------------------------------
//     case ULTRASONIC_DONE:
//         instance->state = ULTRASONIC_IDLE;
//         break;

//     default:
//         instance->state = ULTRASONIC_IDLE;
//         break;
//     }
// }
