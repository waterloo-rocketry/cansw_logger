#ifndef SD_H_
#define SD_H_

#include <stdint.h>

uint8_t init_fs();

void sd_card_log_to_file(const char *buffer, uint16_t len);

#endif // compile guard
