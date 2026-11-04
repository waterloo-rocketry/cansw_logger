#include <common.h>
#include <stdint.h>
#include <stdio.h>

#include "lfs.h"
#include "main.h"

#include "fs.h"
#include "log.h"
#include "mbr.h"
#include "stm32/littlefs_sd_shim.h"

extern SD_HandleTypeDef hsd1;
extern const struct lfs_config config;

#define MAX_FILE_PER_DIR 1000

lfs_t lfs_sd;
lfs_t lfs_flash;

lfs_file_t logfile_sd;
lfs_file_t logfile_flash;

uint32_t index_counter = 0;
uint32_t page_counter = 0;


static void fs_new_file(void) {
	// Create directory as nessary
	if ((index_counter % MAX_FILE_PER_DIR) == 0) {
		char dir_name[100];
		sprintf(dir_name, "dir_%04lu", index_counter / MAX_FILE_PER_DIR);
		lfs_mkdir(&lfs_sd, dir_name);
		lfs_mkdir(&lfs_flash, dir_name);
	}

	// Choose file name
	char log_filename[100];
	sprintf(log_filename,
			"dir_%04lu/log_%04lu.bin",
			index_counter / MAX_FILE_PER_DIR,
			index_counter % MAX_FILE_PER_DIR);

	++index_counter;

	// Update counter files (flash and sd)
	
	lfs_file_t counter_file;
	lfs_file_open(&lfs_sd, &counter_file, "/counter.bin", LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
	lfs_file_write(&lfs_sd, &counter_file, &index_counter, sizeof(index_counter));
	lfs_file_close(&lfs_sd, &counter_file);

	if (lfs_file_open(&lfs_sd, &logfile_sd, log_filename, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_EXCL) != 0) {
	}

	lfs_file_open(&lfs_flash, &counter_file, "/counter.bin", LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
	lfs_file_write(&lfs_flash, &counter_file, &index_counter, sizeof(index_counter));
	lfs_file_close(&lfs_flash, &counter_file);

	if (lfs_file_open(&lfs_flash, &logfile_flash, log_filename, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_EXCL) != 0) {
	}

	page_counter = 0;
}

w_status_t fs_init(void) {

	uint32_t sd_index = 0;
	uint32_t flash_index = 0;

	HAL_SD_InitCard(&hsd1);

	// LittleFS mount

	if (lfsshim_sd_mount_mbr(&lfs_sd, &hsd1) != 0) {
		return W_FAILURE;
	}

	if (lfs_mount(&lfs_flash, &config) < 0){
		return W_FAILURE;
	}

	// Read the file count counter
	lfs_file_t counter_file;
	if (lfs_file_open(&lfs_sd, &counter_file, "counter.bin", LFS_O_RDONLY) == 0) {
		lfs_file_read(&lfs_sd, &counter_file, &sd_index, sizeof(index_counter));
		lfs_file_close(&lfs_sd, &counter_file);
	}

	if (lfs_file_open(&lfs_flash, &counter_file, "counter.bin", LFS_O_RDONLY) == 0) {
		lfs_file_read(&lfs_flash, &counter_file, &flash_index, sizeof(index_counter));
		lfs_file_close(&lfs_flash, &counter_file);
	}

	// set index counter to the larger index
	index_counter = (sd_index > flash_index) ? sd_index : flash_index;


	fs_new_file();

	return W_SUCCESS;
}

void fs_write_page(const uint8_t *page) {
	lfs_ssize_t sd_written = lfs_file_write(&lfs_sd, &logfile_sd, page, PAGE_SIZE);
	lfs_ssize_t flash_written = lfs_file_write(&lfs_flash, &logfile_flash, page, PAGE_SIZE);

	++page_counter;

	if (page_counter >= MAX_FILE_SIZE_PAGES) {
		lfs_file_close(&lfs_sd, &logfile_sd);
		lfs_file_close(&lfs_flash, &logfile_flash);
		fs_new_file();
	} else {
		lfs_file_sync(&lfs_sd, &logfile_sd);
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
