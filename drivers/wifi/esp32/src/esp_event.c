/*
 * Copyright (c) 2020 Espressif Systems (Shanghai) Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include "esp_event.h"
#include "esp_private/wifi.h"
#include "esp_log.h"

const char *TAG = "esp_event";

K_THREAD_STACK_DEFINE(event_task_stack, CONFIG_EVENT_TASK_STACK_SIZE);
static struct k_thread event_task_handle;
static struct k_msgq event_queue;
static void *event_msgq_buffer;

esp_err_t esp_event_send_internal(esp_event_base_t event_base,
			    int32_t event_id,
			    void *event_data,
			    size_t event_data_size,
			    TickType_t ticks_to_wait)
{
    k_msgq_put(&event_queue, (int32_t *)&event_id, K_FOREVER);
    return ESP_OK;
}

void event_task(void *arg)
{
    bool connected_flag = false;
    int32_t event_id;

    while (1) {
	k_msgq_get(&event_queue, &event_id, K_FOREVER);
	switch (event_id) {
	    case SYSTEM_EVENT_STA_START:
	    {
		ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
		esp_wifi_connect();
		break;
	    }

	    case SYSTEM_EVENT_STA_CONNECTED:
	    {
		ESP_LOGI(TAG, "SYSTEM_EVENT_STA_CONNECTED");
		connected_flag = true;
		break;
	    }

	    case SYSTEM_EVENT_STA_DISCONNECTED:
	    {
		ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
		if (!connected_flag) {
		    esp_wifi_connect();
		}
		connected_flag = false;
		break;
	    }
	    default:
		ESP_LOGD(TAG, "Unsupported Event ID");
		break;
	}
    }
}

esp_err_t esp_event_init(void)
{
    event_msgq_buffer = k_malloc(sizeof(system_event_t) * 10);
    if (event_msgq_buffer == NULL) {
	ESP_LOGE(TAG, "Memory allocation failed");
	return ESP_ERR_NO_MEM;
    }
    k_msgq_init(&event_queue, event_msgq_buffer, sizeof(system_event_t), 10);
    k_thread_create(&event_task_handle, event_task_stack, CONFIG_EVENT_TASK_STACK_SIZE,
		(k_thread_entry_t)event_task, NULL, NULL, NULL,
		CONFIG_EVENT_TASK_PRIO, K_INHERIT_PERMS, K_NO_WAIT);

    return ESP_OK;
}
