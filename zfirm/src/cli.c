#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

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
	if (v < 0 || v > 12) {
		shell_error(sh, "the voltage must be in [0..12] range");
		return -EINVAL;
	}
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_batmet,
                               SHELL_CMD_ARG(v, NULL, "Set desired voltage", cmd_v, 2, 0),
                               SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(bm, &sub_batmet, "Battery meter commands", NULL);
