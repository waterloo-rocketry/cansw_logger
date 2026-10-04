#include "lfs.h"
#include "mx25l25645g.h"

extern mx25l25645g_t flash;

int lfs_flash_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size){
    
    uint32_t address = block * c->block_size + off;
    mx25l25645g_quad_read(&flash, (uint8_t *)buffer, address, size);

    return 0;
}

int lfs_flash_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size){

    uint32_t address = block * c->block_size + off;
    mx25l25645g_write(&flash, (uint8_t *)buffer, address, size);

    return 0;
}

int lfs_flash_erase(const struct lfs_config *c, lfs_block_t block){

    uint32_t address = block * c->block_size;
    mx25l25645g_erase_sector(&flash, address);

    return 0;
}

int lfs_flash_sync(const struct lfs_config *c){

    return 0;
}

const struct lfs_config config = {

    .read = lfs_flash_read,
    .prog = lfs_flash_prog,
    .erase = lfs_flash_erase,
    .sync = lfs_flash_sync,

    .read_size = 1U,
    .prog_size = 1U,
    .block_size = 4096U,
    .block_count = 8192U,
    .block_cycles = -1,
    .cache_size = 1U,
    .lookahead_size = 256U,
    .compact_thresh = -1,
    .name_max = 0,
    .file_max = 0,
    .attr_max = 0,
    .metadata_max = 0, 
    .inline_max = -1
};
