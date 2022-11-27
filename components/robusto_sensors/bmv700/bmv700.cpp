

#include "bmv700.h"

// #include "SoftwareSerial.h"
#include <HardwareSerial.h>
#include "esp_log.h"


#include <VeDirectFrameHandler.h>
#include <sdkconfig.h>


#if CONFIG_ROBUSTO_BMV700_SIMULATION
#include "simulate/simulation_samples.h"
#endif


// Change this to reflect how your sensor is connected.
const uint8_t txPin = 17; // Green - HW 17tx of the MCU to rx of the sensor
const uint8_t rxPin = 16; // Yellow - HW 16

SemaphoreHandle_t xBMVSemaphore;

char *ve_log_prefix;
char viewbuf[1000];
int bufi;

// Use UART 1
HardwareSerial veSerial(1);

VeDirectFrameHandler vedfh;

static void static_log_e(const char *module, const char *error)
{
    ESP_LOGE(ve_log_prefix, "Error in BMV700, module: %s, error: %s", module, error);
}

void HexCallback(const char *buffer, int size, void *data)
{
    ESP_LOGE(ve_log_prefix, "In hex callback");
    char tmp[100];
    memcpy(tmp, buffer, size * sizeof(char));
    tmp[size] = 0;
    ESP_LOGE(ve_log_prefix, "BMV700 hex frame:\n%s", tmp);
}

int prox(int val)
{
    ESP_LOGI(ve_log_prefix, "V %i", val);
    return val;
}

void ReadVEData()
{
    ESP_LOGI(ve_log_prefix, "In read VEData()");
    bufi = 0;
    static unsigned long wait_millis = millis();

    // if (!veSerial.available()) {
    //     ESP_LOGI(ve_log_prefix, "VE.Direct - Waiting.");
    // }
    // veSerial.flush();
    while (!veSerial.available())
    {

        if (millis() - wait_millis > 1000)
        {
            ESP_LOGI(ve_log_prefix, "VE.Direct - No data after 1000 ms.");
            return;
        }
        // veSerial.perform_work();
    }
    ESP_LOGI(ve_log_prefix, "VE.Direct - %i bytes of data available.", veSerial.available());

    while (veSerial.available())
    {

        viewbuf[bufi] = veSerial.read();

        bufi++;
    }
}

void parse()
{

    if (bufi > 0)
    {

        viewbuf[bufi] = 0;

        for (int i = 0; i < bufi; i++)
        {
            vedfh.rxData(viewbuf[i]);
            // veSerial.perform_work();
        }
    }
}

struct sensor_samples *bmv700_read()
{
    
#if CONFIG_ROBUSTO_BMV700_SIMULATION
    ESP_LOGI(ve_log_prefix, "Simulation data:");

    return get_simulation_samples();
    
#else
    struct sensor_samples *samples = (sensor_samples *)malloc(sizeof(struct sensor_samples));
    ReadVEData();

    parse();
    if (vedfh.veEnd > 0)
    {
        ESP_LOGI(ve_log_prefix, "Current data:");
        
        samples->length = vedfh.veEnd;
        samples->samples = (sensor_sample *)malloc(vedfh.veEnd * sizeof(sensor_sample));
        for (int ve_index = 0; ve_index < vedfh.veEnd; ve_index++)
        {
            samples->samples[ve_index].key = vedfh.veName[ve_index];
            samples->samples[ve_index].value = vedfh.veValue[ve_index];
            
            ESP_LOGI(ve_log_prefix, "Name: %s, Value: %s", vedfh.veName[ve_index], vedfh.veValue[ve_index]);
        }
        return samples;        
        // char *loc_format = heap_caps_malloc(format_len + 1, MALLOC_CAP_8BIT);
        // strcpy(loc_format, format);
        // TODO: Create a simulator based on real data. Make it #if-conditional here.
    } else {
        ESP_LOGI(ve_log_prefix, "bmv700_read()- No data .");
        samples->length = 0;
        samples->samples = NULL;
        return samples;
    }
    
#endif
}

int bmv700_init(char *log_prefix)
{

    xBMVSemaphore = xSemaphoreCreateMutex();
    ve_log_prefix = log_prefix;

    // rx of the Arduino to tx of the sensor
    veSerial.begin(19200, SERIAL_8N1, rxPin, txPin, false, 1000); // VE.direct is at 19200 bps.

    if (!veSerial)
    { // If the object did not initialize, then its configuration is invalid
        ESP_LOGE(ve_log_prefix, "Invalid SoftwareSerial pin configuration, check config");
    }
    veSerial.flush();
    vedfh.setErrorHandler(&static_log_e);
    // hex protocol callback
    vedfh.addHexCallback(&HexCallback, (void *)42);
    ESP_LOGI(ve_log_prefix, "VE.Direct interface successfully initiated. rxPin = %i, txPin = %i", rxPin, txPin);

    return 0;
}