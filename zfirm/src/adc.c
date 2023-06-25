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

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
		DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
		                     DT_SPEC_AND_COMMA)
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
	}
	return 0;
}

void adc_do_sample() {
	int16_t buf;
	struct adc_sequence sequence = {
			.buffer = &buf,
			/* buffer size in bytes, not number of samples */
			.buffer_size = sizeof(buf),
	};
	LOG_DBG("ADC reading:");
	for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
		(void) adc_sequence_init_dt(&adc_channels[i], &sequence);
		int err = adc_read(adc_channels[i].dev, &sequence);
		if (err < 0) {
			LOG_ERR("- %s, channel %d: error: %d ",
			        adc_channels[i].dev->name,
			        adc_channels[i].channel_id,
			        err);
		} else {
			LOG_DBG("- %s, channel %d: %"PRId16,
			        adc_channels[i].dev->name,
			        adc_channels[i].channel_id,
			        buf);
		}
	}
}