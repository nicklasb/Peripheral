

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

int init_bmv700(char *log_prefix) {
    
    
    ve_log_prefix = log_prefix;

    // rx of the Arduino to tx of the sensor
    VeSerial.begin(19200, SWSERIAL_8N1);                         // Sensor transmits its data at 9600 bps.
    if (!VeSerial) { // If the object did not initialize, then its configuration is invalid 
        ESP_LOGE(ve_log_prefix, "Invalid SoftwareSerial pin configuration, check config");
    } 
    vedfh.setErrorHandler(&static_log_e);   

    ESP_LOGI(ve_log_prefix, "VE.Direct interface successfully initiated.");

    return 0;


}

 int prox(int val) {
    ESP_LOGI(ve_log_prefix, "V %i",val);
    return val;
}

void test_bmv700() {
    char viewbuf[1000];
    int tmp;
    int i = 0;
    while (VeSerial.available()) {
        tmp = VeSerial.read();
        viewbuf[i] = tmp;
        vedfh.rxData(tmp);
        i++;
    }
    if (i > 0) {
        viewbuf[i] = 0;
        
        ESP_LOGI(ve_log_prefix, "TmpBuf: %s\nVeEnd: %i", viewbuf, vedfh.veEnd);
        for (int ve_index = 0; i < vedfh.veEnd; i++) {
            ESP_LOGI(ve_log_prefix, "Value: %s, Name: %s", vedfh.veName[ve_index], vedfh.veValue[ve_index]);
        }       
    }

   

}