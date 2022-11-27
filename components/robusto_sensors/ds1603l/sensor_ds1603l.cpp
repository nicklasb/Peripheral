

#include "sensor_ds1603l.h"

#if CONFIG_ROBUSTO_LOAD_DS1603L

#include "DS1603L.h"

#include "SoftwareSerial.h"

#include "esp_log.h"

// Change this to reflect how your sensor is connected.
const uint8_t txPin = 18;                               // tx of the MCU to rx of the sensor
const uint8_t rxPin = 5;   

char *ds_log_prefix;


SoftwareSerial sensorSerial(rxPin, txPin);

DS1603L sensor(sensorSerial);

//// If your sensor is connected to Serial, Serial1, Serial2, AltSoftSerial, etc. pass that object to the sensor constructor.


int ds1603l_read() {
    ESP_LOGI(ds_log_prefix, "Reading DS1603 sensor..");
    uint16_t reading = sensor.readSensor();       // Call this as often or as little as you want - the sensor transmits every 1-2 seconds.
    ESP_LOGI(ds_log_prefix, "Read DS1603 sensor..");
    uint8_t sensorStatus = sensor.getStatus();           // Check the status of the sensor (not detected; checksum failed; reading success).
    ESP_LOGI(ds_log_prefix, "Read DS1603 sensor status %i..", sensorStatus);
    switch (sensorStatus) {                           // For possible values see DS1603L.h
    case DS1603L_NO_SENSOR_DETECTED:                // No sensor detected: no valid transmission received for >10 seconds.
        ESP_LOGW(ds_log_prefix, "No sensor detected (yet). If no sensor after 1 second, check whether your connections are good.");
        return -2;           

    case DS1603L_READING_CHECKSUM_FAIL:             // Checksum of the latest transmission failed.
        ESP_LOGE(ds_log_prefix, "Data received; checksum failed. Latest level reading: %u", reading);
        return -1;   

    case DS1603L_READING_SUCCESS:                   // Latest reading was valid and received successfully.
        ESP_LOGI(ds_log_prefix, "Water level : %u", reading);
        return reading;
    }
    return -1;
}


int ds1603l_init(char *ds_log_prefix) {
    
    
    ds_log_prefix = ds_log_prefix;
    // rx of the Arduino to tx of the sensor
    sensorSerial.begin(9600, SWSERIAL_8N1);                         // Sensor transmits its data at 9600 bps.
    if (!sensorSerial) { // If the object did not initialize, then its configuration is invalid 
        ESP_LOGE(ds_log_prefix, "Invalid SoftwareSerial pin configuration, check config");
    } 
    sensor.begin();   

    ESP_LOGI(ds_log_prefix, "DS1603 sensor successfully initiated.");

    return 0;


}

#endif