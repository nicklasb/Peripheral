#include "esp_log.h"
#include <string.h>

#include "sdp.h"
#include "sdp_task.h"

/**
 * @brief Takes a closer look on the incoming request queue item, does it need urgent attention?
 *
 * @param work_item
 */
int do_on_filter_request(struct work_queue_item *work_item)
{

    ESP_LOGI(log_prefix, "In filter request callback on the controller!");

    /* Allow the data to be imported */
    return 0;
}

/**
 * @brief Takes a closer look on the incoming data queue item, does it need urgent attention?
 *
 * @param work_item
 */
int do_on_filter_data(struct work_queue_item *work_item)
{
    ESP_LOGI(log_prefix, "In filter data callback on the controller!");

    /* Allow the data to be imported */
    return 0;
}

/**
 * @brief Handle priority messages
 *
 * @param work_item
 */
void do_on_priority(struct work_queue_item *work_item)
{

    ESP_LOGI(log_prefix, "In ble data callback on the controller!");
    if (strcmp((char *)(work_item->raw_data), (char *)"status") == 0)
    {
        ESP_LOGI(log_prefix, "Got asked for status!");
    }
}

void do_on_work(struct work_queue_item *queue_item)
{
    ESP_LOGI(log_prefix, "In do_on_work task on the peripheral, got a message:\n");
    for (int i = 0; i < queue_item->partcount; i++) {
        ESP_LOGI(log_prefix, "Message part %i: \"%s\"", i, queue_item->parts[i]);
    }
 
    /* Note that the worker task is run on Core 1 (APP) as upposed to all the other callbacks. */
    ESP_LOGI(log_prefix, "In do_on_work task, responding to the controller. Conversation id = %u", queue_item->conversation_id);
    char reply_data[17] = "response\0testdata";

    sdp_reply(*queue_item, DATA, &reply_data, sizeof(reply_data));

    /* Always call the cleanup crew when done */
    cleanup_queue_task(queue_item);
}