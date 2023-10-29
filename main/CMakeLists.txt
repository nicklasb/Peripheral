FILE(GLOB app_sources  "main.c" "*.c" "../secret/local_settings.h")

idf_component_register(
    SRCS ${app_sources}
    INCLUDE_DIRS "." "../include" "../lib"
    REQUIRES robusto ds1603l bmv700 dht22 
) 
