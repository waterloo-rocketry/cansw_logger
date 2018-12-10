#ifndef LOGGER_ERROR_H_
#define LOGGER_ERROR_H_

#include <stdint.h>

typedef enum {
    E_SD_NONE,
    E_SD_FAIL_READ_BLOCK,
    E_SD_FAIL_GO_IDLE,
    E_SD_FAIL_SEND_IF_COND,
    E_SD_FAIL_VOLTAGE_CHECK,
    E_SD_FAIL_FS_INIT,
    E_SD_FAIL_READ_FILE,
    E_SD_FAIL_UNIMPLEMENTED_FUNCTION,
    E_SD_FAIL_WRITE_BLOCK,
} cansw_logger_error_t;

void error(cansw_logger_error_t error);

#endif //compile guard
