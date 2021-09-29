MISRAC_DISABLE
#include <stm32l4xx_hal.h>
#include <stm32l4xx_hal_def.h>
#include <os.h>
#include <Types.h>
MISRAC_ENABLE

typedef enum : uint8_t
{
    E_VALVE_FUNCTION_SHUTDOWN =0,
    E_VALVE_FUNCTION_BRAKE,
    E_VALVE_FUNCTION_REVERSE,
    E_VALVE_FUNCTION_FORWARD,
    E_VALVE_FUNCTION_CURRUENT_REG1,
    E_VALVE_FUNCTION_CURRENT_REG2
}eValveFunctions;

typedef enum: uint8_t
{  
    VALVE_STATE_ON = 0,
    VALVE_STATE_OFF
}eValveState_t;

class DValve
{
 private:
    
   TIM_HandleTypeDef* timer;
   uint32_t timChannel;
   GPIO_TypeDef* portName ;
   uint16_t pinNumber ;
   eValveState_t currentValveState;
public:
 DValve( TIM_HandleTypeDef* tim,uint32_t channel,GPIO_TypeDef* dirPort, uint16_t dirPin);
 void triggerValve(eValveState_t valveState);
 void valveTest(eValveFunctions valFunction);

};