FILE(GLOB_RECURSE sources ./*.*)
idf_component_register(SRCS ${sources} INCLUDE_DIRS . 
REQUIRES VeDirectFrameHandler espsoftwareserial arduino-esp32) 