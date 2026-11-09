#include "stm32h7xx_hal.h"
#include "mx25l25645g.h"
#include "platform.h"

#define MX25L25645G_CMD_WRITE_ENABLE 0x06U
#define MX25L25645G_CMD_READ_STATUS 0x05U
#define MX25L25645G_CMD_PAGE_PROGRAM 0x02U
#define MX25L25645G_CMD_SECTOR_ERASE 0x20U
#define MX25L25645G_CMD_EQIO 0x35U
#define MX25L25645G_CMD_RDID 0xAFU
#define MX25L25645G_CMD_READ 0x03U
#define MX25L25645G_CMD_4READ 0xEBU
#define MX25L25645G_CMD_EN4B 0xB7U

#define MX25L25645G_WIP_MASK 0x01U

#define MX25L25645G_QSPI_TIMEOUT_MS 100U
#define MX25L25645G_SECTOR_ERASE_TIMEOUT_MS 600U /* max sector erase cycle time is 400 ms, from datasheet (14) */
#define MX25L25645G_PAGE_PROGRAM_TIMEOUT_MS 10U /* max page program time is 0.75 ms */

static mx25l25645g_t *active_flash = NULL;

w_status_t mx25l25645g_init(mx25l25645g_t *flash, QSPI_HandleTypeDef *hqspi, GPIO_TypeDef *cs_port, uint16_t cs_pin){

    flash->cs_pin = cs_pin;
    flash->cs_port = cs_port;
    flash->hqspi = hqspi;

	QSPI_CommandTypeDef command_eqio = {0};

	command_eqio.Instruction = MX25L25645G_CMD_EQIO;
	command_eqio.AddressMode = QSPI_ADDRESS_NONE;
	command_eqio.DataMode = QSPI_DATA_NONE;
	command_eqio.InstructionMode = QSPI_INSTRUCTION_1_LINE;

	QSPI_CommandTypeDef command_en4b = {0};
	command_en4b.AddressMode = QSPI_ADDRESS_NONE;
	command_en4b.Instruction = MX25L25645G_CMD_EN4B;
	command_en4b.DataMode = QSPI_DATA_NONE;
	command_en4b.InstructionMode = QSPI_INSTRUCTION_4_LINES;

	/* Enable 4 lines */
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_RESET);
	if (HAL_QSPI_Command(flash->hqspi, &command_eqio, MX25L25645G_QSPI_TIMEOUT_MS) != HAL_OK){

		HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);
		return W_FAILURE;
	}
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);

	/* Enable 4 byte address mode */
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_RESET);
	if (HAL_QSPI_Command(hqspi, &command_en4b, MX25L25645G_QSPI_TIMEOUT_MS) != HAL_OK) {
    	return W_FAILURE;
	}
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);

	return W_SUCCESS;
}


w_status_t mx25l25645g_read(mx25l25645g_t *flash, uint8_t *data, uint32_t address, uint32_t size){

	QSPI_CommandTypeDef command_read = {0};

	command_read.Instruction = MX25L25645G_CMD_READ;
	command_read.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	command_read.Address = address;
	command_read.AddressSize = QSPI_ADDRESS_32_BITS;
	command_read.AddressMode = QSPI_ADDRESS_1_LINE;
	command_read.DataMode = QSPI_DATA_1_LINE;
	command_read.NbData = size; 

	/* Read */
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_RESET);
	if (HAL_QSPI_Command(flash->hqspi, &command_read, MX25L25645G_QSPI_TIMEOUT_MS) != HAL_OK){
		HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);
		return W_FAILURE;
	}
	if (HAL_QSPI_Receive(flash->hqspi, data, MX25L25645G_QSPI_TIMEOUT_MS) != HAL_OK){
		HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);
		return W_FAILURE;
	}
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);

	return W_SUCCESS;
}

w_status_t mx25l25645g_quad_read(mx25l25645g_t *flash, uint8_t *data, uint32_t address, uint32_t size){

	QSPI_CommandTypeDef command_read = {0};

	command_read.Instruction = MX25L25645G_CMD_4READ;
	command_read.InstructionMode = QSPI_INSTRUCTION_4_LINES;
	command_read.Address = address;
	command_read.AddressSize = QSPI_ADDRESS_32_BITS;
	command_read.AddressMode = QSPI_ADDRESS_4_LINES;
	command_read.DataMode = QSPI_DATA_4_LINES;
	command_read.DummyCycles = 4U;
	command_read.AlternateBytes = 1U;
	command_read.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
	command_read.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	command_read.NbData = size; 

	/* Read */
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_RESET);
	if (HAL_QSPI_Command(flash->hqspi, &command_read, MX25L25645G_QSPI_TIMEOUT_MS) != HAL_OK){
		HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);
		return W_FAILURE;
	}
	if (HAL_QSPI_Receive(flash->hqspi, data, MX25L25645G_QSPI_TIMEOUT_MS) != HAL_OK){
		HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);
		return W_FAILURE;
	}
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);

	return W_SUCCESS;
}

w_status_t mx25l25645g_write(mx25l25645g_t *flash, uint8_t *data, uint32_t address, uint32_t size){

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
	command_write.AddressSize = QSPI_ADDRESS_32_BITS;
	command_write.Address = address; 

	/* Write enable */
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_RESET);
	if (HAL_QSPI_Command(flash->hqspi, &command_wren, MX25L25645G_QSPI_TIMEOUT_MS) != HAL_OK){
		HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);
		return W_FAILURE;
	}
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);

	/* Page program */
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_RESET);
	if (HAL_QSPI_Command(flash->hqspi, &command_write, MX25L25645G_QSPI_TIMEOUT_MS) != HAL_OK){
		HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);
		return W_FAILURE;
	}
	if (HAL_QSPI_Transmit(flash->hqspi, data, MX25L25645G_QSPI_TIMEOUT_MS) != HAL_OK){
		HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);
		return W_FAILURE;
	}
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);

	uint32_t start_ms = millis();

	/* Wait until WIP = 0 */
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_RESET);
	do {
		if (HAL_QSPI_Command(flash->hqspi, &command_rdsr, MX25L25645G_QSPI_TIMEOUT_MS) != HAL_OK){
			HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);
			return W_FAILURE;
		}
		if (HAL_QSPI_Receive(flash->hqspi, &status_reg, MX25L25645G_QSPI_TIMEOUT_MS) != HAL_OK){
			HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);
			return W_FAILURE;
		}
	} while ((status_reg & MX25L25645G_WIP_MASK) != 0U && millis() - start_ms <= MX25L25645G_PAGE_PROGRAM_TIMEOUT_MS);

	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);

	/* the polling loop timed out */
	if ((status_reg & MX25L25645G_WIP_MASK) != 0U){
		return W_FAILURE;
	}

	return W_SUCCESS;
}

w_status_t mx25l25645g_erase_sector(mx25l25645g_t *flash, uint32_t address){

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
	command_sector_erase.AddressSize = QSPI_ADDRESS_32_BITS;
	command_sector_erase.AddressMode = QSPI_ADDRESS_4_LINES;

	/* Write enable */
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_RESET);
	if (HAL_QSPI_Command(flash->hqspi, &command_wren, MX25L25645G_QSPI_TIMEOUT_MS) != HAL_OK){
		HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);
		return W_FAILURE;
	}
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);

	/* Sector erase */
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_RESET);
	if (HAL_QSPI_Command(flash->hqspi, &command_sector_erase, MX25L25645G_QSPI_TIMEOUT_MS) != HAL_OK){
		HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);
		return W_FAILURE;
	}

	uint32_t start_ms = millis();
		
	/* Wait until WIP = 0 */
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_RESET);
	do {
		if (HAL_QSPI_Command(flash->hqspi, &command_rdsr, MX25L25645G_QSPI_TIMEOUT_MS) != HAL_OK){
			HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);
			return W_FAILURE;
		}
		if (HAL_QSPI_Receive(flash->hqspi, &status_reg, MX25L25645G_QSPI_TIMEOUT_MS) != HAL_OK){
			HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);
			return W_FAILURE;
		}
	} while ((status_reg & MX25L25645G_WIP_MASK) != 0U && millis() - start_ms <= MX25L25645G_SECTOR_ERASE_TIMEOUT_MS);

	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);

	/* the polling loop timed out */
	if ((status_reg & MX25L25645G_WIP_MASK) != 0U){
		return W_FAILURE;
	}

	return W_SUCCESS;
}

w_status_t mx25l25645g_write_dma(mx25l25645g_t *flash, uint8_t *data, uint32_t address, uint32_t size){

	QSPI_CommandTypeDef command_wren = {0};

    command_wren.Instruction = MX25L25645G_CMD_WRITE_ENABLE;
    command_wren.DataMode = QSPI_DATA_NONE;
    command_wren.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    command_wren.AddressMode = QSPI_ADDRESS_NONE;
    command_wren.NbData = 0U;

	QSPI_CommandTypeDef command_write = {0};

	command_write.Instruction = MX25L25645G_CMD_PAGE_PROGRAM;
	command_write.DataMode = QSPI_DATA_4_LINES;
	command_write.InstructionMode = QSPI_INSTRUCTION_4_LINES;
	command_write.AddressMode = QSPI_ADDRESS_4_LINES;
	command_write.NbData = size;
	command_write.AddressSize = QSPI_ADDRESS_32_BITS;
	command_write.Address = address; 

	active_flash = flash;
	active_flash->buffer = data;
	active_flash->tx_in_progress = true;
	active_flash->txrx_failed = false;

	/* Write enable */
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_RESET);
	if (HAL_QSPI_Command(flash->hqspi, &command_wren, MX25L25645G_QSPI_TIMEOUT_MS) != HAL_OK){
		HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);
		flash->buffer = NULL;
		flash->tx_in_progress = false;
		active_flash = NULL;
		return W_FAILURE;
	}
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);

	/* Page program */
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_RESET);
	if (HAL_QSPI_Command(flash->hqspi, &command_write, MX25L25645G_QSPI_TIMEOUT_MS) != HAL_OK){
		HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);
		flash->buffer = NULL;
		flash->tx_in_progress = false;
		active_flash = NULL;
		return W_FAILURE;
	}
	if (HAL_QSPI_Transmit_DMA(flash->hqspi, data) != HAL_OK){
		HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);
		flash->buffer = NULL;
		flash->tx_in_progress = false;
		active_flash = NULL;
		return W_FAILURE;
	}

	return W_SUCCESS;
}

w_status_t mx25l25645g_quad_read_dma(mx25l25645g_t *flash, uint8_t *data, uint32_t address, uint32_t size){

	QSPI_CommandTypeDef command_read = {0};

	command_read.Instruction = MX25L25645G_CMD_4READ;
	command_read.InstructionMode = QSPI_INSTRUCTION_4_LINES;
	command_read.Address = address;
	command_read.AddressSize = QSPI_ADDRESS_32_BITS;
	command_read.AddressMode = QSPI_ADDRESS_4_LINES;
	command_read.DataMode = QSPI_DATA_4_LINES;
	command_read.DummyCycles = 4U;
	command_read.AlternateBytes = 1U;
	command_read.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
	command_read.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	command_read.NbData = size; 

	active_flash = flash;
	active_flash->buffer = data;
	active_flash->rx_in_progress = true;
	active_flash->txrx_failed = false;
	
	/* Read */
	HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_RESET);
	if (HAL_QSPI_Command(flash->hqspi, &command_read, MX25L25645G_QSPI_TIMEOUT_MS) != HAL_OK){
		HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);
		flash->buffer = NULL;
		flash->rx_in_progress = false;
		active_flash = NULL;
		return W_FAILURE;
	}
	if (HAL_QSPI_Receive_DMA(flash->hqspi, data) != HAL_OK){
		HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);
		flash->buffer = NULL;
		flash->rx_in_progress = false;
		active_flash = NULL;
		return W_FAILURE;
	}

	return W_SUCCESS;
}

mx25l25645g_status_t mx25l25645g_check_programming(mx25l25645g_t *flash){

	if (flash->tx_in_progress){
		return MX25L25645G_STATUS_BUSY;
	}

	QSPI_CommandTypeDef command_rdsr = {0};
	uint8_t status_reg = 0U;

	command_rdsr.Instruction = MX25L25645G_CMD_READ_STATUS;
	command_rdsr.DataMode = QSPI_DATA_4_LINES;
	command_rdsr.InstructionMode = QSPI_INSTRUCTION_4_LINES;
	command_rdsr.AddressMode = QSPI_ADDRESS_NONE;
	command_rdsr.NbData = 1U;

	if (HAL_QSPI_Command(flash->hqspi, &command_rdsr, MX25L25645G_QSPI_TIMEOUT_MS) != HAL_OK){
		return MX25L25645G_STATUS_ERROR;
	}

	if (HAL_QSPI_Receive(flash->hqspi, &status_reg, MX25L25645G_QSPI_TIMEOUT_MS) != HAL_OK){
		return MX25L25645G_STATUS_ERROR;
	}

	if ((status_reg & MX25L25645G_WIP_MASK) != 0U){
		return MX25L25645G_STATUS_BUSY;
	}

	return MX25L25645G_STATUS_OK;
}

w_status_t mx25l25645g_abort(mx25l25645g_t *flash){

	if (HAL_QSPI_Abort(flash->hqspi) != HAL_OK){
		return W_FAILURE;
	}

    HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);

	flash->buffer = NULL;
	flash->tx_in_progress = false;
	flash->rx_in_progress = false;
	flash->txrx_failed = true;
	active_flash = NULL;

	return W_SUCCESS;
}

void HAL_QSPI_RxCpltCallback(QSPI_HandleTypeDef *hqspi){

	if (active_flash == NULL){
		return;
	}

	HAL_GPIO_WritePin(active_flash->cs_port, active_flash->cs_pin, GPIO_PIN_SET);

	active_flash->buffer = NULL;
	active_flash->rx_in_progress = false;
	active_flash = NULL;

}

void HAL_QSPI_TxCpltCallback(QSPI_HandleTypeDef *hqspi){
	
	if (active_flash == NULL){
		return;
	}

	HAL_GPIO_WritePin(active_flash->cs_port, active_flash->cs_pin, GPIO_PIN_SET);

	active_flash->buffer = NULL;
	active_flash->tx_in_progress = false;
	active_flash = NULL;

}

void HAL_QSPI_ErrorCallback(QSPI_HandleTypeDef *hqspi){

	HAL_GPIO_WritePin(active_flash->cs_port, active_flash->cs_pin, GPIO_PIN_SET);

	active_flash->buffer = NULL;
	active_flash->tx_in_progress = false;
	active_flash->rx_in_progress = false;
	active_flash->txrx_failed = true;
	active_flash = NULL;

}