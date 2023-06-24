#include <zephyr/drivers/pwm.h>
#include "main.h"

// base blinking period
#define PERIOD (PWM_SEC(1U) / 256)

static const struct pwm_dt_spec leds[] = {
		PWM_DT_SPEC_GET(DT_ALIAS(pledr)),
		PWM_DT_SPEC_GET(DT_ALIAS(pledg))
};

int led_init() {
	for (int i = 0; i < ARRAY_SIZE(leds); i++) {
		if (!device_is_ready(leds[i].dev)) {
			LOG_ERR("Error: PWM device %s is not ready\n",
			        leds[i].dev->name);
			return -1;
		}
	}
	return 0;
}

void led_set(led_t led, uint8_t duty) {
	unsigned t = PERIOD;
	unsigned p = t * duty / UINT8_MAX;
	int rc = pwm_set_dt(&leds[led], t, p);
	//LOG_INF("setting led %d, to period %d, pulse %d", led, t, p);
	if (rc) {
		LOG_ERR("cannot set led %d pwm", led);
	}
}