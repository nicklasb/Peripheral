#include "robusto_sensors.h"

#include <stdio.h>
#include <string.h>
#include "esp_log.h"

char * get_sample_value(struct sensor_samples* samples, char * key) {
    for (int sample_index = 0; sample_index < samples->length; sample_index++)
    {
        if (strcmp(key, samples->samples[sample_index].key) == 0) {
            return samples->samples[sample_index].value;
        }
    }
    return "";
}
/* Format a numeric value */
/**
 * @brief Format a numeric value to a number of decimals and multiplier by a factor.
 * A typical example would be 3 decimals with factor of 0.001 to make milliamperes into amperes.
 * 
 * @param samples 
 * @param key 
 * @param decimals Number of decimals
 * @param factor A factor whi
 * @return char* 
 */
char * get_sample_value_number(struct sensor_samples* samples, char * key, char * format, float factor) {
    if (samples->length > 0) {
        char * raw_value = get_sample_value(samples, key); 
        ESP_LOGI("robusto_sensors.h", "get_sample_value_number %s %s %s %f",  raw_value, key, format, factor);
        double number_value;
        if (sscanf(raw_value, "%lf", &number_value) > 0) {
            char * result;
            asprintf(&result, format, number_value * factor);
            return result;  
        }
    } 
    return "NO DATA";

    

}
