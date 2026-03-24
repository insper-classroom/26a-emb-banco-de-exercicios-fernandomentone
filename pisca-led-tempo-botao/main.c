#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

#define LED_PIN 5
#define BTN_PIN 28

volatile int flag_btn = 0;
volatile int flag_timer = 0;
volatile int flag_alarm = 0;
volatile int tick_count = 0;

static struct repeating_timer timer_count;
static struct repeating_timer timer_blink;

bool count_callback(struct repeating_timer *t) {
    tick_count++;
    return true;
}

bool blink_callback(struct repeating_timer *t) {
    flag_timer = 1;
    return true;
}


void btn_callback(uint gpio, uint32_t events) {
    if (gpio != BTN_PIN) return;
    if (events & GPIO_IRQ_EDGE_FALL) {
        flag_btn = 1;
    }
    if (events & GPIO_IRQ_EDGE_RISE) {
        flag_btn = 2;
    }
}

int main() {
    stdio_init_all();

    gpio_init(BTN_PIN);
    gpio_set_dir(BTN_PIN, GPIO_IN);
    gpio_pull_up(BTN_PIN);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_set_irq_enabled_with_callback(BTN_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &btn_callback);

    while (true) {
        if (flag_btn == 1) {
            flag_btn = 0;
            cancel_repeating_timer(&timer_blink);
            gpio_put(LED_PIN, 0);
            flag_timer = 0;
            tick_count = 0;
            add_repeating_timer_ms(10, count_callback, NULL, &timer_count);
        }

        if (flag_btn == 2) {
            flag_btn = 0;
            cancel_repeating_timer(&timer_count);
            if (tick_count > 0) {
                gpio_put(LED_PIN, 1);
                add_repeating_timer_ms(tick_count * 10, blink_callback, NULL, &timer_blink);
            }
        }

        if (flag_timer) {
            flag_timer = 0;
            gpio_xor_mask(1u << LED_PIN);
        }
    }

    return 0;
}