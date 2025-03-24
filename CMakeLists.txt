cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr)
project(flightbus_demo_cdc_acm)

FILE(GLOB app_sources src/*.cpp src/interfaces/*.c)
target_sources(app PRIVATE ${app_sources})

list(APPEND CMAKE_MODULE_PATH ${ZEPHYR_BASE}/modules/nanopb)
include(nanopb)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
