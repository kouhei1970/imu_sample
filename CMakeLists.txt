cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(imu_sample C CXX ASM)
set(CMAKE_C_STNDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

pico_sdk_init()

add_executable(imu_sample
  imu_sample.cpp
  lsm9ds1_reg.c
)
target_link_libraries(imu_sample hardware_spi pico_stdlib)

pico_enable_stdio_usb(imu_sample 1)
pico_enable_stdio_uart(imu_sample 0)

pico_add_extra_outputs(imu_sample)

