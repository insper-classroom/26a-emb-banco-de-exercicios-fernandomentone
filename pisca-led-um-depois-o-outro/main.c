#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/adc.h"

const int LED_PIN_Y = 10;
const int LED_PIN_B = 14;

const int BTN_PIN_Y = 26;
const int BTN_PIN_B = 19;

static struct repeating_timer timer;
volatile int flag_timer = 0;

static int flag_btn_y = 0;
static int flag_btn_b = 0;

static alarm_id_t alarm;
volatile int flag_alarm;

bool timer_callback(struct repeating_timer *t) {
    flag_timer = 1;
    return true;
}

void btn_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL) {
        if (gpio == BTN_PIN_Y) {
            flag_btn_y = 1;
        } else if ( gpio == BTN_PIN_B) {
            flag_btn_b = 1;
        }
    }
}

int64_t alarm_callback(alarm_id_t id, void *user_data) {
    flag_alarm = 1;
    return 0;
}


int main(){

    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);

    gpio_init(BTN_PIN_B);
    gpio_set_dir(BTN_PIN_B, GPIO_IN);
    gpio_pull_up(BTN_PIN_B);

    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);

    gpio_init(LED_PIN_B);
    gpio_set_dir(LED_PIN_B, GPIO_OUT);

    gpio_set_irq_enabled_with_callback(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_B, GPIO_IRQ_EDGE_FALL, true);
    
    stdio_init_all();

    int fase = 0;
    int ordem = 0; // 1 = amarelo primeiro, 2 = azul primeiro

    while (1) {

        // --- Botão pressionado (só aceita se ocioso) ---
        if (flag_btn_y && fase == 0) {
            flag_btn_y = 0;
            fase = 1;
            ordem = 1; // amarelo primeiro
            gpio_put(LED_PIN_Y, 1);
            add_repeating_timer_ms(100, timer_callback, NULL, &timer); // 5Hz
            alarm = add_alarm_in_ms(1000, alarm_callback, NULL, false);
        }

        if (flag_btn_b && fase == 0) {
            flag_btn_b = 0;
            fase = 1;
            ordem = 2; // azul primeiro
            gpio_put(LED_PIN_B, 1);
            add_repeating_timer_ms(250, timer_callback, NULL, &timer); // 2Hz
            alarm = add_alarm_in_ms(2000, alarm_callback, NULL, false);
        }

        // --- Alarm disparou: transição de fase ---
        if (flag_alarm) {
            flag_alarm = 0;
            cancel_repeating_timer(&timer);

            if (fase == 1) {
                // Apaga o primeiro LED, inicia o segundo
                if (ordem == 1) {
                    gpio_put(LED_PIN_Y, 0);
                    gpio_put(LED_PIN_B, 1);
                    add_repeating_timer_ms(250, timer_callback, NULL, &timer);
                    alarm = add_alarm_in_ms(2000, alarm_callback, NULL, false);
                } else {
                    gpio_put(LED_PIN_B, 0);
                    gpio_put(LED_PIN_Y, 1);
                    add_repeating_timer_ms(100, timer_callback, NULL, &timer);
                    alarm = add_alarm_in_ms(1000, alarm_callback, NULL, false);
                }
                fase = 2;

            } else if (fase == 2) {
                // Apaga tudo, volta pro ocioso
                gpio_put(LED_PIN_Y, 0);
                gpio_put(LED_PIN_B, 0);
                fase = 0;
            }
        }

        // --- Toggle do LED ativo ---
        if (flag_timer) {
            flag_timer = 0;
            if (fase == 1) {
                if (ordem == 1) gpio_xor_mask(1u << LED_PIN_Y);
                else            gpio_xor_mask(1u << LED_PIN_B);
            } else if (fase == 2) {
                if (ordem == 1) gpio_xor_mask(1u << LED_PIN_B);
                else            gpio_xor_mask(1u << LED_PIN_Y);
            }
        }
    }

    return 0;
}