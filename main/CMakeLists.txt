idf_component_register(
    SRCS ${app_sources} "*.*" "../secret/local_settings.h" 
    INCLUDE_DIRS "." "../include" "../lib"
    REQUIRES robusto arduino-esp32 ds1603l bmv700 dht22 
) 
