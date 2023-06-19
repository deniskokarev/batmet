#include <zephyr/kernel.h>
#include <zephyr/drivers/dac.h>
#include "main.h"

#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)

#if (DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, dac) && \
    DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, dac_channel_id) && \
    DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, dac_resolution))
#define DAC_NODE DT_PHANDLE(ZEPHYR_USER_NODE, dac)
#define DAC_CHANNEL_ID DT_PROP(ZEPHYR_USER_NODE, dac_channel_id)
#define DAC_RESOLUTION DT_PROP(ZEPHYR_USER_NODE, dac_resolution)
#else
#error "Unsupported board: see README and check /zephyr,user node"
#define DAC_NODE DT_INVALID_NODE
#define DAC_CHANNEL_ID 0
#define DAC_RESOLUTION 0
#endif

static const struct device *const dac_dev = DEVICE_DT_GET(DAC_NODE);

static const struct dac_channel_cfg dac_ch_cfg = {
		.channel_id  = DAC_CHANNEL_ID,
		.resolution  = DAC_RESOLUTION
};

int run_dac_cycle = 0;

void dac_do_run_cycle(int *dac_phase) {
	const int dac_range = 1U << DAC_RESOLUTION;
	int d = *dac_phase ? +1 : -1;
	int i = *dac_phase ? 0 : (dac_range - 1);
	do {
		int rc = dac_write_value(dac_dev, DAC_CHANNEL_ID, i);
		if (rc != 0) {
			LOG_ERR("dac_write_value() failed with code %d", rc);
		}
		i += d;
		i %= dac_range;
		k_sleep(K_MSEC(1));
	} while (i != 0);
	*dac_phase = !*dac_phase;
}

int dac_do_init() {
	if (!device_is_ready(dac_dev)) {
		LOG_ERR("DAC device %s is not ready", dac_dev->name);
		return -3;
	}

	int rc = dac_channel_setup(dac_dev, &dac_ch_cfg);
	if (rc != 0) {
		LOG_ERR("Setting up of DAC channel failed with code %d", rc);
		return -4;
	}
	return 0;
}

static const float MIN_V = 0.0033;
static const float MAX_V = 1.5375;
static const int MIN_D = 250;
static const int MAX_D = 3900;

const char *dac_err_str(dac_err_t err) {
	return "supported voltages between 0.0033 and 1.5375";
}

/**
 * @param dac - value for the given voltage
 * @param v voltage
 * @return 0 if no error
 */
static int v2dac(int *dac, float v) {
	if (v < MIN_V) {
		return DAC_ERR_LOW;
	} else if (v > MAX_V) {
		return DAC_ERR_HIGH;
	} else {
		v -= MIN_V;
		*dac = MIN_D + v * (MAX_D - MIN_D) / (MAX_V - MIN_V);
		return DAC_OK;
	}
}

int dac_set_v(float v) {
	int d;
	int rc = v2dac(&d, v);
	if (rc != DAC_OK) {
		LOG_ERR("wrong voltage, error %s", dac_err_str(rc));
		return rc;
	}
	rc = dac_write_value(dac_dev, DAC_CHANNEL_ID, d);
	if (rc != 0) {
		LOG_ERR("dac_write_value() failed with code %d", rc);
		return rc;
	}
	return rc;
}

int dac_set_d(int d) {
	if (d < 0 || d > 4095) {
		LOG_ERR("wrong voltage, supported [0..4095]");
		return DAC_ERR_RANGE;
	}
	int rc = dac_write_value(dac_dev, DAC_CHANNEL_ID, d);
	if (rc != 0) {
		LOG_ERR("dac_write_value() failed with code %d", rc);
		return rc;
	}
	return rc;
}