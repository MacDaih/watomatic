#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>
#include "display.h"
#include "sensor.h"


const uint8_t SENSOR_PIN = 26;
const uint8_t PUMP_PIN = 22;
const uint8_t ADC_CHAN = 0;
const uint8_t LED = 25;

const long THRESHOLD = 40;
const int WAIT_TIME = 900;
void blink_init(void) {
    for(int i = 0; i < 5;i++) {
        gpio_put(LED, 1);
        sleep_ms(500);
        gpio_put(LED,0);
        sleep_ms(500);
    }
} 

void run_watering(void) {
    gpio_put(PUMP_PIN, 1);
    sleep_ms(5000);
    gpio_put(PUMP_PIN, 0);
}

int main(void) {
    gpio_init(LED);
    gpio_set_dir(LED, GPIO_OUT);

    gpio_init(PUMP_PIN);
    gpio_set_dir(PUMP_PIN, GPIO_OUT);

    init_sensor(SENSOR_PIN);
    init_display();

    blink_init();

    char p[80];
    int elapsed = 0;
    for(;;) {
        long val = read_sensor();
        if(val <= THRESHOLD && elapsed <= 0) {
            elapsed = WAIT_TIME;
            sprintf(p,"watering ... ");
            render_display(p);
            run_watering();
        } else {
            sprintf(p,"soil moisture %3d %%",val);
            render_display(p);
            sleep_ms(1000);
            elapsed--;
        }
    }
}
