#ifndef __BSP_GPIO_H__
#define __BSP_GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ti_msp_dl_config.h"

#define BSPGPIO_HIGH_STATE 1
#define BSPGPIO_LOW_STATE 0

// BSP_GPIO实例结构体（极简版，只保留必要成员）
typedef struct {
    GPIO_Regs *port;
    uint32_t pin;
} BspGpio_Instance;

/**
 * @brief 注册BSP_GPIO实例（极简版，无中断）
 */
void BspGpio_InstRegister(BspGpio_Instance *inst, GPIO_Regs *port, uint32_t pin);

/**
 * @brief 设置GPIO引脚状态
 */
void BspGpio_SetState(BspGpio_Instance *inst, uint32_t state);

/**
 * @brief 切换GPIO引脚状态
 */
void BspGpio_ToggleState(BspGpio_Instance *inst);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_GPIO_H__ */