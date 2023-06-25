//
// Created by Denis Kokarev on 6/12/23.
//

#ifndef BATMET_MAIN_H
#define BATMET_MAIN_H

#include <zephyr/logging/log.h>
#include "soc.h"

#ifndef LOG_REGISTERED
LOG_MODULE_DECLARE(batmet, LOG_LEVEL_INF);
#endif

// CubeMX-compatible error handler
void Error_Handler();

int adc_init();

void adc_do_sample();

extern int run_dac_cycle;

void dac_do_run_cycle(int *dac_phase);

int dac_do_init();

int dac_set_d(int d);

int dac_set_v(float v);

typedef enum {
	DAC_OK = 0,
	DAC_ERR_RANGE,
	DAC_ERR_LOW,
	DAC_ERR_HIGH,
} dac_err_t;

const char *dac_err_str(dac_err_t err);

/************** LED ***************/

typedef enum {
	LED_GREEN = 0,
	LED_RED,
	LED_SZ
} led_t;

#define LED_MAX ((uint8_t)255)
#define LED_HALF ((uint8_t)127)
#define LED_OFF ((uint8_t)0)

#define LED_FLOW_SLOW (1.0/3)
#define LED_FLOW_NORMAL 1.0
#define LED_FLOW_FAST 2.0

/**
 * Init LEDs and associated PWM manager
 * @return 0 on success
 */
int led_init();

/**
 * Glow the given LED at certain level.
 * @param led LED_RED or LED_GREEN
 * @param duty 0 => dark, 255 => glow 100%
 */
void led_set(led_t led, uint8_t duty);

/**
 * Automatically change LED glowing level.
 * It'll become brighter/darker in sinusoidal fashion.
 * @param led LED_RED or LED_GREEN
 * @param hz frequency in Hz, e.g: 1.0/3 means every 3 seconds
 */
void led_flow(led_t led, float freq);

#endif //BATMET_MAIN_H
