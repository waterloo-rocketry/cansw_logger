#include "can_syslog.h"
#include "sd.h"
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

//private functions
static uint_fast8_t can_message_to_buffer(uint32_t timestamp,
                                          const can_msg_t *message,
                                          char *buffer);

/* CAN logged message format:
 * III L: XX XX XX XX XX XX XX XX TTTTTTTTn
 * 1       10        20        30        40
 * I = SID, 3 bytes
 * L = length, 1 byte
 * X = data, 8 bytes (always). replaced by spaces if no data present
 * T = timestamp, in microseconds. 32 bits unsigned gives us about
 *     one hour before rollover. More than enough. Probably.
 * n = newline character
 */

#define MESSAGE_LENGTH_CHARS 40
#define CAN_LOG_BUFFERS       3

struct log_buffer {
    bool ready_to_log;
    uint16_t buffer_index;
    char buffer[512];
};

static struct log_buffer log_buffers[CAN_LOG_BUFFERS];

//if you aren't an ISR, don't fuck with this variable
uint8_t _log_into_index;

//public functions
void init_can_syslog(void) {
    _log_into_index = 0;
    uint8_t i;
    for(i = 0; i < CAN_LOG_BUFFERS; ++i) {
        log_buffers[i].ready_to_log = false;
        log_buffers[i].buffer_index = 0;
    }
}

void handle_can_interrupt(const can_msg_t *message) {
    //we're gonna copy this message into log_buffers[_log_into_index]
    struct log_buffer *buf = &(log_buffers[_log_into_index]);

    //check if there's room in this one
    if(512 - buf->buffer_index < MESSAGE_LENGTH_CHARS) {
        //todo, fix this
    }

    uint8_t step_ahead = can_message_to_buffer(0xcafebabe,
                                               message,
                                               buf->buffer + buf->buffer_index);
    buf->buffer_index += step_ahead;
}

void force_log_everything(void) {
    //TODO
    sd_card_log_to_file(log_buffers[0].buffer, log_buffers[0].buffer_index);
}

void can_syslog_heartbeat(void) {
    //TODO
    while(1);
    
    uint8_t i;
    for(i = 0; i < CAN_LOG_BUFFERS; ++i) {
        if(log_buffers[i].ready_to_log) {
            sd_card_log_to_file(log_buffers[i].buffer,
                         strlen(log_buffers[i].buffer));
            memset(log_buffers[i].buffer, 0, sizeof(log_buffers[i].buffer));
            log_buffers[i].ready_to_log = false;
        }
    }
}

/*
 * This function converts a CAN message to the following character
 * format and puts it in buffer. The message format has 40 bytes in
 * it, the caller is responsible for making sure that there is at
 * least that much memory available
 */
static uint_fast8_t can_message_to_buffer(uint32_t timestamp,
                                          const can_msg_t *message,
                                          char *buffer) {
    const char nibble_to_char[] = {
        '0', '1', '2', '3',
        '4', '5', '6', '7',
        '8', '9', 'A', 'B',
        'C', 'D', 'E', 'F'
    };

    //write three bytes of SID
    buffer[0] = nibble_to_char[(message->sid >> 8) & 0xf];
    buffer[1] = nibble_to_char[(message->sid >> 4) & 0xf];
    buffer[2] = nibble_to_char[message->sid & 0xf];

    //write a space, then the length, then a colon
    buffer[3] = ' ';
    buffer[4] = nibble_to_char[message->data_len & 0xf];
    buffer[5] = ':';

    //write a space, then the data
    buffer[6] = ' ';
    uint8_t i;
    for(i = 0; i < 8; ++i) {
        if(message->data_len >= (8-i)) {
            //I am so sorry.
            buffer[7 + 3*i] = nibble_to_char[(message->data[message->data_len - 8 + i] >> 4) & 0xf];
            buffer[7 + 3*i + 1] = nibble_to_char[message->data[message->data_len - 8 + i] & 0xf];
        } else {
            //no data to write, put a blank
            buffer[7 + 3*i] = ' ';
            buffer[7 + 3*i + 1] = ' ';
        }
        //write a space following the data
        buffer[7 + 3*i + 2] = ' ';
    }

    //write the timestamp. Max index the for loop will write to is
    //6+3*7+2 = 30, so we start at 31.
    buffer[31] = nibble_to_char[(timestamp >> 28) & 0xf];
    buffer[32] = nibble_to_char[(timestamp >> 24) & 0xf];
    buffer[33] = nibble_to_char[(timestamp >> 20) & 0xf];
    buffer[34] = nibble_to_char[(timestamp >> 16) & 0xf];
    buffer[35] = nibble_to_char[(timestamp >> 12) & 0xf];
    buffer[36] = nibble_to_char[(timestamp >>  8) & 0xf];
    buffer[37] = nibble_to_char[(timestamp >>  4) & 0xf];
    buffer[38] = nibble_to_char[(timestamp >>  0) & 0xf];

    // then the newline character
    buffer[39] = '\n';

    //return the length of string we just wrote. So a hardcoded 40
    return 40;
}
