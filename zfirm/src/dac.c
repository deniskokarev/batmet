#include <zephyr/kernel.h>
#include <zephyr/drivers/dac.h>
#include <zephyr/settings/settings.h>
#include "main.h"

// increment each time CALIBRATE struct changes
#define CALIBRATION_VER 1

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

const char *dac_err_str(dac_err_t err) {
	static const char *dac_err[DAC_ERR_SZ] = {
			[DAC_OK] = "Success",
			[DAC_ERR_RANGE] = "Out of range",
			[DAC_ERR_LOW] = "Too low",
			[DAC_ERR_HIGH] = "Too high",
			[DAC_ERR_WRITE] = "DAC Write Error",
			[DAC_ERR_NO_CALIB] = "Not calibrated",
			[DAC_ERR_SAVE_CALIBRATION] = "Cannot save calibration",
	};

	return dac_err[err];
}

dac_err_t dac_do_init() {
	if (!device_is_ready(dac_dev)) {
		LOG_ERR("DAC device %s is not ready", dac_dev->name);
		return DAC_ERR_DEVICE_NOT_READY;
	}

	int rc = dac_channel_setup(dac_dev, &dac_ch_cfg);
	if (rc != 0) {
		LOG_ERR("Setting up of DAC channel failed with code %d", rc);
		return DAC_ERR_CHANNEL_SETUP;
	}
	return DAC_OK;
}

typedef struct {
	int ver; // should be 1st
	float min;
	float max;
	float a;
	float b;
	float c;
} CALIBRATE;

static CALIBRATE cal_dac;

int cal_handle_set(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg) {
	ARG_UNUSED(len);
	const char *next;
	size_t name_len = settings_name_next(name, &next);
	if (!next && !strncmp(name, "dac", name_len)) {
		return read_cb(cb_arg, &cal_dac, sizeof(cal_dac)) != sizeof(cal_dac); // 0 means success
	} else {
		return -ENOENT;
	}
}

SETTINGS_STATIC_HANDLER_DEFINE(batmet_dac, "cal",
                               NULL,
                               cal_handle_set,
                               NULL,
                               NULL
);

dac_err_t dac_set_calibration(float mn, float mx, float a, float b, float c) {
	cal_dac = (CALIBRATE) {
			.ver = CALIBRATION_VER,
			.min = mn,
			.max = mx,
			.a = a,
			.b = b,
			.c = c
	};
	int rc = settings_save_one("cal/dac", &cal_dac, sizeof(cal_dac));
	if (rc < 0) {
		LOG_ERR("settings_save_one() error %d", rc);
		return DAC_ERR_SAVE_CALIBRATION;
	} else {
		LOG_INF("cal dac saved: ver=%d, min=%f, max=%f, a=%f, b=%f, c=%f", CALIBRATION_VER, mn, mx, a, b, c);
		return DAC_OK;
	}
}

/**
 * @param dac - value for the given voltage
 * @param v voltage
 * @return 0 if no error
 */
static dac_err_t v2dac(int *dac, float v) {
	if (cal_dac.ver < CALIBRATION_VER) {
		return DAC_ERR_NO_CALIB;
	}
	if (v < cal_dac.min) {
		return DAC_ERR_LOW;
	} else if (v > cal_dac.max) {
		return DAC_ERR_HIGH;
	} else {
		float d = cal_dac.a * v * v + cal_dac.b * v + cal_dac.c;
		*dac = (int) (d + 0.5);
		return DAC_OK;
	}
}

dac_err_t dac_set_v(float v) {
	int d;
	int rc = v2dac(&d, v);
	if (rc != DAC_OK) {
		return rc;
	}
	rc = dac_write_value(dac_dev, DAC_CHANNEL_ID, d);
	if (rc != 0) {
		LOG_ERR("dac_write_value() failed with code %d", rc);
		return DAC_ERR_WRITE;
	}
	return DAC_OK;
}

dac_err_t dac_set_d(int d) {
	if (d < 0 || d > 4095) {
		LOG_ERR("supported DAC values [0..4095]");
		return DAC_ERR_RANGE;
	}
	int rc = dac_write_value(dac_dev, DAC_CHANNEL_ID, d);
	if (rc != 0) {
		LOG_ERR("dac_write_value() failed with code %d", rc);
		return DAC_ERR_WRITE;
	}
	return DAC_OK;
}
