#ifndef LOG_H
#define LOG_H

#include <common.h>

#include "canlib.h"

w_status_t log_init(void);
void log_handle_incoming_message(const can_msg_t *msg);
void log_heartbeat(void);

#endif
