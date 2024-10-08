#ifndef LOGGER_ERROR_H_
#define LOGGER_ERROR_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    // SD card related failures
    E_SD_NONE,
    E_SD_FAIL_READ_BLOCK,
    E_SD_FAIL_GO_IDLE,
    E_SD_FAIL_SEND_IF_COND,
    E_SD_FAIL_VOLTAGE_CHECK,
    E_SD_FAIL_FS_INIT,
    E_SD_FAIL_READ_FILE,
    E_SD_FAIL_UNIMPLEMENTED_FUNCTION,
    E_SD_FAIL_WRITE_BLOCK,
    E_SD_FAIL_OPEN_FILE,
    E_SD_FAIL_WRITE_DATA_RESP,
    // syslog related failures
    E_SYSLOG_ALL_BUFFERS_FULL,
} cansw_logger_error_t;

void error(cansw_logger_error_t error);

cansw_logger_error_t get_last_error(void);

bool any_errors(void);

#endif // compile guard
