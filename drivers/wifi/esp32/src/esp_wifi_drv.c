/*
 * Copyright (c) 2020 Espressif Systems (Shanghai) Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT espressif_esp32_wifi

#define LOG_MODULE_NAME esp32_wifi
#define LOG_LEVEL CONFIG_ETHERNET_LOG_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#include <net/ethernet.h>
#include <net/net_pkt.h>
#include <net/net_if.h>
#include <device.h>
#include <soc.h>
#include <ethernet/eth_stats.h>
#include "esp_networking_priv.h"
#include "esp_private/wifi.h"
#include "esp_event.h"
#include "esp_timer.h"
#include "wifi_system.h"
#include "esp_wpa.h"

#define DEV_DATA(dev) \
	((struct esp32_wifi_runtime *)(dev)->data)

struct esp32_wifi_runtime {
	struct net_if *iface;
	uint8_t mac_addr[6];
	bool tx_err;
	uint32_t tx_word;
	int tx_pos;
#if defined(CONFIG_NET_STATISTICS_ETHERNET)
	struct net_stats_eth stats;
#endif
};

static int eth_esp32_send(const struct device *dev, struct net_pkt *pkt)
{
	const int pkt_len = net_pkt_get_len(pkt);

	uint8_t *tx_buffer = (uint8_t *) k_malloc(pkt_len);
	if (tx_buffer == NULL) {
		return -EIO;
	}

	/* Read the packet payload */
	if (net_pkt_read(pkt, tx_buffer, pkt_len) < 0) {
		k_free(tx_buffer);
		return -EIO;
	}

	/* Enqueue packet for transmission */
	esp_wifi_internal_tx(ESP_IF_WIFI_STA, (void *)tx_buffer, pkt_len);
	k_yield();

	LOG_DBG("pkt sent %p len %d", pkt, pkt_len);
	k_free(tx_buffer);

	return 0;
}

esp_err_t eth_esp32_rx(void *buffer, uint16_t len, void *eb)
{
	struct net_pkt *pkt;
	struct net_if *iface;
	iface = net_if_get_default();
	pkt = net_pkt_rx_alloc_with_buffer(iface, len,
			AF_UNSPEC, 0, K_NO_WAIT);
	if (!pkt) {
		LOG_ERR("Failed to get net buffer");
		return ESP_FAIL;
	}

	if (net_pkt_write(pkt, buffer, len) < 0) {
		LOG_ERR("Failed to write pkt");
		goto pkt_unref;
	}

	if (net_recv_data(iface, pkt) < 0) {
		LOG_ERR("Failed to push received data");
		goto pkt_unref;
	}
	esp_wifi_internal_free_rx_buffer(eb);
	return ESP_OK;

pkt_unref:
	net_pkt_unref(pkt);
	return ESP_FAIL;
}

void esp_wifi_register_rx_callback(void)
{
	esp_wifi_internal_reg_rxcb(ESP_IF_WIFI_STA, (wifi_rxcb_t)eth_esp32_rx);
	struct net_if *iface;
	iface = net_if_get_default();
	net_if_up(iface);
}

static void eth_esp32_init(struct net_if *iface)
{
	const struct device *dev = net_if_get_device(iface);
	struct esp32_wifi_runtime *dev_data = DEV_DATA(dev);

	dev_data->iface = iface;
	/* Start interface when we are actually connected with WiFi network */
	net_if_flag_set(iface, NET_IF_NO_AUTO_START);
	esp_read_mac(dev_data->mac_addr, ESP_MAC_WIFI_STA);

	/* Assign link local address. */
	net_if_set_link_addr(iface,
			dev_data->mac_addr, 6, NET_LINK_ETHERNET);

	ethernet_init(iface);
}

#if defined(CONFIG_NET_STATISTICS_ETHERNET)
static struct net_stats_eth *eth_esp32_stats(const struct device *dev)
{
	return &(DEV_DATA(dev)->stats);
}
#endif

static int eth_esp32_dev_init(const struct device *dev)
{
	esp_timer_init();
	esp_event_init();
	wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT() ;
	esp_err_t ret = esp_wifi_init(&config);
	ret |= esp_supplicant_init();
	ret |= esp_wifi_start();
	ret |= esp_wifi_set_mode(WIFI_MODE_STA);

	wifi_config_t wifi_config = {
		.sta = {
			.ssid = CONFIG_WIFI_SSID,
			.password = CONFIG_WIFI_PASSWORD,
		},
	};

	ret |= esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
	ret |= esp_wifi_connect();
	if (ret != ESP_OK) {
		LOG_ERR("Connect failed");
	}

	return ret;
}

DEVICE_DECLARE(eth_esp32);

struct esp32_wifi_runtime eth_data = {
	.mac_addr = {0},
	.tx_err = false,
	.tx_word = 0,
	.tx_pos = 0,
};

static const struct ethernet_api eth_esp32_apis = {
	.iface_api.init	= eth_esp32_init,
	.send =  eth_esp32_send,
#if defined(CONFIG_NET_STATISTICS_ETHERNET)
	.get_stats = eth_esp32_stats,
#endif
};

NET_DEVICE_INIT(eth_esp32, DT_INST_LABEL(0),
		eth_esp32_dev_init, device_pm_control_nop,
		&eth_data, NULL, CONFIG_ETH_INIT_PRIORITY,
		&eth_esp32_apis, ETHERNET_L2,
		NET_L2_GET_CTX_TYPE(ETHERNET_L2), NET_ETH_MTU);
