
#include "ble_task.h"
#include "esp_log.h"
#include "host/ble_gatt.h"
#include "host/ble_hs_mbuf.h"
#include "host/ble_hs_id.h"
#include "ble_service.h"
#include "host/ble_hs_adv.h"
#include "peer.c"

/*
This is the running task of the peripheral, it currently connects to the server (actually peripheral) and sends data.
TODO: Instead this will be rewritten competely:
It will instead contact all peripherals and ask for data, in the following situations:
1. After periodically being woken from a timer signal
2. Because a GPIO went high (which is likely to be because of an alarm)
3. Because of a instruction via SMS (or other way in).
*/

void ble_client_my_task(void *pvParameters)
{

    /* Use a special semaphore for BLE.
   TODO: Consider if there instead should be one for Core 0
    (that might not be possible if not a param here; should a controller init exist?)
   */
    xBLESemaphore = xSemaphoreCreateMutex();

    ESP_LOGI(task_tag, "My Task peripheral: In my task");

    ESP_LOGI(task_tag, "My Task peripheral: BLE peripheral task started\n");

    int ret;
    char myarray[31] = "Peripheral sent this message!\0";   
 
        // Create an MBUF
        //struct os_mbuf *om;

    //om = ble_hs_mbuf_from_flat(myarray, sizeof(myarray));

    struct peer *curr_peer;
    const struct peer_chr *chr;
   
    for (;;)
    {


        ESP_LOGI(task_tag, "My Task controller: Loop peers:");

        SLIST_FOREACH(curr_peer, &peers, next)
        {

            chr = peer_chr_find_uuid(curr_peer,
                                     BLE_UUID16_DECLARE(GATT_SPP_SVC_UUID),
                                     BLE_UUID16_DECLARE(GATT_SPP_CHR_UUID));

            ESP_LOGI(task_tag, "Peer: %i,  Address: %02X%02X%02X%02X%02X%02X", curr_peer->conn_handle,
                     curr_peer->desc.peer_ota_addr.val[0], curr_peer->desc.peer_ota_addr.val[1],
                     curr_peer->desc.peer_ota_addr.val[2], curr_peer->desc.peer_ota_addr.val[3],
                     curr_peer->desc.peer_ota_addr.val[4], curr_peer->desc.peer_ota_addr.val[5]);

            

            if (pdTRUE == xSemaphoreTake(xBLESemaphore, portMAX_DELAY))
            {

                ret = ble_gattc_write_flat(curr_peer->conn_handle, ble_spp_svc_gatt_read_val_handle, &myarray, sizeof(myarray), NULL, NULL);
                if (ret == 0)
                {
                    ESP_LOGI(task_tag, "My Task peripheral: Wrote flat data characteristic!!");
                }
                else
                {
                    ESP_LOGI(task_tag, "My Task peripheral: Error writing flat data, error: %i", ret);
                }
                xSemaphoreGive(xBLESemaphore);
            }
            else
            {
                ESP_LOGI(task_tag, "My Task peripheral: Couldn't get semaphore");
            }
        }

        ESP_LOGI(task_tag, "....iteration ended.");
        vTaskDelay(2000);
    }
    

    vTaskDelete(NULL);
}