#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

// sensor lowest raw read value
const long RAW_LOW = 1100;

// sensor highest raw read value
const long RAW_HIGH = 2400;


void init_sensor(uint8_t pin) {
    adc_init();

    adc_gpio_init(pin);

    adc_select_input(0);
}

// returning the percentage from adc
long read_sensor(void) {
    long adc_val = adc_read();

    return (adc_val - RAW_HIGH) * 100 / (RAW_LOW - RAW_HIGH);
}

