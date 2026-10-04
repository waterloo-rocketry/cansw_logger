#include "stm32h7xx_hal.h"

typedef struct {
    QSPI_HandleTypeDef *hqspi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
} mx25l25645g_t;

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