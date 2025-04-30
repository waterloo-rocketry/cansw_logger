#ifndef PLATFORM_H
#define PLATFORM_H

#include "main.h"

#define LED_RED_ON() (HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET))
#define LED_RED_OFF() (HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET))

#define LED_GREEN_ON() (HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET))
#define LED_GREEN_OFF() (HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET))

// number of milliseconds since bootup
static inline uint32_t millis(void) {
    return HAL_GetTick();
};

#endif /* PLATFORM_H */
