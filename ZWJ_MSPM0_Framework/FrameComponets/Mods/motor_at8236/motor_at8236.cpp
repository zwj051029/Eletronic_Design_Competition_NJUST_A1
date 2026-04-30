#include "motor_at8236.hpp"

void MotorAT8236::Init(GPIO_Regs *encoderA_port, uint32_t encoderA_pin, 
                       GPIO_Regs *encoderB_port, uint32_t encoderB_pin,
                       uint16_t encoderLines = 1, uint16_t gearRatio = 1,
                       BspTIMPWM_TypeDef *PWMA_inst, GPTIMER_Regs *PWMA_htim, uint32_t PWMA_channel,
                       BspTIMPWM_TypeDef *PWMB_inst, GPTIMER_Regs *PWMB_htim, uint32_t PWMB_channel,
                       float max_speed)
{
    BspGpio_InstRegister(&encoderA_inst, encoderA_port, encoderA_pin);
    BspGpio_InstRegister(&encoderB_inst, encoderB_port, encoderB_pin);
    BspTIMPWM_InstRegist(&PWMA, PWMA_htim, PWMA_channel);
    BspTIMPWM_InstRegist(&PWMA, PWMB_htim, PWMB_channel);

    this->initialized = true;
            
                    
                  }
                  