#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

int main(void) {
	while (1) {
		printk("Hello World! %s\n", CONFIG_ARCH);
		k_sleep(K_MSEC(1000));
	}
	return 0;
}
