#include "sdp_task.h"
#include "sdp.h"
#include <sdp_helpers.h>
#include <sensor_ds1603l.h>

#include <string.h>
#include <esp_log.h>
#include <driver/adc.h>
#include "esp_adc_cal.h"
#include <math.h>
#include "LUT.h"

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

    int make_status_message(uint8_t **message)
    {
        return add_to_message(message, "%s|%.1f seconds|%i bytes|%.2f C",
                              "running",
                              (float)esp_timer_get_time() / 1000000,
                              heap_caps_get_free_size(MALLOC_CAP_8BIT), 0.5);
    }

    float read_190k_level_meter () {
        adc1_config_width(ADC_WIDTH_12Bit);
        adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_11db);
        float adcMax = 4095; // ADC resolution 12-bit, TODO: Use in formula for clarity
        // This could probably improve (is only tested without LUT and with a 180 ohm meter.)
        return (float)(100-(adc1_get_raw(ADC1_CHANNEL_6)-2060)/20.36);

    } 
    float read_10k_thermistor() {
        adc1_config_width(ADC_WIDTH_12Bit);
        adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_11db);
        float adcMax = 4095; // ADC resolution 12-bit
        float Vs = 3.3;    // supply voltage
        int beta = 3950;   // Beta from manufacturer
        //Rt = R1 * Vout/(Vs - Vout)
        int adc_out = adc1_get_raw(ADC1_CHANNEL_6);
        
        if (adc_out > -1) {
            adc_out = ADC_LUT[(int)adc_out];
            float Vout = adc_out * Vs/adcMax;
            ESP_LOGI(log_prefix, "2 Vout_Raw = %f", Vout);
        
            float Rt = 9290 * Vout/(Vs - Vout);
            float T =  1 / (1/298.15 + log(Rt/10000)/beta);
            ESP_LOGI(log_prefix, "2 Vout = %f, Rt = %f, T(K) = %f T(C) = %f", Vout, Rt, T, T - 273.15);
            return T- 273.15;
        } else {
            ESP_LOGE(log_prefix, "In read_10k_thermistor() Parameter error, sending -9999.");
            return -99;
        }

    } 

    int make_sensors_message(uint8_t **message)
    {

        // Collect sensor data


        return add_to_message(message, "%.2f|%.2f|%i", read_10k_thermistor(),read_190k_level_meter(),read_ds1603l());
        //return add_to_message(message, "%i|%i dfdfg",res, read_ds1603l());
    
    }

    void do_on_work(struct work_queue_item *work_item)
    {
        ESP_LOGI(log_prefix, "In do_on_work task on the peripheral, got a message:\n");
        for (int i = 0; i < work_item->partcount; i++)
        {
            ESP_LOGI(log_prefix, "Message part %i: \"%s\"", i, work_item->parts[i]);
        }

        uint8_t *reply_data = NULL;
        int reply_length = 0;
        if (strcmp((char *)(work_item->parts[0]), "status") == 0)
        {
            ESP_LOGI(log_prefix, "Is it a status message");
            reply_length = make_status_message(&reply_data);
        }
        else if (strcmp((char *)(work_item->parts[0]), "sensors") == 0)
        {
            ESP_LOGI(log_prefix, "Is it a sensor request message");
            
            reply_length = make_sensors_message(&reply_data);

        }

        /* Note that the worker task is run on Core 1 (APP) as upposed to all the other callbacks. */
        ESP_LOGI(log_prefix, "In do_on_work task, responding to the controller. Conversation id = %u", work_item->conversation_id);
        if (reply_data == NULL)
        {
            ESP_LOGI(log_prefix, "Reply empty for some reason");
        }
        else
        {
            sdp_reply(*work_item, DATA, reply_data, reply_length);
            free(reply_data);
        }

        /* Always call the cleanup crew when done */
        cleanup_queue_task(work_item);
    }

    void init_sensors()
    {
        init_ds1603l(log_prefix);
        if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
            ESP_LOGI(log_prefix, "eFuse Two Point: Supported\n");
        } else {
            ESP_LOGI(log_prefix, "eFuse Two Point: NOT supported\n");
        }

        //Check Vref is burned into eFuse
        if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
            ESP_LOGI(log_prefix, "eFuse Vref: Supported\n");
        } else {
            ESP_LOGI(log_prefix, "eFuse Vref: NOT supported\n");
        }

    }

void init_sdp_task() {
    sdp_init(do_on_work, do_on_priority, "Peripheral\0", false);
    init_sensors();
}




