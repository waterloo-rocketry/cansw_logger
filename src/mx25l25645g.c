#include <stdint.h>

#include "stm32h7xx_hal.h"

#include "common.h"
#include "mx25l25645g.h"

extern QSPI_HandleTypeDef hqspi;

// Device must be already in QuadSPI(QPI) mode
static uint8_t mx25l25645g_read_status_register(void) {
	QSPI_CommandTypeDef commandtypedef = {0};

	// Instruction Fields
	commandtypedef.Instruction = 0x05; // RDSR
	commandtypedef.InstructionMode = QSPI_INSTRUCTION_4_LINES;

	// Address Fields
	commandtypedef.Address = 0;
	commandtypedef.AddressSize = 0;
	commandtypedef.AddressMode = QSPI_ADDRESS_NONE;

	// Alternate Bytes Fields
	commandtypedef.AlternateBytes = 0;
	commandtypedef.AlternateBytesSize = 0;
	commandtypedef.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;

	// Data Fields
	commandtypedef.DataMode = QSPI_DATA_4_LINES;
	commandtypedef.NbData = 1;

	// Other Fields
	commandtypedef.DummyCycles = 0;
	commandtypedef.DdrMode = QSPI_DDR_MODE_DISABLE;
	commandtypedef.DdrHoldHalfCycle = 0;
	commandtypedef.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	uint8_t sr;

	HAL_QSPI_Command(&hqspi, &commandtypedef, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
	HAL_QSPI_Receive(&hqspi, &sr, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

	return sr;
}

// Device must be already in QuadSPI(QPI) mode
static uint8_t mx25l25645g_read_config_register(void) {
	QSPI_CommandTypeDef commandtypedef = {0};

	// Instruction Fields
	commandtypedef.Instruction = 0x15; // RDCR
	commandtypedef.InstructionMode = QSPI_INSTRUCTION_4_LINES;

	// Address Fields
	commandtypedef.Address = 0;
	commandtypedef.AddressSize = 0;
	commandtypedef.AddressMode = QSPI_ADDRESS_NONE;

	// Alternate Bytes Fields
	commandtypedef.AlternateBytes = 0;
	commandtypedef.AlternateBytesSize = 0;
	commandtypedef.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;

	// Data Fields
	commandtypedef.DataMode = QSPI_DATA_4_LINES;
	commandtypedef.NbData = 1;

	// Other Fields
	commandtypedef.DummyCycles = 0;
	commandtypedef.DdrMode = QSPI_DDR_MODE_DISABLE;
	commandtypedef.DdrHoldHalfCycle = 0;
	commandtypedef.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	uint8_t cr;

	HAL_QSPI_Command(&hqspi, &commandtypedef, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
	HAL_QSPI_Receive(&hqspi, &cr, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

	return cr;
}

// Device must be already in QuadSPI(QPI) mode
static void mx25l25645g_set_write_enable(void) {
	QSPI_CommandTypeDef commandtypedef = {0};

	// Instruction Fields
	commandtypedef.Instruction = 0x06; // WREN
	commandtypedef.InstructionMode = QSPI_INSTRUCTION_4_LINES;

	// Address Fields
	commandtypedef.Address = 0;
	commandtypedef.AddressSize = 0;
	commandtypedef.AddressMode = QSPI_ADDRESS_NONE;

	// Alternate Bytes Fields
	commandtypedef.AlternateBytes = 0;
	commandtypedef.AlternateBytesSize = 0;
	commandtypedef.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;

	// Data Fields
	commandtypedef.DataMode = QSPI_DATA_NONE;
	commandtypedef.NbData = 0;

	// Other Fields
	commandtypedef.DummyCycles = 0;
	commandtypedef.DdrMode = QSPI_DDR_MODE_DISABLE;
	commandtypedef.DdrHoldHalfCycle = 0;
	commandtypedef.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	HAL_QSPI_Command(&hqspi, &commandtypedef, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
}

void mx25l25645g_init(void) {
	QSPI_CommandTypeDef commandtypedef = {0};

	// Instruction Fields
	commandtypedef.Instruction = 0;
	commandtypedef.InstructionMode = QSPI_INSTRUCTION_4_LINES;

	// Address Fields
	commandtypedef.Address = 0;
	commandtypedef.AddressSize = 0;
	commandtypedef.AddressMode = QSPI_ADDRESS_NONE;

	// Alternate Bytes Fields
	commandtypedef.AlternateBytes = 0;
	commandtypedef.AlternateBytesSize = 0;
	commandtypedef.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;

	// Data Fields
	commandtypedef.DataMode = QSPI_DATA_NONE;
	commandtypedef.NbData = 0;

	// Other Fields
	commandtypedef.DummyCycles = 0;
	commandtypedef.DdrMode = QSPI_DDR_MODE_DISABLE;
	commandtypedef.DdrHoldHalfCycle = 0;
	commandtypedef.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	// Exit QuadSPI(QPI) Mode, in case of a warm reset
	commandtypedef.Instruction = 0xf5; // RSTQIO
	HAL_QSPI_Command(&hqspi, &commandtypedef, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

	// Reset flash chip(two commands)
	commandtypedef.Instruction = 0x66; // RSTEN
	commandtypedef.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	HAL_QSPI_Command(&hqspi, &commandtypedef, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

	commandtypedef.Instruction = 0x99; // RST
	HAL_QSPI_Command(&hqspi, &commandtypedef, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

	HAL_Delay(40);

	// Read JEDEC ID in SPI Mode
	uint8_t spi_id[3];
	commandtypedef.Instruction = 0x9f; // RDID
	commandtypedef.DataMode = QSPI_DATA_1_LINE;
	commandtypedef.NbData = 3;
	HAL_QSPI_Command(&hqspi, &commandtypedef, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
	HAL_QSPI_Receive(&hqspi, spi_id, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

	// Switch to 32-bits address mode
	commandtypedef.Instruction = 0xb7; // EN4B
	commandtypedef.DataMode = QSPI_DATA_NONE;
	commandtypedef.NbData = 0;
	HAL_QSPI_Command(&hqspi, &commandtypedef, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

	// Switch to QuadSPI(QPI) Mode
	commandtypedef.Instruction = 0x35; // EQIO
	HAL_QSPI_Command(&hqspi, &commandtypedef, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

	// Set status and configuration register
	mx25l25645g_set_write_enable();
	uint8_t sr_cr[2];
	sr_cr[0] = (1 << 6); // Status Register, QE
	sr_cr[1] = 0; // Configuration Register
	commandtypedef.Instruction = 0x01; // WRSR
	commandtypedef.InstructionMode = QSPI_INSTRUCTION_4_LINES;
	commandtypedef.DataMode = QSPI_DATA_4_LINES;
	commandtypedef.NbData = 2;
	HAL_QSPI_Command(&hqspi, &commandtypedef, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
	HAL_QSPI_Transmit(&hqspi, sr_cr, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

	// Read JEDEC ID in QuadSPI(QPI) Mode
	uint8_t qpi_id[3];
	commandtypedef.Instruction = 0xaf; // QPIID
	commandtypedef.NbData = 3;
	HAL_QSPI_Command(&hqspi, &commandtypedef, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
	HAL_QSPI_Receive(&hqspi, qpi_id, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

	uint8_t sr = mx25l25645g_read_status_register();
	uint8_t cr = mx25l25645g_read_config_register();

	return;
}

/**
 * Flash Sector Erase, 4K aligned
 * @param address Address to be erased, must be a multiple of 0x1000 for alignment
 */
void mx25l25645g_erase(uint32_t address) {
	w_assert((address & 0xfff) == 0); // Alignment check

	mx25l25645g_set_write_enable();

	while (!(mx25l25645g_read_status_register() & 0x2)) {
		mx25l25645g_set_write_enable();
	}

	QSPI_CommandTypeDef commandtypedef = {0};

	// Instruction Fields
	commandtypedef.Instruction = 0x21; // SE4B
	commandtypedef.InstructionMode = QSPI_INSTRUCTION_4_LINES;

	// Address Fields
	commandtypedef.Address = address;
	commandtypedef.AddressSize = QSPI_ADDRESS_32_BITS;
	commandtypedef.AddressMode = QSPI_ADDRESS_4_LINES;

	// Alternate Bytes Fields
	commandtypedef.AlternateBytes = 0;
	commandtypedef.AlternateBytesSize = 0;
	commandtypedef.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;

	// Data Fields
	commandtypedef.DataMode = QSPI_DATA_NONE;
	commandtypedef.NbData = 0;

	// Other Fields
	commandtypedef.DummyCycles = 0;
	commandtypedef.DdrMode = QSPI_DDR_MODE_DISABLE;
	commandtypedef.DdrHoldHalfCycle = 0;
	commandtypedef.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	HAL_QSPI_Command(&hqspi, &commandtypedef, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

	while (mx25l25645g_read_status_register() & 0x1) {}
}

/**
 * Full chip erase
 */
void mx25l25645g_erase_chip(void) {
	mx25l25645g_set_write_enable();

	while (!(mx25l25645g_read_status_register() & 0x2)) {
		mx25l25645g_set_write_enable();
	}

	QSPI_CommandTypeDef commandtypedef = {0};

	// Instruction Fields
	commandtypedef.Instruction = 0x60; // CE
	commandtypedef.InstructionMode = QSPI_INSTRUCTION_4_LINES;

	// Address Fields
	commandtypedef.Address = 0;
	commandtypedef.AddressSize = 0;
	commandtypedef.AddressMode = QSPI_ADDRESS_NONE;

	// Alternate Bytes Fields
	commandtypedef.AlternateBytes = 0;
	commandtypedef.AlternateBytesSize = 0;
	commandtypedef.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;

	// Data Fields
	commandtypedef.DataMode = QSPI_DATA_NONE;
	commandtypedef.NbData = 0;

	// Other Fields
	commandtypedef.DummyCycles = 0;
	commandtypedef.DdrMode = QSPI_DDR_MODE_DISABLE;
	commandtypedef.DdrHoldHalfCycle = 0;
	commandtypedef.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	HAL_QSPI_Command(&hqspi, &commandtypedef, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

	while (mx25l25645g_read_status_register() & 0x1) {}
}

/**
 * Flash Page Write, 256B aligned
 * @param address Address to be written, must be a multiple of 0x100 for alignment
 * @param npages Number of 256B pages to be written
 * @param buffer Data buffer
 */
void mx25l25645g_write(uint32_t address, uint32_t npages, const uint8_t *buffer) {
	w_assert((address & 0xff) == 0); // Alignment check

	QSPI_CommandTypeDef commandtypedef = {0};

	// Instruction Fields
	commandtypedef.Instruction = 0x3E; // 4PP4P
	commandtypedef.InstructionMode = QSPI_INSTRUCTION_4_LINES;

	// Address Fields
	commandtypedef.Address = address;
	commandtypedef.AddressSize = QSPI_ADDRESS_32_BITS;
	commandtypedef.AddressMode = QSPI_ADDRESS_4_LINES;

	// Alternate Bytes Fields
	commandtypedef.AlternateBytes = 0;
	commandtypedef.AlternateBytesSize = 0;
	commandtypedef.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;

	// Data Fields
	commandtypedef.DataMode = QSPI_DATA_4_LINES;
	commandtypedef.NbData = 256;

	// Other Fields
	commandtypedef.DummyCycles = 0;
	commandtypedef.DdrMode = QSPI_DDR_MODE_DISABLE;
	commandtypedef.DdrHoldHalfCycle = 0;
	commandtypedef.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	for (uint32_t page = 0; page < npages; page++) {
		mx25l25645g_set_write_enable();

		while (!(mx25l25645g_read_status_register() & 0x2)) {
			mx25l25645g_set_write_enable();
		}

		commandtypedef.Address = address + page * 256;
		HAL_QSPI_Command(&hqspi, &commandtypedef, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
		HAL_QSPI_Transmit(&hqspi, buffer + page * 256, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

		while (mx25l25645g_read_status_register() & 0x1) {}
	}
}

/**
 * Flash Page Read, 256B aligned
 * @param address Address to be read, must be a multiple of 0x100 for alignment
 * @param npages Number of 256B pages to be read
 * @param buffer Data buffer
 */
void mx25l25645g_read(uint32_t address, uint32_t npages, uint8_t *buffer) {
	w_assert((address & 0xff) == 0); // Alignment check

	QSPI_CommandTypeDef commandtypedef = {0};

	// Instruction Fields
	commandtypedef.Instruction = 0x6C; // QREAD4B
	commandtypedef.InstructionMode = QSPI_INSTRUCTION_4_LINES;

	// Address Fields
	commandtypedef.Address = address;
	commandtypedef.AddressSize = QSPI_ADDRESS_32_BITS;
	commandtypedef.AddressMode = QSPI_ADDRESS_4_LINES;

	// Alternate Bytes Fields
	commandtypedef.AlternateBytes = 0;
	commandtypedef.AlternateBytesSize = 0;
	commandtypedef.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;

	// Data Fields
	commandtypedef.DataMode = QSPI_DATA_4_LINES;
	commandtypedef.NbData = 256 * npages;

	// Other Fields
	commandtypedef.DummyCycles = 6;
	commandtypedef.DdrMode = QSPI_DDR_MODE_DISABLE;
	commandtypedef.DdrHoldHalfCycle = 0;
	commandtypedef.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	HAL_StatusTypeDef st =
		HAL_QSPI_Command(&hqspi, &commandtypedef, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
	st = HAL_QSPI_Receive(&hqspi, buffer, 1000000);

	return;
}
