#include "can_syslog.h"
#include "error.h"
#include "platform.h"
#include "sd.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// private functions
static void log_can_buffer(uint8_t index);
static bool is_can_buffer_full(uint8_t index);
static uint_fast8_t can_message_to_buffer(uint32_t timestamp, const can_msg_t *message,
                                          char *buffer);

/* CAN logged message format:
 * IIIXXXXXXXXXXXXXXXXn
 * 1       10        20
 * I = SID, 3 bytes
 * X = data, 2 hex characters per bytes and up to 8 bytes
 * n = newline character
 */

#define MESSAGE_LENGTH_CHARS 20
#define CAN_LOG_BUFFERS 3
#define CAN_BUFFER_SIZE 3072

struct log_buffer {
    bool ready_to_log;
    uint16_t buffer_index;
    char buffer[CAN_BUFFER_SIZE];
};

static struct log_buffer log_buffers[CAN_LOG_BUFFERS];

// if you aren't an ISR, don't fuck with this variable
uint8_t _log_into_index;

// public functions
void init_can_syslog(void) {
    _log_into_index = 0;
    uint8_t i;
    for (i = 0; i < CAN_LOG_BUFFERS; ++i) {
        log_buffers[i].ready_to_log = false;
        log_buffers[i].buffer_index = 0;
    }
}

void handle_can_interrupt(const can_msg_t *message) {
    // if we're pointing at a full buffer, then all the buffers are full
    if (log_buffers[_log_into_index].ready_to_log) {
        // there's nothing we can do. Report an error and return
        error(E_SYSLOG_ALL_BUFFERS_FULL);
        return;
    } else {
        // copy the message into this buffer
        struct log_buffer *buf = &(log_buffers[_log_into_index]);
        uint8_t step_ahead =
            can_message_to_buffer(micros(), message, buf->buffer + buf->buffer_index);
        buf->buffer_index += step_ahead;
        // check if that caused the buffer to become full
        if (is_can_buffer_full(_log_into_index)) {
            // mark it as ready to log and increment buffer index
            buf->ready_to_log = true;
            _log_into_index = ((_log_into_index + 1) % CAN_LOG_BUFFERS);
        }
    }
}

void force_log_everything(void) {
    // go through each buffer. If there's a byte of data in them, log the buffer
    uint8_t i;
    for (i = 0; i < CAN_LOG_BUFFERS; ++i) {
        if (log_buffers[i].buffer_index != 0) {
            log_can_buffer(i);
        }
    }
}

void can_syslog_heartbeat(void) {
    uint8_t i;
    for (i = 0; i < CAN_LOG_BUFFERS; ++i) {
        uint8_t j = (_log_into_index + i) % CAN_LOG_BUFFERS;
        if (log_buffers[j].ready_to_log) {
            LED_GREEN_ON();
            log_can_buffer(j);
            LED_GREEN_OFF();
        }
    }
}

/*
 * This function converts a CAN message to the following character
 * format and puts it in buffer. The message format has 40 bytes in
 * it, the caller is responsible for making sure that there is at
 * least that much memory available
 */
static uint_fast8_t can_message_to_buffer(uint32_t timestamp, const can_msg_t *message,
                                          char *buffer) {
    const char nibble_to_char[] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    // write three bytes of SID
    buffer[0] = nibble_to_char[(message->sid >> 8) & 0xf];
    buffer[1] = nibble_to_char[(message->sid >> 4) & 0xf];
    buffer[2] = nibble_to_char[message->sid & 0xf];

    // write the data
    uint8_t i;
    for (i = 0; i < message->data_len; ++i) {
        buffer[3 + 2 * i] = nibble_to_char[(message->data[i] >> 4) & 0xf];
        buffer[3 + 2 * i + 1] = nibble_to_char[message->data[i] & 0xf];
    }
    buffer[3 + 2 * i] = '\n';

    // return the length of string we just wrote.
    return 3 + 2 * i + 1;
}

/*
 * This function logs all of the characters in log_buffers[index]
 * onto the SD card. It then resets buffer_index in that buffer to
 * zero. Note that this function does not check ready_to_log bit,
 * it assumes that the caller wants this buffer logged regardless
 */
static void log_can_buffer(uint8_t index) {
    if (index >= CAN_LOG_BUFFERS) {
        return;
    }
    if (log_buffers[index].buffer_index == 0) {
        return;
    }
    sd_card_log_to_file(log_buffers[index].buffer, log_buffers[index].buffer_index);
    memset(log_buffers[index].buffer, 0, sizeof(log_buffers[index].buffer));
    log_buffers[index].buffer_index = 0;
    log_buffers[index].ready_to_log = false;
}

static bool is_can_buffer_full(uint8_t index) {
    if (index >= CAN_LOG_BUFFERS || log_buffers[index].buffer_index >= CAN_BUFFER_SIZE ||
        (CAN_BUFFER_SIZE - log_buffers[index].buffer_index) <= MESSAGE_LENGTH_CHARS) {
        return true;
    } else {
        return false;
    }
}
