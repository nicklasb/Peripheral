#include "dht22.h"

#if CONFIG_ROBUSTO_LOAD_DHT22 
#include "hal/gpio_types.h"
#include <robusto_system.h>
#include "DHT.h"
#include "esp_log.h"

char * log_prefix;

struct dht22_result dht22_read() {

    struct dht22_result res;

    
        ESP_LOGI(log_prefix, "=== Reading DHT ===\n");
        int ret = readDHT();
        if (ret != DHT_OK) {
            errorHandler(ret);
            res.humidity = ret;
            res.temperature = -99999;
        }
        else {
            res.humidity = getHumidity();
            res.temperature = getTemperature();
        }

        ESP_LOGI(log_prefix, "Hum: %.1f Tmp: %.1f\n", res.humidity, res.temperature);
    return res;
}


void dht22_init(char * _log_prefix) {
    log_prefix = _log_prefix;

    robusto_gpio_set_direction(GPIO_NUM_5, false);
    robusto_gpio_set_pullup(GPIO_NUM_5, true);
    setDHTgpio(GPIO_NUM_5);
    /* Make a read just to wake up the sensor */
    readDHT();
    ESP_LOGI(log_prefix, "DHT22 sensor initiated");

}

#endif