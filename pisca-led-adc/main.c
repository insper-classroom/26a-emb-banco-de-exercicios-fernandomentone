#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/adc.h"

#define LED_PIN 4
#define POT_PIN 28

volatile int flag_timer = 0;
static struct repeating_timer timer;

static bool led_state = false;

bool timer_callback(struct repeating_timer *t) {
    flag_timer = 1;
    return true;
}

int main() {
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    adc_init();
    adc_gpio_init(POT_PIN);  // prepara o pino para ADC
    adc_select_input(2);     // pino 28 = canal 2

    int zone;
    int zone_old = -1;

    while (true) {

        uint16_t raw = adc_read();  // lê o valor (0..4095)
        float volts = raw * 3.3f / 4095;  // converte para volts

        if (volts <= 1.0)
            zone = 0;
        
        else if (volts <= 2.0)
            zone = 1;
        else
            zone = 2;

        if (zone != zone_old) {
            zone_old = zone;
            cancel_repeating_timer(&timer);

            if (zone == 0) {
            gpio_put(LED_PIN, 0);
            } else if (zone == 1) {
                add_repeating_timer_ms(300, timer_callback, NULL, &timer);
            } else {
                add_repeating_timer_ms(500, timer_callback, NULL, &timer);
            }
        }
        
        if (flag_timer) {
            flag_timer = 0;
            led_state = !led_state;
            gpio_put(LED_PIN, led_state);
        }
    }
}