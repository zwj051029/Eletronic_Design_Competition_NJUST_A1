#include "oled.hpp"
#include "bsp_delay.h"
#include <type_traits>

void OLED::Init(GPIO_Regs *scl_port, uint32_t scl_pin, GPIO_Regs *sda_port, uint32_t sda_pin) {
    iic_inst.Init(scl_port, scl_pin, sda_port, sda_pin);
    this->initialized = true;
}

void OLED::Enable() {
    BspDelay_ms(100);

    WriteCmd(0xAE); // 关闭显示

    WriteCmd(0xD5); // 设置显示时钟分频比/振荡器频率
    WriteCmd(0x80);

    WriteCmd(0xA8); // 设置多路复用率
    WriteCmd(0x3F);

    WriteCmd(0xD3); // 设置显示偏移
    WriteCmd(0x00);

    WriteCmd(0x40); // 设置显示开始行

    WriteCmd(0xA1); // 设置左右方向，0xA1正常 0xA0左右反置

    WriteCmd(0xC8); // 设置上下方向，0xC8正常 0xC0上下反置

    WriteCmd(0xDA); // 设置COM引脚硬件配置
    WriteCmd(0x12);

    WriteCmd(0x81); // 设置对比度控制
    WriteCmd(0xCF);

    WriteCmd(0xD9); // 设置预充电周期
    WriteCmd(0xF1);

    WriteCmd(0xDB); // 设置VCOMH取消选择级别
    WriteCmd(0x30);

    WriteCmd(0xA4); // 设置整个显示打开/关闭

    WriteCmd(0xA6); // 设置正常/倒转显示

    WriteCmd(0x8D); // 设置充电泵
    WriteCmd(0x14);

    WriteCmd(0xAF); // 开启显示

    Clear(); // OLED清屏

    this->enabled = true;
}

void OLED::Disable() {
    this->enabled = false;
}

void OLED::Clear(void) {
    uint8_t i, j;
    for (j = 0; j < 8; j++) {
        SetCursor(j, 0);
        for (i = 0; i < 128; i++) {
            WriteData(0x00);
        }
    }
}

void OLED::WriteCmd(uint8_t cmd) {
    this->iic_inst.Start();
    this->iic_inst.SendByte(0X78);
    this->iic_inst.SendByte(0X00);
    this->iic_inst.SendByte(cmd);
    this->iic_inst.Stop();
}

void OLED::WriteData(uint8_t data) {
    this->iic_inst.Start();
    this->iic_inst.SendByte(0X78);
    this->iic_inst.SendByte(0X40);
    this->iic_inst.SendByte(data);
    this->iic_inst.Stop();
}

void OLED::SetCursor(uint8_t y, uint8_t x) {
    WriteCmd(0xB0 | y);                 // 设置Y位置
    WriteCmd(0x10 | ((x & 0xF0) >> 4)); // 设置X位置高4位
    WriteCmd(0x00 | (x & 0x0F));        // 设置X位置低4位
}

void OLED::Showchar(uint16_t x, uint16_t y, char param) {
    uint8_t i;
    SetCursor((x - 1) * 2, (y - 1) * 8); // 设置光标位置在上半部分
    for (i = 0; i < 8; i++) {
        WriteData(OLED_F8x16[param - ' '][i]); // 显示上半部分内容
    }
    SetCursor((x - 1) * 2 + 1, (y - 1) * 8); // 设置光标位置在下半部分
    for (i = 0; i < 8; i++) {
        WriteData(OLED_F8x16[param - ' '][i + 8]); // 显示下半部分内容
    }
}

void OLED::ShowString(uint16_t x, uint16_t y, string param) {
    if (param.empty())
        return;
    for (uint16_t i = 0; i < param.length(); ++i) {
        Showchar(x, y + i, param[i]); // 逐字符显示，列坐标 +1
    }
}

void OLED::ShowNum(uint16_t x, uint16_t y, uint16_t param) {
    string str = to_string(param);
    ShowString(x, y, str);
}

void OLED::ShowFloat(uint16_t x, uint16_t y, float param) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%.2f", param); // 格式化为两位小数
    ShowString(x, y, std::string(buf));
}