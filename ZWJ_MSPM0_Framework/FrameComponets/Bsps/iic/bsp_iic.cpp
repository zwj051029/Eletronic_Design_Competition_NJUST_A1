#include "bsp_iic.hpp"
#include "bsp_delay.h"

void SoftWare_IIC::Init(GPIO_Regs *scl_port, uint32_t scl_pin, GPIO_Regs *sda_port, uint32_t sda_pin) {
    if (scl_port == NULL || sda_port == NULL) {
        return;
    }

    BspGpio_InstRegister(&scl_inst, scl_port, scl_pin);
    BspGpio_InstRegister(&sda_inst, sda_port, sda_pin);
}

void SoftWare_IIC::SCL_SetState(IIC_State scl_state) {
    BspGpio_SetState(&scl_inst, scl_state);
}

void SoftWare_IIC::SDA_SetState(IIC_State sda_state) {
    BspGpio_SetState(&sda_inst, sda_state);
}

void SoftWare_IIC::Start() {
    SDA_SetState(HIGH_STATE);
    SCL_SetState(HIGH_STATE);
    BspDelay_us(2);
    SDA_SetState(LOW_STATE);
    BspDelay_us(2);
    SCL_SetState(LOW_STATE);
}

void SoftWare_IIC::Stop() {
    SDA_SetState(LOW_STATE);
    SCL_SetState(HIGH_STATE);
    BspDelay_us(2);
    SDA_SetState(HIGH_STATE);
    BspDelay_us(2);
}

void SoftWare_IIC::SendByte(uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
        SDA_SetState(IIC_State(!!(byte & (0x80 >> i))));
        SCL_SetState(HIGH_STATE);
        SCL_SetState(LOW_STATE);
    }

    SDA_SetState(HIGH_STATE);
    BspDelay_us(2);
    SCL_SetState(HIGH_STATE);
    BspDelay_us(2);
    SCL_SetState(LOW_STATE);
}

uint8_t SoftWare_IIC::ReadByte() {
    uint8_t byte = 0;
    SDA_SetState(HIGH_STATE);

    for (uint8_t i = 0; i < 8; i++) {
        SCL_SetState(HIGH_STATE);
        byte = (byte << 1) | BspGpio_GetState(&sda_inst);
        SCL_SetState(LOW_STATE);
    }

    return byte;
}

void SoftWare_IIC::SendAck(IIC_State Ack) {
    SDA_SetState(Ack);        // 主机把应答位数据放到SDA线
    SCL_SetState(HIGH_STATE); // 释放SCL，从机在SCL高电平期间，读取应答位
    SCL_SetState(LOW_STATE);  // 拉低SCL，开始下一个时序模块
}

uint8_t SoftWare_IIC::ReceiveAck(void) {
    uint8_t Ack;              // 定义应答位变量
    SDA_SetState(HIGH_STATE); // 接收前，主机先确保释放SDA，避免干扰从机的数据发送
    SCL_SetState(HIGH_STATE); // 释放SCL，主机机在SCL高电平期间读取SDA

    Ack = BspGpio_GetState(&sda_inst); // 将应答位存储到变量里
    SCL_SetState(LOW_STATE);           // 拉低SCL，开始下一个时序模块
    return Ack;                        // 返回定义应答位变量
}
