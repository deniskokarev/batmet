#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/dac.h>
#include "opamp.h"


#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)

#if (DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, dac) && \
	DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, dac_channel_id) && \
	DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, dac_resolution))
#define DAC_NODE DT_PHANDLE(ZEPHYR_USER_NODE, dac)
#define DAC_CHANNEL_ID DT_PROP(ZEPHYR_USER_NODE, dac_channel_id)
#define DAC_RESOLUTION DT_PROP(ZEPHYR_USER_NODE, dac_resolution)
#else
#error "Unsupported board: see README and check /zephyr,user node"
#define DAC_NODE DT_INVALID_NODE
#define DAC_CHANNEL_ID 0
#define DAC_RESOLUTION 0
#endif

LOG_MODULE_REGISTER(batmet, LOG_LEVEL_DBG);

static const struct gpio_dt_spec red_led = GPIO_DT_SPEC_GET(DT_ALIAS(ledr), gpios);
static const struct gpio_dt_spec green_led = GPIO_DT_SPEC_GET(DT_ALIAS(ledg), gpios);

static const struct device *const dac_dev = DEVICE_DT_GET(DAC_NODE);

static const struct dac_channel_cfg dac_ch_cfg = {
		.channel_id  = DAC_CHANNEL_ID,
		.resolution  = DAC_RESOLUTION
};

void Error_Handler() {
	LOG_ERR("opamp initialization error");
}

int main(void) {
	MX_OPAMP2_Init();

	if (!gpio_is_ready_dt(&red_led) || !gpio_is_ready_dt(&green_led)) {
		LOG_ERR("gpio_is_ready_dt() error");
		return -1;
	}
	if (gpio_pin_configure_dt(&red_led, GPIO_OUTPUT_ACTIVE) < 0 ||
	    gpio_pin_configure_dt(&green_led, GPIO_OUTPUT_ACTIVE) < 0) {
		LOG_ERR("gpio_pin_configure_dt() error");
		return -2;
	}

	if (!device_is_ready(dac_dev)) {
		LOG_ERR("DAC device %s is not ready", dac_dev->name);
		return -3;
	}

	int rc = dac_channel_setup(dac_dev, &dac_ch_cfg);
	if (rc != 0) {
		LOG_ERR("Setting up of DAC channel failed with code %d", rc);
		return -4;
	}

	int dac_phase = 1; // start hi
	const int dac_range = 1U << DAC_RESOLUTION;

	int led_state = 0; // start with green
	while (1) {
		LOG_INF("Hello World! %s", CONFIG_ARCH);
		gpio_pin_set_dt(&red_led, led_state);
		gpio_pin_set_dt(&green_led, !led_state);
		led_state = !led_state;
		rc = dac_write_value(dac_dev, DAC_CHANNEL_ID, (dac_range-1) * dac_phase);
		if (rc != 0) {
			LOG_ERR("dac_write_value() failed with code %d", rc);
			return -5;
		}
		dac_phase = !dac_phase;
		k_sleep(K_MSEC(5000));
	}
	return 0;
}
