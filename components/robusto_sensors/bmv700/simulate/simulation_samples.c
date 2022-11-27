#include "simulation_samples.h"

#if CONFIG_ROBUSTO_BMV700_SIMULATION

#include "robusto_sensors.h"

#include "stdint.h"
#include "stdlib.h"

#include "esp_timer.h"


struct sensor_sample test_data1[30] = {{"H1", "-281237"},
                                {"H2", "0"},
                                {"H3", "-81752"},
                                {"H4", "19"},
                                {"H5", "0"},
                                {"H6", "-9177906"},
                                {"H7", "10"},
                                {"H8", "14968"},
                                {"H9", "0"},
                                {"H10", "47"},
                                {"H11", "0"},
                                {"H12", "0"},
                                {"H15", "3"},
                                {"H16", "14870"},
                                {"H17", "11692"},
                                {"H18", "16413"},
                                {"PID", "0x204"},
                                {"V", "13786"},
                                {"VM", "12656"},
                                {"DM", "836"},
                                {"I", "100"},
                                {"P", "1"},
                                {"CE", "0"},
                                {"SOC", "1000"},
                                {"TTG", "-1"},
                                {"Alarm", "ON"},
                                {"Relay", "OFF"},
                                {"AR", "128"},
                                {"BMV", "702"},
                                {"FW", "308"}};
struct sensor_sample test_data2[30] = {{"H1", "-281237"},
                                {"H2", "0"},
                                {"H3", "-81752"},
                                {"H4", "19"},
                                {"H5", "0"},
                                {"H6", "-9177906"},
                                {"H7", "10"},
                                {"H8", "14968"},
                                {"H9", "0"},
                                {"H10", "47"},
                                {"H11", "0"},
                                {"H12", "0"},
                                {"H15", "3"},
                                {"H16", "14870"},
                                {"H17", "11692"},
                                {"H18", "16413"},
                                {"PID", "0x204"},
                                {"V", "13787"},
                                {"VM", "12656"},
                                {"DM", "836"},
                                {"I", "0"},
                                {"P", "0"},
                                {"CE", "0"},
                                {"SOC", "1000"},
                                {"TTG", "-1"},
                                {"Alarm", "ON"},
                                {"Relay", "OFF"},
                                {"AR", "128"},
                                {"BMV", "702"},
                                {"FW", "308"}};
struct sensor_sample test_data3[30] = {{"H1", "-281237"},
                                {"H2", "0"},
                                {"H3", "-81752"},
                                {"H4", "19"},
                                {"H5", "0"},
                                {"H6", "-9177906"},
                                {"H7", "10"},
                                {"H8", "14968"},
                                {"H9", "0"},
                                {"H10", "47"},
                                {"H11", "0"},
                                {"H12", "0"},
                                {"H15", "3"},
                                {"H16", "14870"},
                                {"H17", "11692"},
                                {"H18", "16413"},
                                {"PID", "0x204"},
                                {"V", "13787"},
                                {"VM", "12656"},
                                {"DM", "836"},
                                {"I", "0"},
                                {"P", "0"},
                                {"CE", "0"},
                                {"SOC", "1000"},
                                {"TTG", "-1"},
                                {"Alarm", "ON"},
                                {"Relay", "OFF"},
                                {"AR", "128"},
                                {"BMV", "702"},
                                {"FW", "308"}};

/**
 * @brief Get simulation data randomly from either of the above datasets
 * 
 * @return struct sensor_samples* 
 */
struct sensor_samples * get_simulation_samples() {

    struct sensor_samples *samples = (struct sensor_samples *)malloc(sizeof(struct sensor_samples));

    samples->length = 30;
    srand((unsigned int)esp_timer_get_time());
    uint8_t random_int = rand() % 3;
    if (random_int == 0) {
        samples->samples = test_data1;
    } else
    if (random_int == 1) {
        samples->samples = test_data2;
    } else
    if (random_int == 2) {
        samples->samples = test_data3;
    } 
    
    return samples;    

}



#endif