#pragma once

#include "bsp_gpio.h"

class Ultrasonic {
private:
    BspGpio_Instance echo_inst;
    BspGpio_Instance trig_inst;

    bool initialized = false;
    bool enabled = false;

public:
    void Init(GPIO_Regs *echo_port, uint32_t echo_pin, GPIO_Regs *trig_port, uint32_t trig_pin);

    void Enable();

    void Disable();
};