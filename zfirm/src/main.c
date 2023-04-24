#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(batmet, 4);

int main(void) {
	while (1) {
		LOG_INF("Hello World! %s\n", CONFIG_ARCH);
		k_sleep(K_MSEC(10000));
	}
	return 0;
}
