#include "sensor_task.h"
#include <robusto_init.h>
#include <robusto_logging.h>
#include <robusto_message.h>
#include <robusto_queue.h>
#include <robusto_umts.h>
#include <robusto_incoming.h>
#include "../secret/local_settings.h"


#include <string.h>

#include <driver/adc.h>
#include <math.h>
#include <time.h>

#include <robusto_sensors.h>
#include <bmv700.h>
#include <dht22.h>
#include <sensor_ds1603l.h>

#include <esp_timer.h>
#include <esp_heap_caps.h>

#include "LUT.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#include <robusto_conductor.h>
#include <robusto_sleep.h>

char * tasks_log_prefix = "Peripheral-task";
robusto_peer_t * central_peer = NULL; 

// Testing
esp_timer_handle_t periodic_timer;

/**
 * @brief Takes a closer look on the incoming request queue item, does it need urgent attention?
 *
 * @param message
 */
int do_on_filter_request(robusto_message_t *message)
{

    ROB_LOGD(tasks_log_prefix, "In filter request callback on the peripheral!");

    /* Allow the data to be imported */
    return 0;
}

/**
 * @brief Takes a closer look on the incoming data queue item, does it need urgent attention?
 *
 * @param message
 */
int do_on_filter_data(robusto_message_t *message)
{
    ROB_LOGD(tasks_log_prefix, "In filter data callback on the peripheral!");

    /* Allow the data to be imported */
    return 0;
}

/**
 * @brief Handle priority messages
 *
 * @param message
 */
void do_on_priority(robusto_message_t *message)
{

    ROB_LOGI(tasks_log_prefix, "In ble data callback on the peripheral!");
    if (strcmp((char *)(message->raw_data), (char *)"status") == 0)
    {
        ROB_LOGD(tasks_log_prefix, "Got asked for status!");
    }
    

}

int make_status_message(uint8_t **message)
{
    return build_strings_data(message, "%s|%.1f seconds|%i bytes|%.2f C",
                          "running",
                          (float)esp_timer_get_time() / 1000000,
                          heap_caps_get_free_size(MALLOC_CAP_8BIT), 0.5);
}

float read_190_Straight_Ohm_level_meter()
{
    // Tested: EBM Liquid level sensor S3 300
    // Actual ohm range: 0.4 to 191.4
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
    float adcMax = 4095; // ADC resolution 12-bit, TODO: Use in formula for clarity
    float Vs = 5;        // supply voltage
    float Vfs = 3.3;     // Full-scale voltage (given 3.3 volts above)
    float Rp = 66;       // Pre-Rt resistors to push voltage into a better region
    float Rk = 242;      // Known resistor
                         // Sensor details
    float Ra = 2;        // Manual adjustment, other resistance after sensor (or negative if before)
    float Rm = 195.4;    // Max R from sensor as measured by the ADC
    float Hf = 15;       // mm floater sinks in liquid
    float Hs = 250;      // mm max movement of sensor
    float Hb = 7;        // mm blindness at bottom that the floater can't sink

    int adc_out = adc1_get_raw(ADC1_CHANNEL_6);
    if (adc_out > -1)
    {
        adc_out = ADC_LUT[(int)adc_out];
        float Vout = adc_out * Vfs / adcMax;

        float Rt = Rk * Vout / (Vs - Vout);
        float Rf = Rt - Rp - Ra;
        float raw_height = (Rf / Rm * Hs) + Hf;
        if (raw_height < Hb)
        {
            raw_height = 0;
        }
        ROB_LOGI(tasks_log_prefix, "190K: raw_height = %f, Vout = %f, Rf= %f, Rt = %fadc_out = %i ", raw_height, Vout, Rf, Rt, adc_out);

        return raw_height;
    }
    else
    {
        ROB_LOGE(tasks_log_prefix, "In read_190k_level_meter() Parameter error, sending -9999.");
        return -99;
    }
    // return (float)(100-(adc1_get_raw(ADC1_CHANNEL_6)-2060)/20.36);
}

/**
 * @brief Calculates unknown resistance in a voltage divider setup
 * Uses a lookup table for calibration.
 * TODO: Break LUT out into a parameter
 *
 * @param adc_in The raw value of the ADC
 * @param adc_max The max value of the ADC
 * @param Vs Supply voltage
 * @param Vfs Full-scale voltage
 * @param Rk Known resistance before the sensor (typical divider setup)
 * @param Rp Known resistance *after* the sensor to put voltage in a better reqion for ADC
 * @return float
 */
float calc_resistance(int adc_in, int adc_max, float Vs, float Vfs, float Rk, float Rp)
{
    float adc_cal = ADC_LUT[(int)adc_in];
    float Vout = adc_cal * Vfs / adc_max;
    // Calculate total unknown resistance
    float Rt = Rk * Vout / (Vs - Vout);
    // Adjust for extra resistor (often used to push voltages into a better measurement region)
    ROB_LOGI(tasks_log_prefix, "calc_resistance:\n\
    adc_in = %i => adc_cal = %f\n\
    Vout = %f = adc_cal * Vfs / adc_max = %f * %f / %i\n\
    Rt = %f = Rk * Vout / (Vs - Vout) = %f * %f / (%f - %f)\n\
    Rf = %f = Rt - Rp = %f - %f",
    adc_in, adc_cal,
    Vout, adc_cal, Vfs, adc_max, 
    Rt, Rk, Vout, Vs, Vout,
    Rt - Rp, Rt, Rp);
    return Rt - Rp;

}

float read_180_ohm_lever_level_meter()
{
    // Tested: Lever sensor (from 1980)
    // Actual ohm range: 0.4 to 191.4
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
    // ESP32 data
    float adc_max = 4095; // ADC steps with 12-bit resolution
    float Vs = 3.3;       // supply voltage
    float Vfs = 3.3;      // Full-scale voltage (given 3.3 volts above)
    // Divider setup
    float Rp = 62;  // Known resistors after the sensors to push voltage down into a better region for the ADC
    float Rk = 242; // Known resistors befor the sensor
    
    // Sensor details
    float Rm = 185; // Max R from sensor (manually measured 185)as measured by the ADC
    float Hf = 15;  // mm floater sinks in liquid
    float Hs = 250; // mm max movement of sensor
    float Hb = 0;   // mm blindness at bottom that the floater can't sink

    // A little bit of DSP settings
    int multisampling = 40;

    // Multisample and average to minimize the significant noise
    float total = 0;
    int raw = 0;
    for (int i = 0; i < multisampling; i++) {
        raw = adc1_get_raw(ADC1_CHANNEL_6);
        total+= ADC_LUT[(int)raw];
    }

   float adc_in = (double)total/multisampling;

    if (adc_in > -1)
    {
        float Rf = calc_resistance(adc_in, adc_max, Vs, Vfs, Rk, Rp);
        // The height is a function of max/min resistance, max movement, and the sinking of the float
        float raw_height = (Rf / Rm * Hs) + Hf;

        // Find calibration values
        int j = 0;
        while ((raw_height > lever_LUT_key[j]) && (j < 25)) {
            j++;
        }
        // Calibrate
        float cal_height = raw_height + lever_LUT_value[j];

        // Is it below the blind spot?
        if (cal_height < Hb)
        {
            ROB_LOGI(tasks_log_prefix, "Lever sensor: raw_height (%f) is below blind spot (%f) ,setting to 0.", raw_height, Hb);
            cal_height = 0;
        }
        ROB_LOGI(tasks_log_prefix, "Lever sensor:\n    raw_height = %f = (Rf / Rm * Hs) + Hf = (%f / %f * %f) + %f\n"
                "    cal_height = (raw_heigh + lever_LUT_value) = (%f + %f) = %f",
                raw_height, Rf, Rm, Hs, Hf, raw_height, lever_LUT_value[j], cal_height);

        return cal_height;
    }
    else
    {
        ROB_LOGE(tasks_log_prefix, "Lever sensor: In read_190k_level_meter() Parameter error, sending -9999.");
        return -99;
    }
    // return (float)(100-(adc1_get_raw(ADC1_CHANNEL_6)-2060)/20.36);
}

float read_10k_thermistor()
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
    float adcMax = 4095; // ADC resolution 12-bit
    float Vs = 3.3;      // supply voltage
    int beta = 3950;     // Beta from manufacturer
    // Rt = R1 * Vout/(Vs - Vout)
    int adc_out = adc1_get_raw(ADC1_CHANNEL_6);

    if (adc_out > -1)
    {
        adc_out = ADC_LUT[(int)adc_out];
        float Vout = adc_out * Vs / adcMax;
        ROB_LOGI(tasks_log_prefix, "2 Vout_Raw = %f", Vout);

        float Rt = 9290 * Vout / (Vs - Vout);
        float T = 1 / (1 / 298.15 + log(Rt / 10000) / beta);
        ROB_LOGI(tasks_log_prefix, "2 Vout = %f, Rt = %f, T(K) = %f T(C) = %f", Vout, Rt, T, T - 273.15);
        return T - 273.15;
    }
    else
    {
        ROB_LOGE(tasks_log_prefix, "In read_10k_thermistor() Parameter error, sending -9999.");
        return -99;
    }
}

int make_sensors_message(uint8_t **message)
{

    // Collect sensor data

    // return add_to_message(message, "%.2f", read_180_ohm_lever_level_meter());
    return 0;
    //return add_to_message(message, "%.i|%.2f", ds1603l_read(), read_180_ohm_lever_level_meter());
    // return add_to_message(message, "%.2f|%.2f|%i", read_180_ohm_lever_level_meter(), read_10k_thermistor(),ds1603l_read());
    // return add_to_message(message, "%i|dfdfg", ds1603l_read());
}
#if 0
void do_on_work(robust_message_t *message)
{

    ROB_LOGI(tasks_log_prefix, "In do_on_work task on the peripheral, got a message:\n");
    for (int i = 0; i < message->stringcount; i++)
    {
        ROB_LOGI(tasks_log_prefix, "Message part %i: \"%s\"", i, message->strings[i]);
    }

    uint8_t *reply_data = NULL;
    int reply_length = 0;
    if (strcmp((char *)(message->strings[0]), "status") == 0)
    {
        ROB_LOGI(tasks_log_prefix, "Is it a status message");
        reply_length = make_status_message(&reply_data);
    }
    else if (strcmp((char *)(message->strings[0]), "sensors") == 0)
    {
        ROB_LOGI(tasks_log_prefix, "Is it a sensor request message");

        reply_length = make_sensors_message(&reply_data);
    }

    /* Note that the worker task is run on Core 1 (APP) as upposed to all the other callbacks. */
    ROB_LOGD(tasks_log_prefix, "In do_on_work task, responding to the controller. Conversation id = %u", message->conversation_id);
    if (reply_data == NULL)
    {
        ROB_LOGI(tasks_log_prefix, "Reply empty for some reason");
    }
    else
    {
        sdp_reply(*message, DATA, reply_data, reply_length);
        free(reply_data);
    }


}
#endif

void init_sensors()
{
    //init_ds1603l(tasks_log_prefix);

    bmv700_init(tasks_log_prefix);
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK)
    {
        ROB_LOGI(tasks_log_prefix, "eFuse Two Point: Supported\n");
    }
    else
    {
        ROB_LOGI(tasks_log_prefix, "eFuse Two Point: NOT supported\n");
    }

    // Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK)
    {
        ROB_LOGI(tasks_log_prefix, "eFuse Vref: Supported\n");
    }
    else
    {
        ROB_LOGI(tasks_log_prefix, "eFuse Vref: NOT supported\n");
    }
    dht22_init(tasks_log_prefix);
}
/**
 * @brief This is periodically waking up the controller, sends a request for sensor data
 *
 */
void periodic_sensor_test(void *arg)
{
    /* Note that the worker task is run on Core 1 (APP) as opposed to all the other callbacks. */
    //ROB_LOGI(tasks_log_prefix, "In periodic_sensor_query test on the peripheral.");

    struct sensor_samples *samples = bmv700_read();
    struct dht22_result dht22_res = dht22_read();
    
    char * humidity;
    asprintf(&humidity, "%.2f", dht22_res.humidity);

    char * temperature;
    asprintf(&temperature, "%.2f", dht22_res.temperature);

    char * curr_time;
    asprintf(&curr_time, "%.2f", (double)esp_timer_get_time()/(double)(1000000));

    char * since_start;
    asprintf(&since_start, "%.2f", 1.0); //(double)robusto_conductor_server_get_time_since_start()/(double)(1000000));

    int free_mem = heap_caps_get_free_size(MALLOC_CAP_8BIT);

    char * total_awake_time;
    asprintf(&total_awake_time, "%.2f", (double)robusto_get_total_time_awake()/(double)(1000000));
    
    uint8_t *message = NULL;

    ROB_LOGI(tasks_log_prefix, "Making message.");

    int data_length = build_strings_data(&message, "report|%s|%s|%s|%s|%i|%s|%s|%s|%s|%s",
                          humidity, temperature, curr_time,  since_start, free_mem, 
                          total_awake_time, get_sample_value_number(samples,"V", "%.3f", 0.001), 
                          get_sample_value_number(samples,"SOC", "%.1f", 0.1), 
                          get_sample_value_number(samples,"I", "%.3f", 0.001), 
                          get_sample_value_number(samples,"VM", "%.3f", 0.001)); 
    char * data = robusto_malloc(data_length);
    memcpy(data, message, data_length);
    for (int i = 0; i < data_length - 1; i++) {
        if (data[i] == NULL) {
            data[i] = "|";
        }

    }
    ROB_LOGI(tasks_log_prefix, "Message : %s", data);
    // rob_log_bit_mesh(ROB_LOG_INFO, tasks_log_prefix, data, data_length);
    send_message_strings(central_peer, ROBUSTO_MQTT_SERVICE_ID, 0, message, data_length, NULL);
}

void init_sensor_task()
{

    init_robusto();
   //robusto_register_handler(&do_on_work);
    
    central_peer = robusto_add_init_new_peer(local_hosts[2].hostname, local_hosts[2].base_mac_address, robusto_mt_espnow);

    init_sensors();
    /* Allow for some communication between the peripheral and the controller */
    while (central_peer->state < PEER_KNOWN_INSECURE) {
        r_delay(1000);
        // TODO: Don't wait forever
    }
    
    periodic_sensor_test(NULL);
/*
    const esp_timer_create_args_t periodic_timer_args = {
            .callback =  &periodic_sensor_test,
            .name = "periodic_query"
    };

    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_once(periodic_timer, 1000));
*/
    robusto_conductor_client_give_control(central_peer); 
}
