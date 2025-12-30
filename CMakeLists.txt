cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

add_compile_options(-DLV_CONF_INCLUDE_SIMPLE)
project(ESP32-C6-LCD-1.47-Test)
