#include <stdio.h>

#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico/stdlib.h"

#define BTN_PIN_G 28
#define BTN_PIN_Y 26

#define LED_PIN_G 5
#define LED_PIN_Y 9

volatile int flag_btn_g = 0;
volatile int flag_btn_y = 0;

volatile int alarm_g = 0;
volatile int alarm_y = 0;

volatile int timer_g = 0;
volatile int timer_y = 0;

void btn_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL) {  // fall edge
        if (gpio == BTN_PIN_G) {
            flag_btn_g = 1;
        } else if (gpio == BTN_PIN_Y) {
            flag_btn_y = 1;
        }
    }
}

int64_t alarm_callback_g(alarm_id_t id, void *user_data) {
    alarm_g = 1;
    return 0;
}

int64_t alarm_callback_y(alarm_id_t id, void *user_data) {
    alarm_y = 1;
    return 0;
}

bool timer_callback_g(struct repeating_timer_t *rt) {
    timer_g = 1;
    return true; 
}

bool timer_callback_y(struct repeating_timer_t *rt) {
    timer_y = 1;
    return true; 
}

void setup() {
    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G, GPIO_IN);
    gpio_pull_up(BTN_PIN_G);

    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);

    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);

    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);

    gpio_set_irq_enabled_with_callback(BTN_PIN_G, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);
}

int main() {
    stdio_init_all();
    setup();

    alarm_id_t alarm_id_g = 0;
    alarm_id_t alarm_id_y = 0;

    repeating_timer_t start_timer_g;
    repeating_timer_t start_timer_y;

    int led_state_g = 0;
    int led_state_y = 0;

    int alarm_g_ativado = 0;
    int alarm_y_ativado = 0;

    while (1) {

        if (timer_g && alarm_g_ativado) {
            timer_g = 0;
            led_state_g = !led_state_g;
            gpio_put(LED_PIN_G, led_state_g);
        }

        if (timer_y && alarm_y_ativado) {
            timer_y = 0;
            led_state_y = !led_state_y;
            gpio_put(LED_PIN_Y, led_state_y);
        }

        if (flag_btn_g && !alarm_g_ativado) {
            flag_btn_g = 0;
            alarm_id_g = add_alarm_in_ms(1000, alarm_callback_g, NULL, false);
            add_repeating_timer_ms(100, timer_callback_g, NULL, &start_timer_g);
            alarm_g_ativado = 1;
        } 

        if (flag_btn_y && !alarm_y_ativado) {
            flag_btn_y = 0;
            alarm_id_y = add_alarm_in_ms(2000, alarm_callback_y, NULL, false);
            add_repeating_timer_ms(250, timer_callback_y, NULL, &start_timer_y);
            alarm_y_ativado = 1;
        } 

        if (alarm_g || alarm_y) {
            alarm_g = 0;
            alarm_y = 0;
            alarm_g_ativado = 0;
            alarm_y_ativado = 0;
            timer_g = 0;
            timer_y = 0;
            led_state_g = 0;
            led_state_y = 0;
            cancel_repeating_timer(&start_timer_g);
            cancel_repeating_timer(&start_timer_y);
            cancel_alarm(alarm_id_g);
            cancel_alarm(alarm_id_y);
            gpio_put(LED_PIN_G, 0);
            gpio_put(LED_PIN_Y, 0);
        }

    }

    return 0;
    }