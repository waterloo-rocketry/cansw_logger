#include <stdint.h>

#include "stm32h7xx_hal.h"

#include "mx25l25645g.h"

extern QSPI_HandleTypeDef hqspi;

void mx25l25645g_init(void) {
	QSPI_CommandTypeDef commandtypedef;

	// Instruction Fields
	commandtypedef.Instruction = 0; // RDID
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

	// Read JEDEC ID in QuadSPI(QPI) Mode
	uint8_t qpi_id[3];
	commandtypedef.Instruction = 0xaf; // QPIID
	commandtypedef.InstructionMode = QSPI_INSTRUCTION_4_LINES;
	commandtypedef.DataMode = QSPI_DATA_4_LINES;
	commandtypedef.NbData = 3;
	HAL_QSPI_Command(&hqspi, &commandtypedef, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
	HAL_QSPI_Receive(&hqspi, qpi_id, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

	return;
}
