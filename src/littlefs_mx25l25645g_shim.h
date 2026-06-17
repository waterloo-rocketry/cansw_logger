#ifndef LITTLEFS_MX25L25645G_SHIM_H
#define LITTLEFS_MX25L25645G_SHIM_H

#include <stdint.h>

#include "common.h"

#include "lfs.h"

w_status_t lfsshim_mx25l25645g_mount(lfs_t *lfs, uint32_t first_block_offset);
w_status_t lfsshim_mx25l25645g_mount_mbr(lfs_t *lfs);

#endif
