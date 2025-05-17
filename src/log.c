#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "log.h"

static size_t active_buffer = 0;
static uint8_t sequence_number = 0;

struct {
    size_t pointer;
    uint8_t buffer[4096];
    bool ready_to_write;
} log_buffer[4];

w_status_t log_init(void) {
    for (int i = 0; i < 4; i++) {
        log_buffer[i].pointer = 0;
        log_buffer[0].ready_to_write = false;
    }

    return W_SUCCESS;
}

void log_handle_incoming_message(const can_msg_t *msg) {
    size_t pointer = log_buffer[active_buffer].pointer;

    if ((4096 - pointer) < (9 + msg->data_len)) { // Not enough space in buffer
        // Fill remaining space with 0xff
        for (; pointer < 4096; pointer++) {
            log_buffer[active_buffer].buffer[pointer] = 0xff;
        }
        // Label buffer ready for review
        log_buffer[active_buffer].ready_to_write = true;
        // Increment active buffer
        active_buffer = (active_buffer >= 3) ? 0 : active_buffer + 1;
        // Increment sequence number
        ++sequence_number;
    }

    pointer = log_buffer[active_buffer].pointer;

    // Pointer should not point in page signature
    w_assert((pointer == 0) || (pointer >= 4));

    if (pointer < 4) {
        // Write page signature
        log_buffer[active_buffer].buffer[0] = 'L';
        log_buffer[active_buffer].buffer[1] = 'O';
        log_buffer[active_buffer].buffer[2] = 'G';
        log_buffer[active_buffer].buffer[3] = sequence_number & 0xff;
        log_buffer[active_buffer].pointer = 4;
    }

    pointer = log_buffer[active_buffer].pointer;

    // Write serialized CAN message
    log_buffer[active_buffer].buffer[pointer + 0] = msg->sid & 0xff;
    log_buffer[active_buffer].buffer[pointer + 1] = (msg->sid >> 8) & 0xff;
    log_buffer[active_buffer].buffer[pointer + 2] = (msg->sid >> 16) & 0xff;
    log_buffer[active_buffer].buffer[pointer + 3] = (msg->sid >> 24) & 0xff;
    log_buffer[active_buffer].buffer[pointer + 8] = msg->data_len;
    memcpy(log_buffer[active_buffer].buffer + pointer + 9, msg->data, msg->data_len);
    log_buffer[active_buffer].pointer += 9 + msg->data_len;
}

void log_heartbeat(void){
	
}
