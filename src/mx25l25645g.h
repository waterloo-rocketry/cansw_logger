#include "stm32h7xx_hal.h"
#include "common.h"
#include <stdbool.h>

typedef struct {
    QSPI_HandleTypeDef *hqspi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
    uint8_t *buffer;
    volatile bool tx_in_progress;
    volatile bool rx_in_progress;
    volatile bool txrx_failed;

} mx25l25645g_t;

typedef enum {
    MX25L25645G_STATUS_OK,
    MX25L25645G_STATUS_BUSY,
    MX25L25645G_STATUS_ERROR
} mx25l25645g_status_t;

/**
 * @brief Intitialization 
 */
w_status_t mx25l25645g_init(mx25l25645g_t *external_flash, QSPI_HandleTypeDef *hqspi, GPIO_TypeDef *cs_port, uint16_t cs_pin);


/**
 * @brief read from flash
 */
w_status_t mx25l25645g_read(mx25l25645g_t *flash, uint8_t *data, uint32_t address, uint32_t size);

/**
 * @brief write to flash
 */
w_status_t mx25l25645g_write(mx25l25645g_t *flash, uint8_t *data, uint32_t address, uint32_t size);


/**
 * @brief sector erase
 */
w_status_t mx25l25645g_erase_sector(mx25l25645g_t *flash, uint32_t address);

/**
 * @brief read from flash qpi mode
 */
w_status_t mx25l25645g_quad_read(mx25l25645g_t *flash, uint8_t *data, uint32_t address, uint32_t size);

/**
 * @brief Starts a DMA write, sets tx_in_progress = true while the transfer is ongoing
 */
w_status_t mx25l25645g_write_dma(mx25l25645g_t *flash, uint8_t *data, uint32_t address, uint32_t size);

/**
 * @brief Starts a DMA read, sets rx_in_progress = true while the transfer is ongoing
 */
w_status_t mx25l25645g_quad_read_dma(mx25l25645g_t *flash, uint8_t *data, uint32_t address, uint32_t size);

/**
 * @brief check programming
 */
mx25l25645g_status_t mx25l25645g_check_programming(mx25l25645g_t *flash);

/**
 * @brief aborts the current qspi transfer
 */
w_status_t mx25l25645g_abort(mx25l25645g_t *flash);

/**
 * @brief returns the status of rx_in_progress
 */
bool mx25l25645g_rx_in_progress(mx25l25645g_t *flash);

/**
 * @brief returns the status of tx_in_progress
 */
bool mx25l25645g_tx_in_progress(mx25l25645g_t *flash);

/**
 * @brief returns the status of txrx_failed
 */
bool mx25l25645g_txrx_failed(mx25l25645g_t *flash);
