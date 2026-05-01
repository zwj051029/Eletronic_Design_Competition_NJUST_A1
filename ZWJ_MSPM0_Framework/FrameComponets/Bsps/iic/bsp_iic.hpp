#pragma once

#include "bsp_gpio.h"

typedef enum {
    LOW_STATE = 0,
    HIGH_STATE = 1,
} IIC_State;

class SoftWare_IIC {
private:
    BspGpio_Instance scl_inst;
    BspGpio_Instance sda_inst;

    void SCL_SetState(IIC_State scl_state);
    void SDA_SetState(IIC_State sda_state);

public:
    void Init(GPIO_Regs *scl_port, uint32_t scl_pin, GPIO_Regs *sda_port, uint32_t sda_pin);

    void Start();
    void Stop();

    uint8_t ReadByte();
    uint8_t ReceiveAck(void);

    void SendByte(uint8_t byte);
    void SendAck(IIC_State Ack);
};

class HardWare_IIC {};