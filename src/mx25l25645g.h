#include "stm32h7xx_hal.h"
#include <stdbool.h>

typedef struct {
    QSPI_HandleTypeDef *hqspi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
    uint8_t *buffer;
    volatile bool tx_in_progress;
    volatile bool rx_in_progress;

} mx25l25645g_t;

typedef enum {
    MX25L25645G_STATUS_OK,
    MX25L25645G_STATUS_BUSY,
    MX25L25645G_STATUS_ERROR
} mx25l25645g_status_t;

/**
 * @brief Intitialization 
 */
void mx25l25645g_init(mx25l25645g_t *external_flash, QSPI_HandleTypeDef *hqspi, GPIO_TypeDef *cs_port, uint16_t cs_pin);


/**
 * @brief read from flash
 */
void mx25l25645g_read(mx25l25645g_t *flash, uint8_t *data, uint32_t address, uint32_t size);

/**
 * @brief write to flash
 */
void mx25l25645g_write(mx25l25645g_t *flash, uint8_t *data, uint32_t address, uint32_t size);


/**
 * @brief sector erase
 */
void mx25l25645g_erase_sector(mx25l25645g_t *flash, uint32_t address);

/**
 * @brief read from flash qpi mode
 */
void mx25l25645g_quad_read(mx25l25645g_t *flash, uint8_t *data, uint32_t address, uint32_t size);

/**
 * @brief write dma
 */
void mx25l25645g_write_dma(mx25l25645g_t *flash, uint8_t *data, uint32_t address, uint32_t size);

/**
 * @brief quad read dma
 */
void mx25l25645g_quad_read_dma(mx25l25645g_t *flash, uint8_t *data, uint32_t address, uint32_t size);

/**
 * @brief check programming
 */
mx25l25645g_status_t mx25l25645g_check_programming(mx25l25645g_t *flash);
