#include <sdkconfig.h>

#if CONFIG_ROBUSTO_BMV700_SIMULATION

#ifndef _TEST_DATA_H_
#define _TEST_DATA_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <robusto_sensors.h>

struct sensor_samples *get_simulation_samples();


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
#endif