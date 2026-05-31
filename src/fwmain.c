#include "canlib.h"
#include "main.h"

#include "fs.h"
#include "health_check.h"
#include "log.h"
#include "mx25l25645g.h"
#include "platform.h"

#define BOARD_HEARTBEAT_PERIOD 500

extern FDCAN_HandleTypeDef hfdcan1;

void can_callback_function(const can_msg_t *message, uint32_t) {
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
	stm32h7_can_init(&hfdcan1, can_callback_function);

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
	mx25l25645g_init();

	for (;;) {
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
	}
}

void w_assert_fail(const char *file, int line, const char *statement) {
	NVIC_SystemReset();
}
