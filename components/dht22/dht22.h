#ifndef _DHT22_H_
#define _DHT22_H_

struct dht22_result {
    float humidity;
    float temperature;
};

void dht22_init(char * _log_prefix);

struct dht22_result dht22_read();

#endif