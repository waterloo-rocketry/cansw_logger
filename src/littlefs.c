#include "lfs.h"
#include "mx25l25645g.h"
#include "platform.h"

#define FLASH_TIMEOUT_MS 100U
#define LFS_CACHE_SIZE 1U
#define LFS_LOOKAHEAD_SIZE 256U

extern mx25l25645g_t flash;

uint8_t read_buffer[LFS_CACHE_SIZE];
uint8_t prog_buffer[LFS_CACHE_SIZE];
uint32_t lookahead_buffer[LFS_LOOKAHEAD_SIZE / 4];

static int lfs_flash_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size){
    
    uint32_t address = block * c->block_size + off;
    
    if (mx25l25645g_quad_read_dma(&flash, (uint8_t *)buffer, address, size) != W_SUCCESS){
        return LFS_ERR_IO;
    }

    uint32_t time_start = millis();

    while (mx25l25645g_rx_in_progress(&flash)){
        if (millis() - time_start >= FLASH_TIMEOUT_MS){
            
            // TODO: handle failed abort
            mx25l25645g_abort(&flash);
            return LFS_ERR_IO;
        }
    }

    if (mx25l25645g_txrx_failed(&flash)){
        return LFS_ERR_IO;
    }

    return 0;
}

static int lfs_flash_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size){

    uint32_t address = block * c->block_size + off;

    if (mx25l25645g_write_dma(&flash, (uint8_t *)buffer, address, size) != W_SUCCESS){
        return LFS_ERR_IO;
    }

    uint32_t time_start = millis();

    while (mx25l25645g_tx_in_progress(&flash)){
        if (millis() - time_start >= FLASH_TIMEOUT_MS){
            
            // TODO: handle failed abort
            mx25l25645g_abort(&flash);
            return LFS_ERR_IO;
        }
    }

    if (mx25l25645g_txrx_failed(&flash)){
        return LFS_ERR_IO;
    }

    time_start = millis();

    for (;;) {
        mx25l25645g_status_t status = mx25l25645g_check_programming(&flash);

        if (status == MX25L25645G_STATUS_OK) {
            break;
        }

        if (status == MX25L25645G_STATUS_ERROR) {
            return LFS_ERR_IO;
        }

        if (millis() - time_start >= FLASH_TIMEOUT_MS){
            return LFS_ERR_IO;
        }

        // busy, so check again
    }

    return 0;
}

static int lfs_flash_erase(const struct lfs_config *c, lfs_block_t block){

    uint32_t address = block * c->block_size;

    if (mx25l25645g_erase_sector(&flash, address) != W_SUCCESS){
        return LFS_ERR_IO;
    }

    return 0;
}

static int lfs_flash_sync(const struct lfs_config *c){

    return 0;
}

const struct lfs_config config = {

    .read = lfs_flash_read,
    .prog = lfs_flash_prog,
    .erase = lfs_flash_erase,
    .sync = lfs_flash_sync,

    .read_buffer = read_buffer,
    .prog_buffer = prog_buffer, 
    .lookahead_buffer = lookahead_buffer,
    .read_size = 1U,
    .prog_size = 1U,
    .block_size = 4096U,
    .block_count = 8192U,
    .block_cycles = -1,
    .cache_size = LFS_CACHE_SIZE,
    .lookahead_size = LFS_LOOKAHEAD_SIZE,
    .compact_thresh = -1,
    .name_max = 0,
    .file_max = 0,
    .attr_max = 0,
    .metadata_max = 0, 
    .inline_max = -1
};
