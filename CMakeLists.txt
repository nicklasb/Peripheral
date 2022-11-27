cmake_minimum_required(VERSION 3.16.0)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS "../Controller/components/")
# TODO: Convince ESP-IDF to nest components
set(EXTRA_COMPONENT_DIRS "${EXTRA_COMPONENT_DIRS} components/robusto_sensors/bmv700 components/robusto_sensors/dht22 components/robusto_sensors/ds1603l") 

project(Peripheral)
