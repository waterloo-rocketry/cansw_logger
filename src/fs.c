#include <common.h>
#include <stdint.h>
#include <stdio.h>

#include "fatfs.h"
#include "main.h"

#include "fs.h"

FATFS fatfs;

w_status_t fs_init(void) {
    unsigned int retval;

    f_mount(&fatfs, "", 0);

    // Read the file count counter
    FIL counter_file;
    uint32_t counter = 0;
    if (f_open(&counter_file, "counter.bin", FA_READ) == FR_OK) {
        f_read(&counter_file, &counter, sizeof(counter), &retval);
    }
    f_close(&counter_file);

    // Create directory as nessary
    if ((counter % 1000) == 0) {
        char dir_name[100];
        sprintf(dir_name, "dir_%04d", counter / 1000);
        f_mkdir(dir_name);
    }

    // Choose file name
    char log_filename[100];
    sprintf(log_filename, "dir_%04d/log_%04x.bin", counter / 1000, counter % 1000);

    // Update counter file
    ++counter;
    f_open(&counter_file, "counter.bin", FA_WRITE | FA_CREATE_ALWAYS);
    f_write(&counter_file, &counter, sizeof(counter), &retval);
    f_close(&counter_file);

    FIL file;
    if (f_open(&file, log_filename, FA_WRITE | FA_CREATE_NEW) != FR_OK) {}

    char header[] = {'=', '=', 'W', 'A', 'T', 'E', 'R', 'L', 'O', 'O', ' ', 'R', 'O', 'C',
                     'K', 'E', 'T', 'R', 'Y', ' ', 'C', 'A', 'N', ' ', 'L', 'O', 'G', 'G',
                     'E', 'R', ' ', 'V', '0', '.', '2', '.', '0', '=', '=', '\n'};

    if (f_write(&file, header, sizeof(header), &retval) != FR_OK) {}
    f_close(&file);

    return W_SUCCESS;
}
