#include <stdint.h>
#include <stdio.h>

#include "fatfs.h"
#include "rocketlib.h"

#include "fs.h"
#include "log.h"
#include "main.h"
#include "platform.h"

extern SD_HandleTypeDef hsd1;
extern const struct lfs_config config;

#define MAX_FILE_PER_DIR 1000

static FATFS fatfs = {0};
static FIL logfile = {0};
lfs_t lfs_flash = {0};
lfs_file_t logfile_flash = {0};

static uint32_t index_counter = 0;
static uint32_t page_counter = 0;
static FRESULT fs_result = FR_OK;


static void fs_new_file(void) {
	unsigned int retval = 0;

	// Create directory as nessary
	if ((index_counter % MAX_FILE_PER_DIR) == 0) {
		char dir_name[100];
		sprintf(dir_name, "dir_%04lu", index_counter / MAX_FILE_PER_DIR);

		f_mkdir(dir_name);
		lfs_mkdir(&lfs_flash, dir_name);
	}

	// Choose file name
	char log_filename[100];
	sprintf(log_filename, "dir_%04lu/log_%04lu.bin", index_counter / MAX_FILE_PER_DIR,
	        index_counter % MAX_FILE_PER_DIR);

	++index_counter;

	// Update counter files (flash and sd)
	FIL counter_file_sd;
	lfs_file_t counter_file_flash;

	f_open(&counter_file_sd, "counter.bin", FA_WRITE | FA_CREATE_ALWAYS);
	f_write(&counter_file_sd, &index_counter, sizeof(index_counter), &retval);
	f_close(&counter_file_sd);

	fs_result = f_open(&logfile, log_filename, FA_WRITE | FA_OPEN_ALWAYS);
	
	lfs_file_open(&lfs_flash, &counter_file_flash, "/counter.bin", LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
	lfs_file_write(&lfs_flash, &counter_file_flash, &index_counter, sizeof(index_counter));
	lfs_file_close(&lfs_flash, &counter_file_flash);

	if (lfs_file_open(&lfs_flash, &logfile_flash, log_filename, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_EXCL) != 0) {
	}

	page_counter = 0;
}

w_status_t fs_init(void) {

	unsigned int retval = 0;
	uint32_t sd_index = 0;
	uint32_t flash_index = 0;

	HAL_SD_InitCard(&hsd1);

	if (f_mount(&fatfs, "", 0) != FR_OK) {
		return W_IO_ERROR;
	}

	// LittleFS mount
	if (lfs_mount(&lfs_flash, &config) < 0){
		return W_FAILURE;
	}

	FIL counter_file_sd;
	lfs_file_t counter_file_flash;
	
	// Read the file count counter
	if (f_open(&counter_file_sd, "counter.bin", FA_READ) == FR_OK) {
		f_read(&counter_file_sd, &sd_index, sizeof(index_counter), &retval);
	}
	f_close(&counter_file_sd);

	if (lfs_file_open(&lfs_flash, &counter_file_flash, "counter.bin", LFS_O_RDONLY) == 0) {
		lfs_file_read(&lfs_flash, &counter_file_flash, &flash_index, sizeof(index_counter));
		lfs_file_close(&lfs_flash, &counter_file_flash);
	}

	// set index counter to the larger index
	index_counter = (sd_index > flash_index) ? sd_index : flash_index;

	fs_new_file();

	return W_SUCCESS;
}

void fs_write_page(const uint8_t *page) {

	unsigned int retval;
	fs_result = f_write(&logfile, page, PAGE_SIZE, &retval);
	lfs_ssize_t flash_written = lfs_file_write(&lfs_flash, &logfile_flash, page, PAGE_SIZE);

	++page_counter;

	if (page_counter >= MAX_FILE_SIZE_PAGES) {
		f_close(&logfile);
		lfs_file_close(&lfs_flash, &logfile_flash);
		fs_new_file();
	} else {
		f_sync(&logfile);
		lfs_file_sync(&lfs_flash, &logfile_flash);
	}
}

uint32_t fs_get_log_written_size(void) {
	return page_counter * PAGE_SIZE;
}

uint32_t fs_get_sd_log_file_name(void) {
	// Because index_counter is file name of next file to be created, so decrement by 1 to get
	// current file name
	return index_counter - 1;
}

uint32_t fs_get_error(void) {
	if (fs_result != FR_OK) {
		return 1 << E_FS_ERROR_OFFSET;
	}
	return 0;
}
