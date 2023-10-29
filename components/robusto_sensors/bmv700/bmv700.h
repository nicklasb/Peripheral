


/*
 * BMV700 series library.
 * 
 * Copyright Nicklas Börjesson, 2022
 */
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <robconfig.h>
//#include <Stream.h>                                             // Generic Stream library, upon which a.o. Serial and SoftwareSerial are built.
#include "../robusto_sensors.h"

int bmv700_init(char *log_prefix);

struct sensor_samples* bmv700_read();

#ifdef __cplusplus
} /* extern "C" */
#endif