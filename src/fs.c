#include <common.h>
#include <stdint.h>
#include <stdio.h>

#include "fatfs.h"
#include "main.h"

#include "fs.h"
#include "log.h"
#include "platform.h"

extern SD_HandleTypeDef hsd1;

#define MAX_FILE_PER_DIR 1000

static FATFS fatfs;
static FIL logfile;

static uint32_t index_counter = 0;
static uint32_t page_counter = 0;
static FRESULT fs_result = FR_OK;

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

	fs_result = f_open(&logfile, log_filename, FA_WRITE | FA_OPEN_ALWAYS);

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
	fs_result = f_write(&logfile, page, PAGE_SIZE, &retval);
	++page_counter;

	if (page_counter >= MAX_FILE_SIZE_PAGES) {
		f_close(&logfile);
		fs_new_file();
	} else {
		f_sync(&logfile);
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
	if(fs_result != FR_OK) {
		return 1 << E_FS_ERROR_OFFSET;
	}
	return 0;
}
