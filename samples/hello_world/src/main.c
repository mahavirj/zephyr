/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include "esp_wifi.h"
#include "esp_timer.h"
#include "esp_wpa.h"

void main(void)
{
    esp_timer_init();
    esp_event_init();
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t ret = esp_wifi_init(&config);

    ret |= esp_supplicant_init();
    ret |= esp_wifi_start();
    ret |= esp_wifi_set_mode(WIFI_MODE_STA);
    wifi_config_t wifi_config = {
	.sta = {
	    .ssid = "myssid",
	    .password = "mypassword",
	    /* Setting a password implies station will connect to all security modes including WEP/WPA.
	     * However these modes are deprecated and not advisable to be used. Incase your Access point
	     * doesn't support WPA2, these mode can be enabled by commenting below line */
	    .threshold.authmode = WIFI_AUTH_WPA2_PSK,
	},
    };
    ret |= esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    if (ret != ESP_OK) {
	printk("WiFi Init Failed\n");
    }
}
