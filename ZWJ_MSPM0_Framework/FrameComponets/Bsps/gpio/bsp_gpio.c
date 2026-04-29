#include "bsp_gpio.h"

#define BSPGPIO_MAX_CANINSTS 48

static BspGpio_Instance *bspgpio_insts[BSPGPIO_MAX_CANINSTS] = {NULL};
static uint8_t bspgpio_insts_count = 0;

/**
 * @brief 注册BSP_GPIO实例
 */
void BspGpio_InstRegister(BspGpio_Instance *inst, GPIO_Regs *port, uint32_t pin) {
    if (inst == NULL || port == NULL)
        return;
    inst->port = port;
    inst->pin = pin;

    // 检查重复注册
    if (bspgpio_insts_count < BSPGPIO_MAX_CANINSTS) {
        for (uint8_t i = 0; i < bspgpio_insts_count; i++) {
            if (bspgpio_insts[i]->port == port && bspgpio_insts[i]->pin == pin) {
                while (1)
                    ;
            }
        }
        bspgpio_insts[bspgpio_insts_count++] = inst;
    }
}

/**
 * @brief 获取GPIO引脚状态
 */
uint32_t BspGpio_GetState(BspGpio_Instance *inst) {
    if (inst == NULL) {
        return BSPGPIO_LOW_STATE;
    }
    return (DL_GPIO_readPins(inst->port, inst->pin) != 0) ? BSPGPIO_HIGH_STATE : BSPGPIO_LOW_STATE;
}

/**
 * @brief 设置GPIO引脚状态
 */
void BspGpio_SetState(BspGpio_Instance *inst, uint32_t state) {
    if (inst == NULL) {
        return;
    }
    if (state == BSPGPIO_HIGH_STATE) {
        DL_GPIO_setPins(inst->port, inst->pin);
    } else {
        DL_GPIO_clearPins(inst->port, inst->pin);
    }
}

/**
 * @brief 切换GPIO引脚状态
 */
void BspGpio_ToggleState(BspGpio_Instance *inst) {
    if (inst == NULL) {
        return;
    }
    DL_GPIO_togglePins(inst->port, inst->pin);
}