#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>

#define LOG_REGISTERED

#include "main.h"
#include "opamp.h"

LOG_MODULE_REGISTER(batmet, LOG_LEVEL_DBG);

void Error_Handler() {
	LOG_ERR("opamp initialization error");
}

int main(void) {
	int rc = led_init();
	if (rc) {
		return rc;
	}
	led_set(LED_GREEN, LED_OFF);
	led_set(LED_RED, LED_MAX);

	rc = dac_do_init();
	if (rc) {
		return rc;
	}

	MX_OPAMP2_Init();
	rc = HAL_OPAMP_Start(&hopamp2);
	if (rc != HAL_OK) {
		LOG_ERR("opamp start error: %d", rc);
		return -5;
	}

	rc = adc_init();
	if (rc) {
		LOG_ERR("ADC init error: %d", rc);
		return -6;
	}

	rc = settings_subsys_init();
	if (rc) {
		LOG_ERR("settings subsys initialization: fail (err %d)", rc);
		return -7;
	}
	LOG_INF("settings subsys initialization: OK");

	led_set(LED_RED, LED_OFF);
	led_flow(LED_GREEN, LED_FLOW_FAST);
	while (1) {
		int dac_phase = 1; // start hi
		adc_do_sample();
		if (run_dac_cycle) {
			dac_do_run_cycle(&dac_phase);
		} else {
			k_sleep(K_MSEC(5000));
		}
	}
	return 0;
}
