#include "std_cpp.h"
#include "ti_drivers_config.h"
#include "ti_msp_dl_config.h"
#include <FreeRTOS.h>
#include <pthread.h>
#include <stdint.h>
#include <task.h>

#ifdef __ICCARM__
#include <DLib_Threads.h>
#endif

extern void *mainThread(void *arg0);

#define THREADSTACKSIZE configMINIMAL_STACK_SIZE * 4

static void prvSetupHardware(void);
void ControlTask(void *argument);

int main(void) {
    BaseType_t xReturn; // 任务创建返回值
    prvSetupHardware();

    MainInitCpp();

    xReturn = xTaskCreate(ControlTask, /* pxTaskCode:    任务函数 */
                          "Control",   /* pcName:        任务名称 */
                          512,         /* uxStackDepth:  栈大小（字） */
                          NULL,        /* pvParameters:  传递给任务的参数 */
                          5,           /* uxPriority:    任务优先级 */
                          NULL         /* pxCreatedTask: 任务句柄（不需要则为NULL） */
    );
    configASSERT(xReturn == pdPASS); // 创建失败则断言报错

    vTaskStartScheduler();

    return (0);
}

void ControlTask(void *argument) {
    (void) argument;
    ControlCpp();
}

static void prvSetupHardware(void) {
    SYSCFG_DL_init();
}

void vApplicationMallocFailedHook(void) {
    taskDISABLE_INTERRUPTS();
    for (;;)
        ;
}

void vApplicationIdleHook(void) {
}

#if (configCHECK_FOR_STACK_OVERFLOW)

#if defined(__IAR_SYSTEMS_ICC__)
__weak void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
#elif (defined(__TI_COMPILER_VERSION__))
#pragma WEAK(vApplicationStackOverflowHook)
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
#elif (defined(__GNUC__) || defined(__ti_version__))
void __attribute__((weak)) vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
#endif
{
    (void) pcTaskName;
    (void) pxTask;

    taskDISABLE_INTERRUPTS();
    for (;;)
        ;
}
#endif
