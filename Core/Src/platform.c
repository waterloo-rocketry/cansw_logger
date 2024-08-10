#include "platform.h"

uint32_t millis(void) {
    return HAL_GetTick();
}

uint32_t micros(void) {
    return 0;
}
