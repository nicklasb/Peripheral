/* 
	DHT22 temperature sensor driver
*/

#ifndef DHT_H_
#define DHT_H_

#include <sdkconfig.h>
#if CONFIG_ROBUSTO_LOAD_DHT22 

#include <stdbool.h>

#define DHT_OK 0
#define DHT_CHECKSUM_ERROR -1
#define DHT_TIMEOUT_ERROR -2

// == function prototypes =======================================

void setDHTgpio(int gpio);
void errorHandler(int response);
int readDHT();
float getHumidity();
float getTemperature();
int getSignalLevel(int usTimeOut, bool state);

#endif
#endif