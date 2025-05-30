
#ifndef INC_WS2812_H_
#define INC_WS2812_H_

#include "main.h"

// Number of LEDs
#define noOfLEDs 59

// Timer Handle
extern TIM_HandleTypeDef htim3;

// PWM Data Array
extern uint8_t pwmData[24 * noOfLEDs + 50];

// Functions
void WS2812_ResetAllLED(void);
void WS2812_SetAllLED(void);
void WS2812_SetLED(int LEDposition, int Red, int Green, int Blue);
void WS2812_Send(void);

#endif /* INC_WS2812_H_ */
