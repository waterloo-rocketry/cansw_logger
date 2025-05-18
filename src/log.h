#ifndef LOG_H
#define LOG_H

#include <common.h>

#include "canlib.h"

#define PAGE_SIZE 4096
#define NUM_BUFFERS 4
#define SIGNATURE_SIZE 4

w_status_t log_init(void);
void log_handle_incoming_message(const can_msg_t *msg);
void log_heartbeat(void);

#endif
