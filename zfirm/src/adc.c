#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>
#include "main.h"

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
    !DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
    ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

#define NCHAN   DT_PROP_LEN(DT_PATH(zephyr_user), io_channels)

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
		DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
		                     DT_SPEC_AND_COMMA)
};

static int16_t buf[NCHAN];

static struct adc_sequence sequence[NCHAN] = {
		{
				.buffer = &buf[0],
				.buffer_size = sizeof(buf[0]),
		},
		{
				.buffer = &buf[1],
				.buffer_size = sizeof(buf[1]),
		},
		{
				.buffer = &buf[2],
				.buffer_size = sizeof(buf[2]),
		},
};

static const char *adc_chan_name[NCHAN] = {
		"EXT",
		"PSUPPLY",
		"SENS"
};

int adc_init() {

	/* Configure channels individually prior to sampling. */
	for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
		if (!device_is_ready(adc_channels[i].dev)) {
			LOG_ERR("ADC controller device not ready");
			return -EINVAL;
		}

		int rc = adc_channel_setup_dt(&adc_channels[i]);
		if (rc < 0) {
			LOG_ERR("Could not setup channel #%d (%d)", i, rc);
			return -EINVAL;
		}
		rc = adc_sequence_init_dt(&adc_channels[i], &sequence[i]);
		if (rc < 0) {
			LOG_ERR("Could not setup channel #%d sequence (%d)", i, rc);
			return -EINVAL;
		}
	}
	return 0;
}

static float adc2v(float adc) {
	return 3.197713845068536e-10 * adc * adc + 0.0008332518627677381 * adc + -0.09870571280531593;
}

void adc_do_sample() {
	for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
		int err = adc_read(adc_channels[i].dev, &sequence[i]);
		if (err < 0) {
			LOG_ERR("- %s, channel %d: error: %d ",
			        adc_channels[i].dev->name,
			        adc_channels[i].channel_id,
			        err);
		} else {
			printk("- %s, channel %d (%s): %"PRId16" (%.3f) V\n",
			       adc_channels[i].dev->name,
			       adc_channels[i].channel_id,
			       adc_chan_name[i],
			       buf[i],
				   adc2v(buf[i])
			);
		}
	}
}