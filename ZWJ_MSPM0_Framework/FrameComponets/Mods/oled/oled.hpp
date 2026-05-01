#pragma once

#include "bsp_iic.hpp"
#include "oled_font.hpp"
#include <string>

using namespace std;

class OLED {
private:
    SoftWare_IIC iic_inst;

    bool initialized = false;
    bool enabled = false;

    void WriteCmd(uint8_t cmd);
    void WriteData(uint8_t data);
    void SetCursor(uint8_t y, uint8_t x);

    void Showchar(uint16_t x, uint16_t y, char param);
    void ShowString(uint16_t x, uint16_t y, string param);
    void ShowNum(uint16_t x, uint16_t y, uint16_t param);
    void ShowFloat(uint16_t x, uint16_t y, float param);

    void Print(uint16_t x, uint16_t y, char c) {
        Showchar(x, y, c);
    }
    void Print(uint16_t x, uint16_t y, const string &s) {
        ShowString(x, y, s);
    }
    void Print(uint16_t x, uint16_t y, uint16_t num) {
        ShowNum(x, y, num);
    }
    void Print(uint16_t x, uint16_t y, float f) {
        ShowFloat(x, y, f);
    }

public:
    void Init(GPIO_Regs *scl_port, uint32_t scl_pin, GPIO_Regs *sda_port, uint32_t sda_pin);

    void Enable();
    void Disable();

    void Clear();

    template <typename T>
    void Show(uint16_t x, uint16_t y, T param) {
        if (!initialized || !enabled)
            return;
        Print(x, y, param); // 通过重载决议自动选择正确的 Print 版本
    }
};