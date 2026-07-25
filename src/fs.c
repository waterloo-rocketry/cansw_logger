#include <common.h>
#include <stdint.h>
#include <stdio.h>

#include "lfs.h"
#include "main.h"

#include "fs.h"
#include "log.h"
#include "mbr.h"
#include "stm32h7/littlefs_sd_shim.h"

extern SD_HandleTypeDef hsd1;

#define MAX_FILE_PER_DIR 1000

lfs_t lfs;
lfs_file_t logfile;

uint32_t index_counter = 0;
uint32_t page_counter = 0;

static void fs_new_file(void) {
	// Create directory as nessary
	if ((index_counter % MAX_FILE_PER_DIR) == 0) {
		char dir_name[100];
		sprintf(dir_name, "dir_%04lu", index_counter / MAX_FILE_PER_DIR);
		lfs_mkdir(&lfs, dir_name);
	}

	// Choose file name
	char log_filename[100];
	sprintf(log_filename,
			"dir_%04lu/log_%04lu.bin",
			index_counter / MAX_FILE_PER_DIR,
			index_counter % MAX_FILE_PER_DIR);

	++index_counter;

	// Update counter file
	lfs_file_t counter_file;
	lfs_file_open(&lfs, &counter_file, "/counter.bin", LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
	lfs_file_write(&lfs, &counter_file, &index_counter, sizeof(index_counter));
	lfs_file_close(&lfs, &counter_file);

	if (lfs_file_open(&lfs, &logfile, log_filename, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_EXCL) != 0) {
	}

	page_counter = 0;
}

w_status_t fs_init(void) {
	HAL_SD_InitCard(&hsd1);

	__disable_irq();

	// LittleFS mount
	if (lfsshim_sd_mount_mbr(&lfs, &hsd1) != 0) {
		__enable_irq();
		return W_FAILURE;
	}

	// Read the file count counter
	lfs_file_t counter_file;
	if (lfs_file_open(&lfs, &counter_file, "counter.bin", LFS_O_RDONLY) == 0) {
		lfs_file_read(&lfs, &counter_file, &index_counter, sizeof(index_counter));
		lfs_file_close(&lfs, &counter_file);
	}

	fs_new_file();

	__enable_irq();
	return W_SUCCESS;
}

void fs_write_page(const uint8_t *page) {
	__disable_irq();

	if (lfs_file_write(&lfs, &logfile, page, PAGE_SIZE) != 0) {}
	++page_counter;

	if (page_counter >= MAX_FILE_SIZE_PAGES) {
		lfs_file_close(&lfs, &logfile);
		fs_new_file();
	} else {
		lfs_file_sync(&lfs, &logfile);
	}

	__enable_irq();
}

uint32_t fs_get_log_written_size(void) {
	return page_counter * PAGE_SIZE;
}

uint32_t fs_get_sd_log_file_name(void) {
	// Because index_counter is file name of next file to be created, so decrement by 1 to get
	// current file name
	return index_counter - 1;
}
