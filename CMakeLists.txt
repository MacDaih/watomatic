 cmake_minimum_required(VERSION 3.28)

# initialize pico-sdk from submodule
# note: this must happen before project()
include(~/pico-sdk/pico_sdk_init.cmake)

project(watomatic)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()
add_executable(watomatic
        main.c
        adc_sensor.c
        display.c
        )

# pull in common dependencies
target_link_libraries(watomatic 
    pico_stdlib
    hardware_i2c
    hardware_adc
    )

# create map/bin/hex file etc.
pico_add_extra_outputs(watomatic)
