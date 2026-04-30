#pragma once

#include "FreeRTOS.h"
#include "bsp_dwt.h"
#include "bsp_gpio.h"
#include "timers.h"

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