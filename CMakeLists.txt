cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr)
project(flightbus_demo_cdc_acm)

FILE(GLOB app_sources src/*.cpp src/interfaces/*.c)
target_sources(app PRIVATE ${app_sources})