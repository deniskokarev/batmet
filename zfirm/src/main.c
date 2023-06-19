#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#define LOG_REGISTERED

#include "main.h"
#include "opamp.h"

LOG_MODULE_REGISTER(batmet, LOG_LEVEL_DBG);

static const struct gpio_dt_spec red_led = GPIO_DT_SPEC_GET(DT_ALIAS(ledr), gpios);
static const struct gpio_dt_spec green_led = GPIO_DT_SPEC_GET(DT_ALIAS(ledg), gpios);

void Error_Handler() {
	LOG_ERR("opamp initialization error");
}

int main(void) {
	// assuming an error
	gpio_pin_set_dt(&red_led, 1);
	gpio_pin_set_dt(&green_led, 1);

	if (!gpio_is_ready_dt(&red_led) || !gpio_is_ready_dt(&green_led)) {
		LOG_ERR("gpio_is_ready_dt() error");
		return -1;
	}
	if (gpio_pin_configure_dt(&red_led, GPIO_OUTPUT_ACTIVE) < 0 ||
	    gpio_pin_configure_dt(&green_led, GPIO_OUTPUT_ACTIVE) < 0) {
		LOG_ERR("gpio_pin_configure_dt() error");
		return -2;
	}

	int rc = dac_do_init();
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

	int led_state = 0; // start with green
	while (1) {
		int dac_phase = 1; // start hi
		gpio_pin_set_dt(&red_led, led_state);
		gpio_pin_set_dt(&green_led, !led_state);
		led_state = !led_state;
		adc_do_sample();
		if (run_dac_cycle) {
			dac_do_run_cycle(&dac_phase);
		} else {
			k_sleep(K_MSEC(5000));
		}
	}
	return 0;
}
