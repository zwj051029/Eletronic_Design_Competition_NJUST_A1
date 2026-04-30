#ifndef __BSP_DELAY_H__
#define __BSP_DELAY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void BspDelay_us(uint32_t us);
void BspDelay_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif