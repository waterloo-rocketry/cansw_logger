#include "canlib.h"
#include "main.h"

#include "fs.h"
#include "log.h"

extern FDCAN_HandleTypeDef hfdcan1;

void can_callback_function(const can_msg_t *message, uint32_t) {
    log_handle_incoming_message(message);
}

void fwmain(void) {
    can_init_stm(&hfdcan1, can_callback_function);
    fs_init();
    log_init();

    for (;;) {
        log_heartbeat();
    }
}
