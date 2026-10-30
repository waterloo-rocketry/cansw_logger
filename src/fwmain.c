#include "canlib.h"
#include "main.h"

#include "fs.h"
#include "health_check.h"
#include "log.h"
#include "platform.h"
#include "mx25l25645g.h"
#include "lfs.h"

#define BOARD_HEARTBEAT_PERIOD 500

extern FDCAN_HandleTypeDef hfdcan1;
extern QSPI_HandleTypeDef hqspi;
extern const struct lfs_config config;
static lfs_t lfs;
static lfs_file_t file;
mx25l25645g_t flash;
MDMA_HandleTypeDef mdma = {0};

static void mdma_init(void){

	__HAL_RCC_MDMA_CLK_ENABLE();
	
	mdma.Instance = MDMA_Channel0;
	mdma.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
	mdma.Init.Request = MDMA_REQUEST_QUADSPI_FIFO_TH;

	/* These are overridden by QSPI HAL during transmit/recieve */
	mdma.Init.DestinationInc = MDMA_DEST_INC_DISABLE;
	mdma.Init.SourceInc = MDMA_SRC_INC_BYTE;

	mdma.Init.BufferTransferLength = 1U;
	mdma.Init.DestDataSize = MDMA_DEST_DATASIZE_BYTE;
	mdma.Init.Priority = MDMA_PRIORITY_HIGH;
	mdma.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
	mdma.Init.TransferTriggerMode = MDMA_BUFFER_TRANSFER;
	mdma.Init.SourceDataSize = MDMA_SRC_DATASIZE_BYTE;

	HAL_MDMA_Init(&mdma);

	__HAL_LINKDMA(&hqspi, hmdma, mdma);

	HAL_NVIC_SetPriority(MDMA_IRQn, 5U, 0U);
	HAL_NVIC_SetPriority(QUADSPI_IRQn, 5U, 0U);
	HAL_NVIC_EnableIRQ(MDMA_IRQn);
	HAL_NVIC_EnableIRQ(QUADSPI_IRQn);

}


void can_callback_function(const can_msg_t *message) {
	switch (get_message_type(message)) {
		case MSG_LEDS_ON:
			LED_RED_ON();
			LED_GREEN_ON();
			break;
		case MSG_LEDS_OFF:
			LED_RED_OFF();
			LED_GREEN_OFF();
			break;
		case MSG_RESET_CMD:
			bool need_reset = false;
			check_board_need_reset(message, &need_reset);
			if (need_reset) {
				NVIC_SystemReset();
			}
			break;
		default:
			break;
	}

	log_handle_incoming_message(message, millis());
}

uint32_t last_board_status_msg = 0;
bool green_led_on = false;

void fwmain(void) {

	int status;
	stm32h7_can_init(&hfdcan1, can_callback_function);
	mdma_init();
	mx25l25645g_init(&flash, &hqspi, GPIOB, GPIO_PIN_10);

	uint8_t data = 0x67U;
	uint8_t read_buffer = 0U;
	uint32_t addr = 0xFFFU;

	mx25l25645g_erase_sector(&flash, 0x0U);

	mx25l25645g_write_dma(&flash, &data, addr, 1U);

	/*

	int err = lfs_mount(&lfs, &config);

	if (err){
		lfs_format(&lfs, &config);
		lfs_mount(&lfs, &config);
	}

	const char *msg = "hello!!";

	char buffer[16] = {0};

	//status = lfs_file_open(&lfs, &file, "test", LFS_O_CREAT | LFS_O_RDWR);
	status = lfs_file_open(&lfs, &file, "test", LFS_O_RDONLY);
	//status = lfs_file_write(&lfs, &file, msg, strlen(msg) + 1);
	status = lfs_file_read(&lfs, &file, buffer, sizeof(buffer));
	status = lfs_file_close(&lfs, &file);

	/*
	if (fs_init() != W_SUCCESS) { // Filesystem Init Failed
		for (;;) {
			can_msg_t msg;
			uint32_t general_error_code = health_check();
			build_general_board_status_msg(
				PRIO_HIGH, millis(), general_error_code | (1 << E_FS_ERROR_OFFSET), &msg);
			stm32h7_can_send(&msg);
			HAL_Delay(500);
		}
	}
	
	log_init();
	*/
	

	for (;;) {

		if (!flash.rx_in_progress && mx25l25645g_check_programming(&flash) == MX25L25645G_STATUS_OK){
			mx25l25645g_quad_read_dma(&flash, &read_buffer, addr, 1U);
		}

		/*
		if (millis() - last_board_status_msg > BOARD_HEARTBEAT_PERIOD) {
			last_board_status_msg = millis();

			can_msg_t msg;
			uint32_t general_error_code = health_check();

			build_general_board_status_msg(PRIO_HIGH, millis(), general_error_code, &msg);
			stm32h7_can_send(&msg);

			HAL_Delay(20); // FIXME cannot transmit 3 messages back to back workaround

			build_analog_sensor_32bit_msg(
				PRIO_LOW, millis(), SENSOR_LOG_WRITTEN_SIZE, fs_get_log_written_size(), &msg);
			stm32h7_can_send(&msg);

			build_analog_sensor_32bit_msg(
				PRIO_LOW, millis(), SENSOR_SD_LOG_FILE_NAME, fs_get_sd_log_file_name(), &msg);
			stm32h7_can_send(&msg);

			if (green_led_on) {
				LED_GREEN_OFF();
				green_led_on = false;
			} else {
				LED_GREEN_ON();
				green_led_on = true;
			}
		}

		log_heartbeat();
		*/
	}
		
}

void w_assert_fail(const char *file, int line, const char *statement) {
	NVIC_SystemReset();
}
