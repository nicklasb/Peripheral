


#include <DS1603L.h>



// Change this to reflect how your sensor is connected.
#include <SoftwareSerial.h>

#include "sensor_ds1603l.h"


#include "esp_log.h"

const byte txPin = 18;                               // tx of the Arduino to rx of the sensor
const byte rxPin = 5;   

char *log_prefix;


SoftwareSerial sensorSerial;

//// If your sensor is connected to Serial, Serial1, Serial2, AltSoftSerial, etc. pass that object to the sensor constructor.


int init_ds1603l(char *log_prefix) {
    DS1603L sensor(sensorSerial);
    log_prefix = log_prefix;
    // rx of the Arduino to tx of the sensor

    sensorSerial.begin(9600, SWSERIAL_8N1, rxPin, txPin, false);                         // Sensor transmits its data at 9600 bps.
    if (!sensorSerial) { // If the object did not initialize, then its configuration is invalid 
        ESP_LOGI(log_prefix, "Invalid SoftwareSerial pin configuration, check config");
    } 
    sensor.begin();   




}

int read_sd1603l() {
    unsigned int reading = sensor.readSensor();       // Call this as often or as little as you want - the sensor transmits every 1-2 seconds.
    byte sensorStatus = sensor.getStatus();           // Check the status of the sensor (not detected; checksum failed; reading success).
    switch (sensorStatus) {                           // For possible values see DS1603L.h
    case DS1603L_NO_SENSOR_DETECTED:                // No sensor detected: no valid transmission received for >10 seconds.
        ESP_LOGW(log_prefix, "No sensor detected (yet). If no sensor after 1 second, check whether your connections are good.")
        return -2;           

    case DS1603L_READING_CHECKSUM_FAIL:             // Checksum of the latest transmission failed.
        ESP_LOGE(log_prefix, "Data received; checksum failed. Latest level reading: %u", reading)
        return -1;   

    case DS1603L_READING_SUCCESS:                   // Latest reading was valid and received successfully.
        ESP_LOGI(log_prefix, "Water level : %u", reading)
        return reading;
    }
}