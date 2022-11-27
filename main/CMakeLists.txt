idf_component_register(SRCS ${app_sources} "main.c" "../include/sdp_task.h" "../include/sdp_task.c" "../secret/local_settings.h" 
INCLUDE_DIRS "." "../include" "../lib"
REQUIRES sdp arduino-esp32 ds1603l bmv700 dht22) 
