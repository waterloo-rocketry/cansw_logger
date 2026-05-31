#include "common.h"
#include "lfs.h"
#include "mbr.h"

#include "mx25l25645g.h"

static uint32_t lfsshim_mx25l25645g_first_block_offset = 0;

static int lfsshim_mx25l25645g_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off,
									void *buffer, lfs_size_t size) {
	uint32_t block_addr = block + lfsshim_mx25l25645g_first_block_offset;

	w_assert((size % 256) == 0);
	w_assert((off % 256) == 0);

	uint32_t num_pages = size / 256;

	mx25l25645g_write(block * 4096 + off, num_pages, buffer);

	return 0; // success
}

static int lfsshim_mx25l25645g_write(const struct lfs_config *c, lfs_block_t block, lfs_off_t off,
									 const void *buffer, lfs_size_t size) {
	uint32_t block_addr = block + lfsshim_mx25l25645g_first_block_offset;

	w_assert((size % 256) == 0);
	w_assert((off % 256) == 0);

	uint32_t num_pages = size / 256;

	mx25l25645g_read(block * 4096 + off, num_pages, buffer);

	return 0; // success
}

static int lfsshim_mx25l25645g_erase(const struct lfs_config *c, lfs_block_t block) {
	mx25l25645g_erase(block * 4096);
}

static int lfsshim_mx25l25645g_sync(const struct lfs_config *c) {
	return 0;
}

// configuration of the filesystem is provided by this struct
const static struct lfs_config mx25l25645g_cfg = {
	// block device operations
	.read = lfsshim_mx25l25645g_read,
	.prog = lfsshim_mx25l25645g_write,
	.erase = lfsshim_mx25l25645g_erase,
	.sync = lfsshim_mx25l25645g_sync,

	// block device configuration
	.read_size = 256,
	.prog_size = 256,
	.block_size = 4096,
	.block_count = 8192,
	.block_cycles = -1,
	.cache_size = 512,
	.lookahead_size = 512,
	.compact_thresh = -1,
	.name_max = 0,
	.file_max = 0,
	.attr_max = 0,
	.metadata_max = 0,
	.inline_max = -1};

w_status_t lfsshim_mx25l25645g_mount(lfs_t *lfs, uint32_t first_block_offset) {
	memset(lfs, 0, sizeof(lfs_t));

	lfsshim_mx25l25645g_first_block_offset = first_block_offset;

	if (lfs_mount(lfs, &mx25l25645g_cfg) != 0) {
		return W_IO_ERROR;
	}

	return W_SUCCESS;
}

w_status_t lfsshim_mx25l25645g_mount_mbr(lfs_t *lfs) {
	uint8_t mbr_sector[512];

	mx25l25645g_read(0, 2, mbr_sector);

	uint32_t first_block_offset = 0;
	w_status_t status;
	if ((status = mbr_parse(mbr_sector, 0x83, &first_block_offset)) != W_SUCCESS) {
		return status;
	}

	if (lfsshim_mx25l25645g_mount(lfs, first_block_offset) != 0) {
		return W_IO_ERROR;
	}
	return W_SUCCESS;
}
