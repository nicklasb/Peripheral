
# Peripheral

***Important: This needs to be in a folder next to the folder of the [Controller](https://github.com/nicklasb/Controller) project.***

This is a peripheral unit for collecting data from sensors and forwarding to a [central Controller](https://github.com/nicklasb/Controller) using:  
* Competent multi-core usage for maximal performance and stability
* BLE for common, always-on situations 
* Waking Controller and being woken using GPIO, then BLE for low-power situations and to sound alarms.   
*Like intrusion (theft, camera), danger (fire, leaks)*

The first evolution is hard-coded to a specific setup, the next will attempt to be more dynamic.



# Progress
Done:
* A buildable Platform IO project


WIP:

* Create a custom BLE peripheral that sends any data to the controller


Upcoming:
* Implement a simple high-level protocol that effeciently can send many kinds of sensor data
* Implement support for a lot of different sensors in the peripherals (see that repo)




## Platform

### Software
This is an esp-idf project, but as I also like PlatformIO, I use /src rather than /main folder structure. 
Otherwise it looks the same. Also not sure why esp-idf wants to deviate from the norm.

### Hardware

- TTGO LoRa32 module (CH9102) 
    - Bluetooth Low Energy (BLE)
    - ESP32 WROVER
    - LoRa 868mhz (This project doesn't use LoRa, at least not initially) 
    - SSD1306 OLED (will not be used initially)

I use this module just because I hade it lying around, serious stuff will be whatever cheap ESP32.

## Custom 

These are settings that might be, and probably is, specific to my setup.  


### Serial flasher config

- Flash SPI mode (QIO)  --->
- Flash SPI speed (80 MHz)  --->  
*These were the flash settings that worked for the LoRa32.*

- CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ needs to be 240Mhz. For some reason that isn't the default all the time. 
- CONFIG_ESP32_XTAL_FREQ needs to be 26 Mhz. The crystal seems to be 26 even though the default is 40 Mhz in the ESP32 menuconfig 

### Component config 


#### Bluetooth
Uses Nimble, and mostly defaults.  
*(in contrast, the [ble_server](https://github.com/nicklasb/ble_server) and [ble_client](https://github.com/nicklasb/ble_client) repos explore the extremes of MTU sizes and transfer rates and other tricks, if that is of interest)*  

- [x] Bluetooth
    - Bluetooth controller(ESP32 Dual Mode Bluetooth)  --->  
     Bluetooth controller mode (BR/EDR/BLE/DUALMODE) (BLE Only)  --->  
    - Bluetooth Host (NimBLE - BLE only)  --->  
    - NimBLE Options  --->  


## TODO:

