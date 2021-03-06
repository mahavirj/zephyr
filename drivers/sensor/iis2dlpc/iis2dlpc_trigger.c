/* ST Microelectronics IIS2DLPC 3-axis accelerometer driver
 *
 * Copyright (c) 2020 STMicroelectronics
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Datasheet:
 * https://www.st.com/resource/en/datasheet/iis2dlpc.pdf
 */

#define DT_DRV_COMPAT st_iis2dlpc

#include <kernel.h>
#include <drivers/sensor.h>
#include <drivers/gpio.h>
#include <logging/log.h>

#include "iis2dlpc.h"

LOG_MODULE_DECLARE(IIS2DLPC, CONFIG_SENSOR_LOG_LEVEL);

/**
 * iis2dlpc_enable_int - enable selected int pin to generate interrupt
 */
static int iis2dlpc_enable_int(struct device *dev,
			       enum sensor_trigger_type type, int enable)
{
	const struct iis2dlpc_device_config *cfg = dev->config;
	struct iis2dlpc_data *iis2dlpc = dev->data;
	iis2dlpc_reg_t int_route;

	if (cfg->int_pin == 1U) {
		/* set interrupt for pin INT1 */
		iis2dlpc_pin_int1_route_get(iis2dlpc->ctx,
				&int_route.ctrl4_int1_pad_ctrl);

		switch (type) {
		case SENSOR_TRIG_DATA_READY:
			int_route.ctrl4_int1_pad_ctrl.int1_drdy = enable;
			break;
#ifdef CONFIG_IIS2DLPC_PULSE
		case SENSOR_TRIG_TAP:
			int_route.ctrl4_int1_pad_ctrl.int1_single_tap = enable;
			break;
		case SENSOR_TRIG_DOUBLE_TAP:
			int_route.ctrl4_int1_pad_ctrl.int1_tap = enable;
			break;
#endif /* CONFIG_IIS2DLPC_PULSE */
		default:
			LOG_ERR("Unsupported trigger interrupt route");
			return -ENOTSUP;
		}

		return iis2dlpc_pin_int1_route_set(iis2dlpc->ctx,
				&int_route.ctrl4_int1_pad_ctrl);
	} else {
		/* set interrupt for pin INT2 */
		iis2dlpc_pin_int2_route_get(iis2dlpc->ctx,
					    &int_route.ctrl5_int2_pad_ctrl);

		switch (type) {
		case SENSOR_TRIG_DATA_READY:
			int_route.ctrl5_int2_pad_ctrl.int2_drdy = enable;
			break;
		default:
			LOG_ERR("Unsupported trigger interrupt route");
			return -ENOTSUP;
		}

		return iis2dlpc_pin_int2_route_set(iis2dlpc->ctx,
				&int_route.ctrl5_int2_pad_ctrl);
	}
}

/**
 * iis2dlpc_trigger_set - link external trigger to event data ready
 */
int iis2dlpc_trigger_set(struct device *dev,
			  const struct sensor_trigger *trig,
			  sensor_trigger_handler_t handler)
{
	struct iis2dlpc_data *iis2dlpc = dev->data;
	union axis3bit16_t raw;
	int state = (handler != NULL) ? PROPERTY_ENABLE : PROPERTY_DISABLE;

	switch (trig->type) {
	case SENSOR_TRIG_DATA_READY:
		iis2dlpc->drdy_handler = handler;
		if (state) {
			/* dummy read: re-trigger interrupt */
			iis2dlpc_acceleration_raw_get(iis2dlpc->ctx, raw.u8bit);
		}
		return iis2dlpc_enable_int(dev, SENSOR_TRIG_DATA_READY, state);
#ifdef CONFIG_IIS2DLPC_PULSE
	case SENSOR_TRIG_TAP:
		iis2dlpc->tap_handler = handler;
		return iis2dlpc_enable_int(dev, SENSOR_TRIG_TAP, state);
	case SENSOR_TRIG_DOUBLE_TAP:
		iis2dlpc->double_tap_handler = handler;
		return iis2dlpc_enable_int(dev, SENSOR_TRIG_DOUBLE_TAP, state);
#endif /* CONFIG_IIS2DLPC_PULSE */
	default:
		LOG_ERR("Unsupported sensor trigger");
		return -ENOTSUP;
	}
}

static int iis2dlpc_handle_drdy_int(struct device *dev)
{
	struct iis2dlpc_data *data = dev->data;

	struct sensor_trigger drdy_trig = {
		.type = SENSOR_TRIG_DATA_READY,
		.chan = SENSOR_CHAN_ALL,
	};

	if (data->drdy_handler) {
		data->drdy_handler(dev, &drdy_trig);
	}

	return 0;
}

#ifdef CONFIG_IIS2DLPC_PULSE
static int iis2dlpc_handle_single_tap_int(struct device *dev)
{
	struct iis2dlpc_data *data = dev->data;
	sensor_trigger_handler_t handler = data->tap_handler;

	struct sensor_trigger pulse_trig = {
		.type = SENSOR_TRIG_TAP,
		.chan = SENSOR_CHAN_ALL,
	};

	if (handler) {
		handler(dev, &pulse_trig);
	}

	return 0;
}

static int iis2dlpc_handle_double_tap_int(struct device *dev)
{
	struct iis2dlpc_data *data = dev->data;
	sensor_trigger_handler_t handler = data->double_tap_handler;

	struct sensor_trigger pulse_trig = {
		.type = SENSOR_TRIG_DOUBLE_TAP,
		.chan = SENSOR_CHAN_ALL,
	};

	if (handler) {
		handler(dev, &pulse_trig);
	}

	return 0;
}
#endif /* CONFIG_IIS2DLPC_PULSE */

/**
 * iis2dlpc_handle_interrupt - handle the drdy event
 * read data and call handler if registered any
 */
static void iis2dlpc_handle_interrupt(void *arg)
{
	struct device *dev = (struct device *)arg;
	struct iis2dlpc_data *iis2dlpc = dev->data;
	const struct iis2dlpc_device_config *cfg = dev->config;
	iis2dlpc_all_sources_t sources;

	iis2dlpc_all_sources_get(iis2dlpc->ctx, &sources);

	if (sources.status_dup.drdy) {
		iis2dlpc_handle_drdy_int(dev);
	}
#ifdef CONFIG_IIS2DLPC_PULSE
	if (sources.status_dup.single_tap) {
		iis2dlpc_handle_single_tap_int(dev);
	}
	if (sources.status_dup.double_tap) {
		iis2dlpc_handle_double_tap_int(dev);
	}
#endif /* CONFIG_IIS2DLPC_PULSE */

	gpio_pin_interrupt_configure(iis2dlpc->gpio, cfg->int_gpio_pin,
				     GPIO_INT_EDGE_TO_ACTIVE);
}

static void iis2dlpc_gpio_callback(struct device *dev,
				    struct gpio_callback *cb, uint32_t pins)
{
	struct iis2dlpc_data *iis2dlpc =
		CONTAINER_OF(cb, struct iis2dlpc_data, gpio_cb);

	if ((pins & BIT(iis2dlpc->gpio_pin)) == 0U) {
		return;
	}

	gpio_pin_interrupt_configure(dev, iis2dlpc->gpio_pin,
				     GPIO_INT_DISABLE);

#if defined(CONFIG_IIS2DLPC_TRIGGER_OWN_THREAD)
	k_sem_give(&iis2dlpc->gpio_sem);
#elif defined(CONFIG_IIS2DLPC_TRIGGER_GLOBAL_THREAD)
	k_work_submit(&iis2dlpc->work);
#endif /* CONFIG_IIS2DLPC_TRIGGER_OWN_THREAD */
}

#ifdef CONFIG_IIS2DLPC_TRIGGER_OWN_THREAD
static void iis2dlpc_thread(int dev_ptr, int unused)
{
	struct device *dev = INT_TO_POINTER(dev_ptr);
	struct iis2dlpc_data *iis2dlpc = dev->data;

	ARG_UNUSED(unused);

	while (1) {
		k_sem_take(&iis2dlpc->gpio_sem, K_FOREVER);
		iis2dlpc_handle_interrupt(dev);
	}
}
#endif /* CONFIG_IIS2DLPC_TRIGGER_OWN_THREAD */

#ifdef CONFIG_IIS2DLPC_TRIGGER_GLOBAL_THREAD
static void iis2dlpc_work_cb(struct k_work *work)
{
	struct iis2dlpc_data *iis2dlpc =
		CONTAINER_OF(work, struct iis2dlpc_data, work);

	iis2dlpc_handle_interrupt(iis2dlpc->dev);
}
#endif /* CONFIG_IIS2DLPC_TRIGGER_GLOBAL_THREAD */

int iis2dlpc_init_interrupt(struct device *dev)
{
	struct iis2dlpc_data *iis2dlpc = dev->data;
	const struct iis2dlpc_device_config *cfg = dev->config;
	int ret;

	/* setup data ready gpio interrupt (INT1 or INT2) */
	iis2dlpc->gpio = device_get_binding(cfg->int_gpio_port);
	if (iis2dlpc->gpio == NULL) {
		LOG_DBG("Cannot get pointer to %s device",
			    cfg->int_gpio_port);
		return -EINVAL;
	}

#if defined(CONFIG_IIS2DLPC_TRIGGER_OWN_THREAD)
	k_sem_init(&iis2dlpc->gpio_sem, 0, UINT_MAX);

	k_thread_create(&iis2dlpc->thread, iis2dlpc->thread_stack,
		       CONFIG_IIS2DLPC_THREAD_STACK_SIZE,
		       (k_thread_entry_t)iis2dlpc_thread, dev,
		       0, NULL, K_PRIO_COOP(CONFIG_IIS2DLPC_THREAD_PRIORITY),
		       0, K_NO_WAIT);
#elif defined(CONFIG_IIS2DLPC_TRIGGER_GLOBAL_THREAD)
	iis2dlpc->work.handler = iis2dlpc_work_cb;
	iis2dlpc->dev = dev;
#endif /* CONFIG_IIS2DLPC_TRIGGER_OWN_THREAD */

	iis2dlpc->gpio_pin = cfg->int_gpio_pin;

	ret = gpio_pin_configure(iis2dlpc->gpio, cfg->int_gpio_pin,
				 GPIO_INPUT | cfg->int_gpio_flags);
	if (ret < 0) {
		LOG_DBG("Could not configure gpio");
		return ret;
	}

	gpio_init_callback(&iis2dlpc->gpio_cb,
			   iis2dlpc_gpio_callback,
			   BIT(cfg->int_gpio_pin));

	if (gpio_add_callback(iis2dlpc->gpio, &iis2dlpc->gpio_cb) < 0) {
		LOG_DBG("Could not set gpio callback");
		return -EIO;
	}

	/* enable interrupt on int1/int2 in pulse mode */
	if (iis2dlpc_int_notification_set(iis2dlpc->ctx, IIS2DLPC_INT_PULSED)) {
		return -EIO;
	}

	return gpio_pin_interrupt_configure(iis2dlpc->gpio, cfg->int_gpio_pin,
					    GPIO_INT_EDGE_TO_ACTIVE);
}
