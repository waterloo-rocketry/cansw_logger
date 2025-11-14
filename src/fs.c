#include <common.h>
#include <stdint.h>
#include <stdio.h>

#include "lfs.h"
#include "main.h"

#include "fs.h"
#include "log.h"
#include "littlefs_shim.h"

#define MAX_FILE_PER_DIR 1000

lfs_t lfs;
lfs_file_t logfile;

uint32_t index_counter = 0;
uint32_t page_counter = 0;

// configuration of the filesystem is provided by this struct
const struct lfs_config cfg = {
    // block device operations
    .read = sd_read,
    .prog = sd_write,
    .erase = sd_erase,
    .sync = sd_sync,

    // block device configuratio4n
    .read_size = 16,
    .prog_size = 16,
    .block_size = 4096,
    .block_count = 128,
    .cache_size = 16,
    .lookahead_size = 16,
    .block_cycles = 500,
};

static void fs_new_file(void) {
    //    unsigned int retval;

    // Create directory as nessary
    if ((index_counter % MAX_FILE_PER_DIR) == 0) {
        char dir_name[100];
        sprintf(dir_name, "dir_%04lu", index_counter / MAX_FILE_PER_DIR);
        lfs_mkdir(&lfs, dir_name);
    }

    // Choose file name
    char log_filename[100];
    sprintf(
        log_filename,
        "dir_%04lu/log_%04lu.bin",
        index_counter / MAX_FILE_PER_DIR,
        index_counter % MAX_FILE_PER_DIR
    );

    ++index_counter;

    // Update counter file
    lfs_file_t counter_file;
    lfs_file_open(&lfs, &counter_file, "counter.bin", LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
    lfs_file_write(&lfs, &counter_file, &index_counter, sizeof(index_counter));
    lfs_file_close(&lfs, &counter_file);

    if (lfs_file_open(&lfs, &logfile, log_filename, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_EXCL) != 0) {
    }

    page_counter = 0;
}

w_status_t fs_init(void) {
    //    unsigned int retval;
    if (lfs_format(&lfs, &cfg)) {
        return W_FAILURE;
    }

    if (lfs_mount(&lfs, &cfg) != 0) {
        return W_IO_ERROR;
    }

    // Read the file count counter
    lfs_file_t counter_file;
    if (lfs_file_open(&lfs, &counter_file, "counter.bin", LFS_O_RDONLY) == 0) {
        lfs_file_read(&lfs, &counter_file, &index_counter, sizeof(index_counter));
    }
    lfs_file_close(&lfs, &counter_file);

    fs_new_file();

    return W_SUCCESS;
}

void fs_write_page(const uint8_t *page) {
    //    unsigned int retval;
    if (lfs_file_write(&lfs, &logfile, page, PAGE_SIZE) != 0) {}
    ++page_counter;

    if (page_counter >= MAX_FILE_SIZE_PAGES) {
        lfs_file_close(&lfs, &logfile);
        fs_new_file();
    } else {
        lfs_file_sync(&lfs, &logfile);
    }
}
