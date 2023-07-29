#include <stdlib.h>
#include <zephyr/console/console.h>
#include "main.h"

/*******************************************
 * Custom cmd-line module when shell is not available
 ********************************************/

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
	long df = 1;
	if (errno) {
		return -EINVAL;
	}
	if (*s == '.') {
		s++;
		char *sf;
		lo = strtol(s, &sf, 10);
		if (errno || lo < 0) {
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
	*res = (double) hi + ((double) lo) / df;
	return 0;
}

static void cmd_help(int argc, char **argv) {
	printk("Supported commands:\n"
	       "\tsink <amperes> - set desired current sink\n"
	       "\td <dac_value> - set dac output value [0..4095]\n"
	       "\ta - sample all ADC channels\n"
	       "\tcsink <min> <max> <a> <b> <c> - store current sink calibration coefficients from cal_i2d.py computation\n"
	);
}

static void cmd_sink(int argc, char *argv[]) {
	if (argc < 2) {
		printk("expecting only one argument - current\n");
	}
	float a;
	int rc = my_strtof(&a, argv[1]);
	if (rc || a < 0) {
		printk("the current must be positive number\n");
	} else {
		rc = dac_set_current(a);
		if (rc) {
			printk("could not set sink current with error: %s\n", dac_err_str(rc));
		} else {
			if (a > 0) {
				printk("sinking = %f A\n", a);
				led_flow(LED_GREEN, LED_FLOW_NORMAL);
			} else {
				printk("sink = off\n");
				led_set(LED_GREEN, LED_ON);
			}
		}
	}
}

static void cmd_d(int argc, char *argv[]) {
	if (argc < 2) {
		printk("expecting only one argument - dac [0..4095]\n");
	}
	char *endp;
	int d = strtol(argv[1], &endp, 10);
	if (d < 0 || d > 4095 || !(*endp == '\r' || *endp == '\n' || *endp == '\t' || *endp == ' ' || *endp == 0)) {
		printk("DAC value must be a number [0..4095]\n");
	} else {
		int rc = dac_set_d(d);
		if (rc) {
			printk("could not set DAC value, error: %s\n", dac_err_str(rc));
		} else {
			if (d > 0) {
				printk("DAC = %d\n", d);
				led_flow(LED_GREEN, LED_FLOW_NORMAL);
			} else {
				printk("DAC = off\n");
				led_set(LED_GREEN, LED_ON);
			}
		}
	}
}

static void cmd_a(int argc, char *argv[]) {
	adc_do_sample();
}

static void cmd_csink(int argc, char *argv[]) {
	if (argc != 6) {
		printk("expecting 5 args: cals <min> <max> <a> <b> <c>\n");
		return;
	}
	float f[5];
	for (int i = 1; i < 6; i++) {
		int rc = my_strtof(&f[i - 1], argv[i]);
		if (rc) {
			printk("%d-th argument to csink must be a floating number, got %s instead\n", i, argv[i]);
			return;
		}
	}
	dac_err_t rc = dac_set_calibration(f[0], f[1], f[2], f[3], f[4]);
	if (rc) {
		printk("csink failed with %s\n", dac_err_str(rc));
	}
}


static void proc_line(char *s) {
	static const char sep[] = " \t";
	char *tok[6] = {};
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
		} else if (strcmp(tok[0], "s") == 0 || strcmp(tok[0], "sink") == 0) {
			cmd_sink(tok_sz, tok);
		} else if (strcmp(tok[0], "d") == 0 || strcmp(tok[0], "dac") == 0) {
			cmd_d(tok_sz, tok);
		} else if (strcmp(tok[0], "a") == 0 || strcmp(tok[0], "adc") == 0) {
			cmd_a(tok_sz, tok);
		} else if (strcmp(tok[0], "cs") == 0 || strcmp(tok[0], "csink") == 0) {
			cmd_csink(tok_sz, tok);
		} else {
			printk("Unknown command '%s', try help\n", tok[0]);
		}
	}
}

/**
 * Process one input command
 */
void cmd_proc() {
	printk("$ ");
	char *s = console_getline();
	proc_line(s);
}
