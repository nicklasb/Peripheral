#include "robusto_sensors.h"

#include <string.h>

char * get_sample_value(struct sensor_samples* samples, char * key) {
    for (int sample_index = 0; sample_index < samples->length; sample_index++)
    {
        if (strcmp(key, samples->samples[sample_index].key) == 0) {
            return samples->samples[sample_index].value;
        }
    }
    return "";
}
