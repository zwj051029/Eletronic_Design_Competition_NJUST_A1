#pragma once

#include "FreeRTOS.h"
#include "bsp_dwt.h"
#include "bsp_gpio.h"
#include "timers.h"

#if defined GPIO_CAPTURE_ULTRASONIC_C0_PORT
#define CAPTURE_ULTRASONIC_CC_IDX DL_TIMER_CC_0_INDEX
#elif defined GPIO_CAPTURE_ULTRASONIC_C1_PORT
#define CAPTURE_ULTRASONIC_CC_IDX DL_TIMER_CC_1_INDEX
#elif defined GPIO_CAPTURE_ULTRASONIC_C2_PORT
#define CAPTURE_ULTRASONIC_CC_IDX DL_TIMER_CC_2_INDEX
#elif defined GPIO_CAPTURE_ULTRASONIC_C3_PORT
#define CAPTURE_ULTRASONIC_CC_IDX DL_TIMER_CC_3_INDEX
#elif defined GPIO_CAPTURE_ULTRASONIC_C4_PORT
#define CAPTURE_ULTRASONIC_CC_IDX DL_TIMER_CC_4_INDEX
#elif defined GPIO_CAPTURE_ULTRASONIC_C5_PORT
#define CAPTURE_ULTRASONIC_CC_IDX DL_TIMER_CC_5_INDEX
#endif

class Ultrasonic_Gpio {
private:
    typedef enum {
        ULTRASONIC_IDLE,
        ULTRASONIC_TRIGGERING,
        ULTRASONIC_WAIT_RISE,
        ULTRASONIC_WAIT_FALL,
        ULTRASONIC_DONE
    } Ultrasonic_State;

    BspGpio_Instance echo_inst;
    BspGpio_Instance trig_inst;

    bool initialized = false;
    bool enabled = false;

    float distance = 0.0f;

    Ultrasonic_State state = ULTRASONIC_IDLE;
    uint64_t timeout_start_tick = 0;
    const uint32_t timeout_ms = 50;
    TimerHandle_t timer_handle = NULL;

    void SendTrigger(void);

    static void TimerCallback(TimerHandle_t xTimer);

public:
    void Init(GPIO_Regs *echo_port, uint32_t echo_pin, GPIO_Regs *trig_port, uint32_t trig_pin);

    void Enable();

    void Disable();

    float GetDistance();
};

// 写得比较草率，先用着，后面再完善
class Ultrasonic_Capture {
private:
    // 80MHz 主频：1us = 80 个时钟
    static constexpr float CYCLE_TO_US = 80.0f;
    // 声速换算系数(cm)：回波时间(us) * 0.017
    static constexpr float DIST_COEFF = 0.017f;
    // 有效测距范围 2cm ~ 400cm
    static constexpr float MIN_DIST_CM = 2.0f;
    static constexpr float MAX_DIST_CM = 400.0f;
    // 滤波配置
    static constexpr uint8_t FILTER_WINDOW = 5; // 滤波窗口
    static constexpr float FILTER_ALPHA = 0.3f; // 一阶滤波系数

    float last_filteredval = 0.0f; // 一阶滤波缓存
    // uint8_t filter_index = 0;               // 滑动窗口索引
    // float filter_buf[FILTER_WINDOW] = {0}; // 滤波缓存

    float distance = 0.0f;

    bool initialized = false;
    bool enabled = false;

    void ResetCapture();

public:
    void Init();

    void Enable();
    void Disable();

    float GetDistance();
};