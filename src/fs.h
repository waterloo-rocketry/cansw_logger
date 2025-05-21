#ifndef FS_H
#define FS_H

#include <common.h>

w_status_t fs_init(void);
void fs_write_page(const uint8_t *page);

#endif
