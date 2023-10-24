#pragma once

#include "stdint.h"

struct sensor_sample {
    char * key;
    char * value;
};

struct sensor_samples {
    uint16_t length;
    struct sensor_sample *samples;
};

char * get_sample_value(struct sensor_samples* samples, char * key);

char * get_sample_value_number(struct sensor_samples* samples, char * key, char * format, float factor);

