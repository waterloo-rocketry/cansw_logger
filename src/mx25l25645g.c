#include "stm32h7xx_hal.h"
#include "mx25l25645g.h"

#define MX25L25645G_CMD_WRITE_ENABLE 0x06U
#define MX25L25645G_CMD_READ_STATUS 0x05U
#define MX25L25645G_CMD_PAGE_PROGRAM 0x02U
#define MX25L25645G_CMD_SECTOR_ERASE 0x20U
#define MX25L25645G_CMD_EQIO 0x35U
#define MX25L25645G_CMD_RDID 0xAFU
#define MX25L25645G_CMD_READ 0x03U
#define MX25L25645G_CMD_4READ 0xEBU

#define MX25L25645G_WIP_MASK 0x01U

// TODO: return certain status
void mx25l25645g_init(mx25l25645g_t *flash, QSPI_HandleTypeDef *hqspi, GPIO_TypeDef *cs_port, uint16_t cs_pin){

	HAL_StatusTypeDef flash_status;

    flash->cs_pin = cs_pin;
    flash->cs_port = cs_port;
    flash->hqspi = hqspi;

	QSPI_CommandTypeDef command_eqio = {0};

	command_eqio.Instruction = MX25L25645G_CMD_EQIO;
	command_eqio.AddressMode = QSPI_ADDRESS_NONE;
	command_eqio.DataMode = QSPI_DATA_NONE;
	command_eqio.InstructionMode = QSPI_INSTRUCTION_1_LINE;

    QSPI_CommandTypeDef command = {0};

    command.Instruction = MX25L25645G_CMD_RDID;
    command.DataMode = QSPI_DATA_4_LINES;
    command.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    command.AddressMode = QSPI_ADDRESS_NONE;
    command.NbData = 3U;

    uint8_t received[3] = {0};

	/* Enable 4 lines */
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_RESET);
	HAL_QSPI_Command(flash->hqspi, &command_eqio, 100U);
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);

	/* Read */
    HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_RESET);
    HAL_QSPI_Command(flash->hqspi, &command, 100U);
    HAL_QSPI_Receive(flash->hqspi, received, 100U);
    HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);
}


void mx25l25645g_read(mx25l25645g_t *flash, uint8_t *data, uint32_t address, uint32_t size){

	QSPI_CommandTypeDef command_read = {0};

	command_read.Instruction = MX25L25645G_CMD_READ;
	command_read.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	command_read.Address = address;
	command_read.AddressSize = QSPI_ADDRESS_24_BITS;
	command_read.AddressMode = QSPI_ADDRESS_1_LINE;
	command_read.DataMode = QSPI_DATA_1_LINE;
	command_read.NbData = size; 

	/* Read */
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_RESET);
	HAL_QSPI_Command(flash->hqspi, &command_read, 100U);
	HAL_QSPI_Receive(flash->hqspi, data, 100U);
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);

}

void mx25l25645g_quad_read(mx25l25645g_t *flash, uint8_t *data, uint32_t address, uint32_t size){

	QSPI_CommandTypeDef command_read = {0};

	command_read.Instruction = MX25L25645G_CMD_4READ;
	command_read.InstructionMode = QSPI_INSTRUCTION_4_LINES;
	command_read.Address = address;
	command_read.AddressSize = QSPI_ADDRESS_24_BITS;
	command_read.AddressMode = QSPI_ADDRESS_4_LINES;
	command_read.DataMode = QSPI_DATA_4_LINES;
	command_read.DummyCycles = 4U;
	command_read.AlternateBytes = 1U;
	command_read.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
	command_read.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	command_read.NbData = size; 

	/* Read */
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_RESET);
	HAL_QSPI_Command(flash->hqspi, &command_read, 100U);
	HAL_QSPI_Receive(flash->hqspi, data, 100U);
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);

}

void mx25l25645g_write(mx25l25645g_t *flash, uint8_t *data, uint32_t address, uint32_t size){

	QSPI_CommandTypeDef command_wren = {0};
	uint8_t status_reg = 0U;

    command_wren.Instruction = MX25L25645G_CMD_WRITE_ENABLE;
    command_wren.DataMode = QSPI_DATA_NONE;
    command_wren.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    command_wren.AddressMode = QSPI_ADDRESS_NONE;
    command_wren.NbData = 0U;

	QSPI_CommandTypeDef command_rdsr = {0};

	command_rdsr.Instruction = MX25L25645G_CMD_READ_STATUS;
	command_rdsr.DataMode = QSPI_DATA_4_LINES;
	command_rdsr.InstructionMode = QSPI_INSTRUCTION_4_LINES;
	command_rdsr.AddressMode = QSPI_ADDRESS_NONE;
	command_rdsr.NbData = 1U;

	QSPI_CommandTypeDef command_write = {0};

	command_write.Instruction = MX25L25645G_CMD_PAGE_PROGRAM;
	command_write.DataMode = QSPI_DATA_4_LINES;
	command_write.InstructionMode = QSPI_INSTRUCTION_4_LINES;
	command_write.AddressMode = QSPI_ADDRESS_4_LINES;
	command_write.NbData = size;
	command_write.AddressSize = QSPI_ADDRESS_24_BITS;
	command_write.Address = address; 

	/* Write enable */
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_RESET);
	HAL_QSPI_Command(flash->hqspi, &command_wren, 100U);
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);

	/* Page program */
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_RESET);
	HAL_QSPI_Command(flash->hqspi, &command_write, 100U);
	HAL_QSPI_Transmit(flash->hqspi, data, 100U);
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);

	/* Wait until WIP = 0 */
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_RESET);
	do {
		HAL_QSPI_Command(flash->hqspi, &command_rdsr, 100U);
		HAL_QSPI_Receive(flash->hqspi, &status_reg, 100U);
	} while ((status_reg & MX25L25645G_WIP_MASK) != 0U);

	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);

}

void mx25l25645g_erase_sector(mx25l25645g_t *flash, uint32_t address){

	QSPI_CommandTypeDef command_wren = {0};
	uint8_t status_reg = 0U;

    command_wren.Instruction = MX25L25645G_CMD_WRITE_ENABLE;
    command_wren.DataMode = QSPI_DATA_NONE;
    command_wren.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    command_wren.AddressMode = QSPI_ADDRESS_NONE;
    command_wren.NbData = 0U;

	QSPI_CommandTypeDef command_rdsr = {0};

	command_rdsr.Instruction = MX25L25645G_CMD_READ_STATUS;
	command_rdsr.DataMode = QSPI_DATA_4_LINES;
	command_rdsr.InstructionMode = QSPI_INSTRUCTION_4_LINES;
	command_rdsr.AddressMode = QSPI_ADDRESS_NONE;
	command_rdsr.NbData = 1U;

	QSPI_CommandTypeDef command_sector_erase = {0};

	command_sector_erase.Instruction = MX25L25645G_CMD_SECTOR_ERASE;
	command_sector_erase.DataMode = QSPI_DATA_NONE;
	command_sector_erase.InstructionMode = QSPI_INSTRUCTION_4_LINES;
	command_sector_erase.NbData = 0U;
	command_sector_erase.Address = address;
	command_sector_erase.AddressSize = QSPI_ADDRESS_24_BITS;
	command_sector_erase.AddressMode = QSPI_ADDRESS_4_LINES;

	/* Write enable */
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_RESET);
	HAL_QSPI_Command(flash->hqspi, &command_wren, 100U);
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);

	/* Sector erase */
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_RESET);
	HAL_QSPI_Command(flash->hqspi, &command_sector_erase, 100U);

	/* Wait until WIP = 0 */
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_RESET);
	do {
		HAL_QSPI_Command(flash->hqspi, &command_rdsr, 100U);
		HAL_QSPI_Receive(flash->hqspi, &status_reg, 100U);
	} while ((status_reg & MX25L25645G_WIP_MASK) != 0U);

	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);

}
