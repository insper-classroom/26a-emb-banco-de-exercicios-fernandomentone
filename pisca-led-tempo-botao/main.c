/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h> 
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

#define LED_PIN 5
#define BTN_PIN 28

static struct repeating_timer timer;
volatile int flag_timer = 0;
volatile int flag_btn = 0;

static volatile uint64_t press_start_time = 0;
static volatile int64_t blink_period = 0;
static volatile bool btn_pressed = false;

bool timer_callback(struct repeating_timer *t) {
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
            btn_pressed = true;
            press_start_time = time_us_64();
            gpio_put(LED_PIN, 0);
            cancel_repeating_timer(&timer);
        }

        if (flag_btn == 2) {
            flag_btn = 0;
            btn_pressed = false;
            blink_period = time_us_64() - press_start_time;

            if (blink_period < 100000) {
                blink_period = 100000; // mínimo de 100ms
            }

            cancel_repeating_timer(&timer);
            add_repeating_timer_us(-blink_period, timer_callback, NULL, &timer);
        }
        if (flag_timer) {
            flag_timer = 0;
            if (!btn_pressed) {
                gpio_put(LED_PIN, !gpio_get(LED_PIN));
            }
        }

    }

    return 0;
}