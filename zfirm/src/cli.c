#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include "main.h"

static int cmd_d(const struct shell *sh, size_t argc, char **argv) {
	char *s;
	long d = strtol(argv[1], &s, 10);
	if (*s != 0) {
		shell_error(sh, "the voltage should be a number");
		return -EINVAL;
	}
	int rc = dac_set_d(d);
	if (rc) {
		shell_error(sh, "could not set voltage with error: %d", rc);
	}
	return 0;
}

static int cmd_v(const struct shell *sh, size_t argc, char **argv) {
	char *s;
	long hi = strtol(argv[1], &s, 10);
	long lo = 0;
	int df = 1;
	if (*s == '.') {
		s++;
		char *sf;
		lo = strtol(s, &sf, 10);
		int l = sf - s;
		s = sf;
		while (l--) {
			df *= 10;
		}
	}
	if (*s != 0) {
		shell_error(sh, "the voltage should be a number");
		return -EINVAL;
	}
	double v = hi + ((double) lo) / df;
	shell_print(sh, "setting voltage %d.%03d", (int) v, (int) ((v - (int) v) * 1000));
	int rc = dac_set_v(v);
	if (rc) {
		shell_error(sh, "could not set voltage with error: %d", rc);
	}
	return 0;
}

static int cmd_dac_cycle(const struct shell *shell, size_t argc, char **argv, void *data) {
	run_dac_cycle = (intptr_t) data;
	shell_print(shell, "dac_cycle = %s", run_dac_cycle ? "on" : "off");
	return 0;
}

SHELL_SUBCMD_DICT_SET_CREATE(sub_dac_cycle_cmds, cmd_dac_cycle, (off, 0, "stop"), (on, 1, "start"));

SHELL_STATIC_SUBCMD_SET_CREATE(sub_batmet,
                               SHELL_CMD_ARG(d, NULL, "Set DAC value [0..4095]", cmd_d, 2, 0),
                               SHELL_CMD_ARG(v, NULL, "Set desired voltage", cmd_v, 2, 0),
                               SHELL_CMD(dac_cycle, &sub_dac_cycle_cmds, "Start/Stop Dac cycle", NULL),
                               SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(bm, &sub_batmet, "Battery meter commands", NULL);
