
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/adc.h"

#define BTN_PIN 28
#define LED_Y 5
#define LED_B 9

static struct repeating_timer timer_y;
static struct repeating_timer timer_b;
volatile int flag_timer_y = 0;
volatile int flag_timer_b = 0;

static bool led_b_active = false;
static bool led_y_active = false;

volatile int flag_btn = 0;

static alarm_id_t alarm;
volatile int flag_alarm;

bool timer_callback_y(struct repeating_timer *t) {
    flag_timer_y = 1;
    return true;
}

bool timer_callback_b(struct repeating_timer *t) {
    flag_timer_b = 1;
    return true;
}

void btn_callback (uint gpio, uint32_t events) {
    if (gpio != BTN_PIN) return;
    if (events & GPIO_IRQ_EDGE_FALL) {
        flag_btn = 1;
    }
}

int64_t alarm_callback(alarm_id_t id, void *user_data) {
    flag_alarm = 1;
    return 0;
}

int main() {

    gpio_init(BTN_PIN);
    gpio_set_dir(BTN_PIN, GPIO_IN);
    gpio_pull_up(BTN_PIN);

    gpio_init(LED_Y);
    gpio_set_dir(LED_Y, GPIO_OUT);

    gpio_init(LED_B);
    gpio_set_dir(LED_B, GPIO_OUT);

    gpio_set_irq_enabled_with_callback(BTN_PIN, GPIO_IRQ_EDGE_FALL, true, &btn_callback);

    stdio_init_all();


    while (true) {
        if (flag_btn) {
            flag_btn = 0;
            led_b_active = true;
            led_y_active = true;
            gpio_put(LED_Y, 1);
            gpio_put(LED_B, 1);
            add_repeating_timer_ms(500, timer_callback_y, NULL, &timer_y);
            add_repeating_timer_ms(150, timer_callback_b, NULL, &timer_b);
            alarm = add_alarm_in_ms(5000, alarm_callback, NULL, false);
        }

        if (flag_alarm) {
            flag_alarm = 0;
            cancel_repeating_timer(&timer_y);
            cancel_repeating_timer(&timer_b);
            flag_timer_y = 0;
            flag_timer_b = 0;
            led_b_active = false;
            led_y_active = false;
            gpio_put(LED_B, 0);
            gpio_put(LED_Y, 0);
        }

        if (flag_timer_y) {
            flag_timer_y = 0;
            gpio_xor_mask(1u << LED_Y);
        }

        if (flag_timer_b) {
            flag_timer_b = 0;
            gpio_xor_mask(1u << LED_B);
        }
    

    }
}
