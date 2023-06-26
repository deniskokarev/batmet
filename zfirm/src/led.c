#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include "main.h"

#define M_PI        3.14159265358979323846
#define M_PI_2      1.57079632679489661923

// base blinking period
#define PERIOD (PWM_SEC(1U) / 256)

static const struct pwm_dt_spec leds[LED_SZ] = {
		PWM_DT_SPEC_GET(DT_ALIAS(pledr)),
		PWM_DT_SPEC_GET(DT_ALIAS(pledg))
};

typedef struct {
	struct k_work_delayable update_work;
	led_t led;
	float hz;
} led_ctx_t;

led_ctx_t led_ctx[LED_SZ];

#define SIN_PI2_SZ  16

// saving about 3KB by using no libm
static float crude_sinf(float x) {
	static const float sin_pi2[SIN_PI2_SZ + 1] = {
			0.0,
			0.0980171403295606,
			0.19509032201612825,
			0.29028467725446233,
			0.3826834323650898,
			0.47139673682599764,
			0.5555702330196022,
			0.6343932841636455,
			0.7071067811865475,
			0.7730104533627369,
			0.8314696123025451,
			0.8819212643483549,
			0.9238795325112867,
			0.9569403357322089,
			0.9807852804032304,
			0.9951847266721968,
			1.0,
	};
	static const float f = M_PI_2 / SIN_PI2_SZ;
	int p = x / (2 * M_PI);
	x -= p * (2 * M_PI);
	p = x / M_PI_2;
	x -= p * M_PI_2;
	int i = x / f;
	switch (p) {
		case 0:
			return sin_pi2[i];
		case 2:
			return -sin_pi2[i];
		case 1:
			return sin_pi2[SIN_PI2_SZ - i];
		case 3:
			return -sin_pi2[SIN_PI2_SZ - i];
		default:
			return 0; // __ASSERT("must not get here");
	}
}

// update LED duty cycle this often
#define FLOW_UPDATE_T K_MSEC(1000 / 16)

/**
 * Periodic job that updates LED PWM level. Started by led_flow()
 * @param work
 */
static void led_update_handler(struct k_work *work) {
	struct k_work_delayable *dwork = k_work_delayable_from_work(work);
	led_ctx_t *ctx = CONTAINER_OF(dwork, led_ctx_t, update_work);
	float t_sec = ((float)k_uptime_get()) / 1000;
	float w = t_sec * M_PI * ctx->hz; // using sin^2, which has period Pi
	float s = crude_sinf(w);
	uint8_t d = s * s * LED_MAX;
	led_set(ctx->led, d);
	k_work_reschedule(dwork, FLOW_UPDATE_T); // re-trigger next invocation
	LOG_DBG("updated %d led to duty %d", ctx->led, d);
}

/**
 * Init LEDs and associated PWM manager
 * @return 0 on success
 */
int led_init() {
	for (int led = 0; led < ARRAY_SIZE(leds); led++) {
		if (!device_is_ready(leds[led].dev)) {
			LOG_ERR("Error: PWM device %s is not ready\n",
			        leds[led].dev->name);
			return -1;
		}
		led_ctx[led].led = led;
		k_work_init_delayable(&led_ctx[led].update_work, led_update_handler);
	}
	return 0;
}

/**
 * Glow the given LED at certain level.
 * @param led LED_RED or LED_GREEN
 * @param duty 0 => dark, 255 => glow 100%
 */
void led_set(led_t led, uint8_t duty) {
	k_work_cancel_delayable(&led_ctx[led].update_work);
	unsigned t = PERIOD;
	unsigned p = t * duty / UINT8_MAX;
	int rc = pwm_set_dt(&leds[led], t, p);
	LOG_DBG("setting led %d, to period %d, pulse %d", led, t, p);
	if (rc) {
		LOG_ERR("cannot set led %d pwm", led);
	}
}

/**
 * Automatically change LED glowing level.
 * It'll become brighter/darker in sinusoidal fashion.
 * @param led LED_RED or LED_GREEN
 * @param hz frequency in Hz, e.g: 1.0/3 means every 3 seconds
 */
void led_flow(led_t led, float hz) {
	led_ctx[led].hz = hz;
	k_work_schedule(&led_ctx[led].update_work, FLOW_UPDATE_T);
}
