

#include "bmv700.h"

#include "SoftwareSerial.h"

#include "esp_log.h"

#include "VeDirectFrameHandler.h"



// Change this to reflect how your sensor is connected.
const uint8_t txPin = 27;                               // tx of the MCU to rx of the sensor
const uint8_t rxPin = 35;   

char *ve_log_prefix;


SoftwareSerial VeSerial(rxPin, txPin);

VeDirectFrameHandler vedfh;

static void static_log_e(const char* module, const char* error) {
    ESP_LOGE(ve_log_prefix, "Error in BMV700, module: %s, error: %s", module, error);
}

int init_bmv700(char *ve_log_prefix) {
    
    
    ve_log_prefix = ve_log_prefix;

    // rx of the Arduino to tx of the sensor
    VeSerial.begin(9600, SWSERIAL_8N1);                         // Sensor transmits its data at 9600 bps.
    if (!VeSerial) { // If the object did not initialize, then its configuration is invalid 
        ESP_LOGE(ve_log_prefix, "Invalid SoftwareSerial pin configuration, check config");
    } 
    vedfh.setErrorHandler(&static_log_e);   

    ESP_LOGI(ve_log_prefix, "DS1603 sensor successfully initiated.");

    return 0;


}

void test_bmv700() {
    //
}