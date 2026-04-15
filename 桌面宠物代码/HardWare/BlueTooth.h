#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

extern uint16_t Action_Mode;
extern uint16_t Face_Mode;
extern uint16_t SpeedDelay;
extern uint16_t SwingDelay;
extern uint8_t WeiBa;
extern uint16_t AllLed;  //开启灯光
extern uint16_t BreatheLed;//开启呼吸灯
extern uint16_t Sustainedmove;
void BlueTooth_Init(void);
extern uint8_t USART1_RxFlag;
extern uint8_t USART1_RxData;
#endif
