#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(batmet, LOG_LEVEL_DBG);

static const struct gpio_dt_spec red_led = GPIO_DT_SPEC_GET(DT_ALIAS(ledr), gpios);
static const struct gpio_dt_spec green_led = GPIO_DT_SPEC_GET(DT_ALIAS(ledg), gpios);

int main(void) {
	if (!gpio_is_ready_dt(&red_led) || !gpio_is_ready_dt(&green_led)) {
		LOG_ERR("gpio_is_ready_dt() error");
		return -1;
	}
	if (gpio_pin_configure_dt(&red_led, GPIO_OUTPUT_ACTIVE) < 0 ||
	    gpio_pin_configure_dt(&green_led, GPIO_OUTPUT_ACTIVE) < 0) {
		LOG_ERR("gpio_pin_configure_dt() error");
		return -2;
	}

	int led_state = 0; // start with green
	while (1) {
		LOG_INF("Hello World! %s", CONFIG_ARCH);
		gpio_pin_set_dt(&red_led, led_state);
		gpio_pin_set_dt(&green_led, !led_state);
		led_state = !led_state;
		k_sleep(K_MSEC(5000));
	}
	return 0;
}
