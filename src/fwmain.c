#include "canlib.h"
#include "main.h"

#include "fs.h"

extern FDCAN_HandleTypeDef hfdcan1;

void can_callback_function(const can_msg_t *message, uint32_t) {}

void fwmain(void) {
    can_init_stm(&hfdcan1, can_callback_function);

    fs_init();

    for (;;) {}
}
