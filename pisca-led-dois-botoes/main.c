#include <stdio.h>

#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico/stdlib.h"

#define LED_Y 9
#define LED_G 5
#define BTN_Y 26
#define BTN_G 28

static struct repeating_timer timer_y;
static struct repeating_timer timer_g;
volatile int flag_timer_y = 0;
volatile int flag_timer_g = 0;

volatile int flag_btn_y = 0;
volatile int flag_btn_g = 0;

static bool led_g_active = false;
static bool led_y_active = false;

volatile int flag_alarm_g = 0;
volatile int flag_alarm_y = 0;
static alarm_id_t alarm_g;
static alarm_id_t alarm_y;


bool timer_callback_y(struct repeating_timer *t) {
    flag_timer_y = 1;
    return true;
}

bool timer_callback_g(struct repeating_timer *t) {
    flag_timer_g = 1;
    return true;
}


void btn_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL) {
        if (gpio == BTN_Y) {
            flag_btn_y = 1;
        } else if ( gpio == BTN_G) {
            flag_btn_g = 1;
        }

    }
}

int64_t alarm_callback_y(alarm_id_t id, void *user_data) {
    flag_alarm_y = 1;
    return 0;
}

int64_t alarm_callback_g(alarm_id_t id, void *user_data) {
    flag_alarm_g = 1;
    return 0;
}


int main() {

    gpio_init(BTN_Y);
    gpio_set_dir(BTN_Y, GPIO_IN);
    gpio_pull_up(BTN_Y);

    gpio_init(BTN_G);
    gpio_set_dir(BTN_G, GPIO_IN);
    gpio_pull_up(BTN_G);

    gpio_init(LED_Y);
    gpio_set_dir(LED_Y, GPIO_OUT);

    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);

    gpio_set_irq_enabled_with_callback(BTN_Y, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled(BTN_G, GPIO_IRQ_EDGE_FALL, true);

    stdio_init_all();

    while (1) {
        if (flag_btn_y) {
            flag_btn_y = 0;
            flag_alarm_y = 0;  // limpa alarm pendente
            led_y_active = true;
            gpio_put(LED_Y, 1);
            add_repeating_timer_ms(250, timer_callback_y, NULL, &timer_y);
            alarm_y = add_alarm_in_ms(2000, alarm_callback_y, NULL, false);
        }

        if (flag_btn_g) {
            flag_btn_g = 0;
            flag_alarm_g = 0;  // limpa alarm pendente
            led_g_active = true;
            gpio_put(LED_G, 1);
            add_repeating_timer_ms(100, timer_callback_g, NULL, &timer_g);
            alarm_g = add_alarm_in_ms(1000, alarm_callback_g, NULL, false);
        }

        if (flag_alarm_g) {
            flag_alarm_g = 0;
            cancel_repeating_timer(&timer_g);
            flag_timer_g = 0;
            led_g_active = false;
            gpio_put(LED_G, 0);

            if (led_y_active) {
                cancel_repeating_timer(&timer_y);
                cancel_alarm(alarm_y);
                led_y_active = false;
                gpio_put(LED_Y, 0);
            }
        }

        if (flag_alarm_y) {
            flag_alarm_y = 0;
            cancel_repeating_timer(&timer_y);
            flag_timer_y = 0;
            led_y_active = false;
            gpio_put(LED_Y, 0);

            if (led_g_active) {
                cancel_repeating_timer(&timer_g);
                cancel_alarm(alarm_g);
                led_g_active = false;
                gpio_put(LED_G, 0);
            }
        }

        if (flag_timer_y) {
            flag_timer_y = 0;
            gpio_xor_mask(1u << LED_Y);
        }

        if (flag_timer_g) {
            flag_timer_g = 0;
            gpio_xor_mask(1u << LED_G);
        }

    
    }
}