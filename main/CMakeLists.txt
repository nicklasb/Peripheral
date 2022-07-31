
#FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/main/*.*)
#idf_component_register(SRCS ${app_sources}) 

idf_component_register(SRCS ${app_sources} "main.c" "../include/sdp_task.h" "../include/sdp_task.c" 
INCLUDE_DIRS "." "../include" "../lib"
REQUIRES sdp arduino ds1603l bmv700) 
