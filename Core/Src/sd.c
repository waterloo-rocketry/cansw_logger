#include <stdio.h>

#include "error.h"
#include "fatfs.h"
#include "main.h"
#include "platform.h"

static char GLOBAL_FILENAME[20];

FATFS fatfs;

void sd_card_log_to_file(const char *buffer, uint16_t length) {
    LED_GREEN_ON();
    FIL file;
    if (f_open(&file, GLOBAL_FILENAME, FA_WRITE | FA_OPEN_APPEND) != FR_OK) {
        error(E_SD_FAIL_OPEN_FILE);
    }

    unsigned int retval;

    if (f_write(&file, buffer, length, &retval) != FR_OK) {
        error(E_SD_FAIL_OPEN_FILE);
    }

    f_close(&file);
    LED_GREEN_OFF();
}

uint8_t init_fs() {
	f_mount(&fatfs, "", 0);

    // count the number of flies in the root directory of the SD card
    uint16_t root_dir_files = 0;
    DIR dir;
    if (f_opendir(&dir, "/") != FR_OK) {
        error(E_SD_FAIL_FS_INIT);
    }

    FILINFO finfo;
    while (f_readdir(&dir, &finfo) == FR_OK && finfo.fname[0] != '\0') {
        root_dir_files++;
    }
    f_closedir(&dir);

    sprintf(GLOBAL_FILENAME, "log_%04x.txt", root_dir_files);

    FIL file;
    if (f_open(&file, GLOBAL_FILENAME, FA_WRITE | FA_CREATE_NEW) != FR_OK) {
        error(E_SD_FAIL_READ_FILE);
    }

    char header[] = {'=', '=', 'W', 'A', 'T', 'E', 'R', 'L', 'O', 'O', ' ', 'R', 'O', 'C',
                     'K', 'E', 'T', 'R', 'Y', ' ', 'C', 'A', 'N', ' ', 'L', 'O', 'G', 'G',
                     'E', 'R', ' ', 'V', '0', '.', '2', '.', '0', '=', '=', '\n'};

    unsigned int retval;

    if (f_write(&file, header, sizeof(header), &retval) != FR_OK) {
        error(E_SD_FAIL_OPEN_FILE);
    }

    f_close(&file);

    return retval;
}
