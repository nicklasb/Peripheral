

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * BMV700 series library.
 * 
 * Copyright Nicklas Börjesson, 2022
 */
#ifndef __BMV700__
#define __BMV700__


//#include <Stream.h>                                             // Generic Stream library, upon which a.o. Serial and SoftwareSerial are built.
#include "../sensors_def.h"

int bmv700_init(char *log_prefix);

sensor_sample* bmv700_read();

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif