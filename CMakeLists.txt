cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(flightbus_demo_cdc_acm)
set(SOURCE_FILES 
        ${CMAKE_SOURCE_DIR}/src/main.cpp
        ${CMAKE_SOURCE_DIR}/src/interfaces/uart_cdc_acm.c
        ${CMAKE_SOURCE_DIR}/src/interfaces/protobufs/protobuf_util.cpp
        ${CMAKE_SOURCE_DIR}/src/interfaces/uart_msg_processor.cpp
        ${CMAKE_SOURCE_DIR}/src/wrappers/UartCdcAcmWrapper.cpp
)
target_sources(app PRIVATE ${SOURCE_FILES})

list(APPEND CMAKE_MODULE_PATH ${ZEPHYR_BASE}/modules/nanopb)
include(nanopb)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
zephyr_nanopb_sources(app src/interfaces/protobufs/flightbus_message.proto)
zephyr_include_directories(src)
enable_language(CXX)