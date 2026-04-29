#include "ultrasonic.hpp"

void Ultrasonic::Init(GPIO_Regs *echo_port, uint32_t echo_pin, GPIO_Regs *trig_port, uint32_t trig_pin) {
    BspGpio_InstRegister(&echo_inst, echo_port, echo_pin);
    BspGpio_InstRegister(&trig_inst, trig_port, trig_pin);

    this->initialized = true;
}

void Ultrasonic::Enable() {
    if (!(this->initialized))
        return;

    this->enabled = true;
}

void Ultrasonic::Disable() {
    if (!(this->initialized))
        return;

    this->enabled = false;
}
