#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>
#include <zephyr/console/console.h>

#define LOG_REGISTERED

#include "main.h"
#include "opamp.h"

LOG_MODULE_REGISTER(batmet, LOG_LEVEL_DBG);

void Error_Handler() {
	LOG_ERR("opamp initialization error");
}

/**
 * convert string to float
 * @param res resulting floating number
 * @param str src string
 * @return 0 on success of -EINVAL on err
 */
int my_strtof(float *res, const char *str) {
	char *s;
	long hi = strtol(str, &s, 10);
	long lo = 0;
	int df = 1;
	if (*s == '.') {
		s++;
		char *sf;
		lo = strtol(s, &sf, 10);
		if (lo < 0) {
			return -EINVAL;
		}
		int l = sf - s;
		s = sf;
		while (l--) {
			df *= 10;
		}
	}
	if (*s != 0) {
		return -EINVAL;
	}
	if (hi < 0) {
		lo = -lo;
	}
	*res = hi + ((float) lo) / df;
	return 0;
}

static void cmd_help(int argc, char **argv) {
	printk("Supported commands:\n"
	       "\tv <voltage> - set desired output voltage\n"
	);
}

static void cmd_v(int argc, char *argv[]) {
	if (argc < 2) {
		printk("expecting only one argument - voltage\n");
	}
	float v;
	int rc = my_strtof(&v, argv[1]);
	if (rc || v < 0) {
		printk("the voltage must be positive number\n");
	} else {
		rc = dac_set_v(v);
		if (rc) {
			printk("could not set voltage with error: %d\n", rc);
		} else {
			if (v > 0) {
				printk("voltage = %f\n", v);
				led_flow(LED_GREEN, LED_FLOW_NORMAL);
			} else {
				printk("voltage = off\n");
				led_set(LED_GREEN, LED_ON);
			}
		}
	}
}

static void proc_line(char *s) {
	static const char sep[] = " \t";
	char *tok[2] = {};
	int tok_sz = 0;
	char *br;
	for (s = strtok_r(s, sep, &br); s; s = strtok_r(NULL, sep, &br)) {
		tok[tok_sz++] = s;
		if (tok_sz >= ARRAY_SIZE(tok)) {
			break;
		}
	}
	if (tok_sz > 0) {
		if (strcmp(tok[0], "help") == 0 || strcmp(tok[0], "h") == 0 || strcmp(tok[0], "?") == 0) {
			cmd_help(tok_sz, tok);
		} else if (strcmp(tok[0], "voltage") == 0 || strcmp(tok[0], "v") == 0) {
			cmd_v(tok_sz, tok);
		} else {
			printk("Unknown command '%s', try help\n", tok[0]);
		}
	}
}

int main(void) {
	console_getline_init();

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
	led_set(LED_GREEN, LED_ON);
	while (1) {
		printk("$ ");
		char *s = console_getline();
		proc_line(s);
	}

	return 0;
}
