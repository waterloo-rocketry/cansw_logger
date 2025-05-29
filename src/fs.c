#include <common.h>
#include <stdint.h>
#include <stdio.h>

#include "fatfs.h"
#include "main.h"

#include "fs.h"
#include "log.h"

#define MAX_FILE_PER_DIR 1000

FATFS fatfs;
FIL logfile;

uint32_t index_counter = 0;
uint32_t page_counter = 0;

static void fs_new_file(void) {
    unsigned int retval;

    // Create directory as nessary
    if ((index_counter % MAX_FILE_PER_DIR) == 0) {
        char dir_name[100];
        sprintf(dir_name, "dir_%04lu", index_counter / MAX_FILE_PER_DIR);
        f_mkdir(dir_name);
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
    FIL counter_file;
    f_open(&counter_file, "counter.bin", FA_WRITE | FA_CREATE_ALWAYS);
    f_write(&counter_file, &index_counter, sizeof(index_counter), &retval);
    f_close(&counter_file);

    if (f_open(&logfile, log_filename, FA_WRITE | FA_CREATE_NEW) != FR_OK) {}

    page_counter = 0;
}

w_status_t fs_init(void) {
    unsigned int retval;

    if (f_mount(&fatfs, "", 0) != FR_OK) {
        return W_IO_ERROR;
    }

    // Read the file count counter
    FIL counter_file;
    if (f_open(&counter_file, "counter.bin", FA_READ) == FR_OK) {
        f_read(&counter_file, &index_counter, sizeof(index_counter), &retval);
    }
    f_close(&counter_file);

    fs_new_file();

    return W_SUCCESS;
}

void fs_write_page(const uint8_t *page) {
    unsigned int retval;
    if (f_write(&logfile, page, PAGE_SIZE, &retval) != FR_OK) {}
    ++page_counter;

    if (page_counter >= MAX_FILE_SIZE_PAGES) {
        f_close(&logfile);
        fs_new_file();
    } else {
        f_sync(&logfile);
    }
}
