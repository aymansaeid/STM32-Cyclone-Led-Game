#include "ws2812.h"

// PWM Data Array
uint8_t pwmData[24 * noOfLEDs + 50];

/**
 * @brief  This function is called when the PWM pulse is finished.
 * @param  htim: pointer to a TIM_HandleTypeDef structure that contains
 * the configuration information for the specified TIM module.
 * @retval None
 */
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM3) {
		HAL_TIM_PWM_Stop_DMA(&htim3, TIM_CHANNEL_1);
		htim3.Instance->CCR1 = 0;
	}
}

/**
 * @brief  Resets all LEDs to an off state.
 * @retval None
 */
void WS2812_ResetAllLED(void) {
	for (int i = 0; i < 24 * noOfLEDs; i++) {
		pwmData[i] = 1; // Corresponds to a "0" bit for WS2812
	}
}

/**
 * @brief  Sets all LEDs to a default color (white).
 * @retval None
 */
void WS2812_SetAllLED(void) {
	for (int i = 0; i < 24 * noOfLEDs; i++) {
		pwmData[i] = 2; // Corresponds to a "1" bit for WS2812
	}
}

/**
 * @brief  Sets the color of a specific LED.
 * @param  LEDposition: The position of the LED in the strip.
 * @param  Red: The red color value (0-255).
 * @param  Green: The green color value (0-255).
 * @param  Blue: The blue color value (0-255).
 * @retval None
 */
void WS2812_SetLED(int LEDposition, int Red, int Green, int Blue) {
	for (int i = 7; i >= 0; i--) { // Set the first 8 out of 24 to green
		pwmData[24 * LEDposition + 7 - i] = ((Green >> i) & 1) + 1;
	}
	for (int i = 7; i >= 0; i--) { // Set the second 8 out of 24 to red
		pwmData[24 * LEDposition + 15 - i] = ((Red >> i) & 1) + 1;
	}
	for (int i = 7; i >= 0; i--) { // Set the third 8 out of 24 to blue
		pwmData[24 * LEDposition + 23 - i] = ((Blue >> i) & 1) + 1;
	}
}

/**
 * @brief  Sends the PWM data to the WS2812 LED strip using DMA.
 * @retval None
 */
void WS2812_Send(void) {
	HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_1, (uint32_t*) pwmData,
			24 * noOfLEDs + 50);
}
