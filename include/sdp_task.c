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
#include <time.h>
#include <bmv700.h>

// Testing
esp_timer_handle_t periodic_timer;

/**
 * @brief Takes a closer look on the incoming request queue item, does it need urgent attention?
 *
 * @param work_item
 */
int do_on_filter_request(struct work_queue_item *work_item)
{

    ESP_LOGD(log_prefix, "In filter request callback on the controller!");

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
    ESP_LOGD(log_prefix, "In filter data callback on the controller!");

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
        ESP_LOGD(log_prefix, "Got asked for status!");
    }
}

int make_status_message(uint8_t **message)
{
    return add_to_message(message, "%s|%.1f seconds|%i bytes|%.2f C",
                          "running",
                          (float)esp_timer_get_time() / 1000000,
                          heap_caps_get_free_size(MALLOC_CAP_8BIT), 0.5);
}

float read_190_Straight_Ohm_level_meter()
{
    // Tested: EBM Liquid level sensor S3 300
    // Actual ohm range: 0.4 to 191.4
    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_11db);
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
        ESP_LOGI(log_prefix, "190K: raw_height = %f, Vout = %f, Rf= %f, Rt = %fadc_out = %i ", raw_height, Vout, Rf, Rt, adc_out);

        return raw_height;
    }
    else
    {
        ESP_LOGE(log_prefix, "In read_190k_level_meter() Parameter error, sending -9999.");
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
    ESP_LOGI(log_prefix, "calc_resistance:\n\
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
    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_11db);
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
            ESP_LOGI(log_prefix, "Lever sensor: raw_height (%f) is below blind spot (%f) ,setting to 0.", raw_height, Hb);
            cal_height = 0;
        }
        ESP_LOGI(log_prefix, "Lever sensor:\n    raw_height = %f = (Rf / Rm * Hs) + Hf = (%f / %f * %f) + %f\n"
                "    cal_height = (raw_heigh + lever_LUT_value) = (%f + %f) = %f",
                raw_height, Rf, Rm, Hs, Hf, raw_height, lever_LUT_value[j], cal_height);

        return cal_height;
    }
    else
    {
        ESP_LOGE(log_prefix, "Lever sensor: In read_190k_level_meter() Parameter error, sending -9999.");
        return -99;
    }
    // return (float)(100-(adc1_get_raw(ADC1_CHANNEL_6)-2060)/20.36);
}

float read_10k_thermistor()
{
    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_11db);
    float adcMax = 4095; // ADC resolution 12-bit
    float Vs = 3.3;      // supply voltage
    int beta = 3950;     // Beta from manufacturer
    // Rt = R1 * Vout/(Vs - Vout)
    int adc_out = adc1_get_raw(ADC1_CHANNEL_6);

    if (adc_out > -1)
    {
        adc_out = ADC_LUT[(int)adc_out];
        float Vout = adc_out * Vs / adcMax;
        ESP_LOGI(log_prefix, "2 Vout_Raw = %f", Vout);

        float Rt = 9290 * Vout / (Vs - Vout);
        float T = 1 / (1 / 298.15 + log(Rt / 10000) / beta);
        ESP_LOGI(log_prefix, "2 Vout = %f, Rt = %f, T(K) = %f T(C) = %f", Vout, Rt, T, T - 273.15);
        return T - 273.15;
    }
    else
    {
        ESP_LOGE(log_prefix, "In read_10k_thermistor() Parameter error, sending -9999.");
        return -99;
    }
}

int make_sensors_message(uint8_t **message)
{

    // Collect sensor data

    // return add_to_message(message, "%.2f", read_180_ohm_lever_level_meter());

    return add_to_message(message, "%.i|%.2f", read_ds1603l(), read_180_ohm_lever_level_meter());
    // return add_to_message(message, "%.2f|%.2f|%i", read_180_ohm_lever_level_meter(), read_10k_thermistor(),read_ds1603l());
    // return add_to_message(message, "%i|dfdfg", read_ds1603l());
}

void do_on_work(struct work_queue_item *work_item)
{

    ESP_LOGD(log_prefix, "In do_on_work task on the peripheral, got a message:\n");
    for (int i = 0; i < work_item->partcount; i++)
    {
        ESP_LOGD(log_prefix, "Message part %i: \"%s\"", i, work_item->parts[i]);
    }

    uint8_t *reply_data = NULL;
    int reply_length = 0;
    if (strcmp((char *)(work_item->parts[0]), "status") == 0)
    {
        ESP_LOGD(log_prefix, "Is it a status message");
        reply_length = make_status_message(&reply_data);
    }
    else if (strcmp((char *)(work_item->parts[0]), "sensors") == 0)
    {
        ESP_LOGD(log_prefix, "Is it a sensor request message");

        reply_length = make_sensors_message(&reply_data);
    }

    /* Note that the worker task is run on Core 1 (APP) as upposed to all the other callbacks. */
    ESP_LOGD(log_prefix, "In do_on_work task, responding to the controller. Conversation id = %u", work_item->conversation_id);
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
    init_bmv700(log_prefix);
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK)
    {
        ESP_LOGI(log_prefix, "eFuse Two Point: Supported\n");
    }
    else
    {
        ESP_LOGI(log_prefix, "eFuse Two Point: NOT supported\n");
    }

    // Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK)
    {
        ESP_LOGI(log_prefix, "eFuse Vref: Supported\n");
    }
    else
    {
        ESP_LOGI(log_prefix, "eFuse Vref: NOT supported\n");
    }
}
/**
 * @brief This is periodically waking up the controller, sends a request for sensor data
 *
 */
void periodic_sensor_test(void *arg)
{
    /* Note that the worker task is run on Core 1 (APP) as upposed to all the other callbacks. */
    //ESP_LOGI(log_prefix, "In prediodic_sensor_query test on the peripheral.");

    //char data[9] = "sensors\0";
    //ESP_LOGI(log_prefix, "Reading VE.direct...");
    test_bmv700();
    
    //ESP_LOGI(log_prefix, "VE done.");
    ESP_ERROR_CHECK(esp_timer_start_once(periodic_timer, 50000));
}

void init_sdp_task()
{
    sdp_init(do_on_work, do_on_priority, "Peripheral\0", false);
    init_sensors();

    const esp_timer_create_args_t periodic_timer_args = {
            .callback =  &periodic_sensor_test,
            .name = "periodic_query"
    };


    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_once(periodic_timer, 500000));
}
