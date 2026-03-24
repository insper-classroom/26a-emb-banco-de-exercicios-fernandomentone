#include <stdio.h>

#include "hardware/gpio.h"
#include "pico/stdlib.h"

#define BTN_PIN_G 28
#define BTN_PIN_Y 26
#define BTN_PIN_R 20
#define LED_PIN_G 5
#define LED_PIN_Y 9
#define LED_PIN_R 13

int sequencia[] = {
    LED_PIN_Y, 
    LED_PIN_G, 
    LED_PIN_R, 
    LED_PIN_Y, 
    LED_PIN_G, 
    LED_PIN_Y,
    LED_PIN_R, 
    LED_PIN_Y, 
    LED_PIN_G, 
    LED_PIN_Y
};

volatile bool flag_btn_g = false;
volatile bool flag_btn_y = false;
volatile bool flag_btn_r = false;

typedef enum {ESPERANDO, MOSTRANDO, INPUT} state_t;
state_t state = ESPERANDO;

void btn_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL) {
        if (gpio == BTN_PIN_G) flag_btn_g = true;
        else if (gpio == BTN_PIN_Y) flag_btn_y = true;
        else if (gpio == BTN_PIN_R) flag_btn_r = true;
        

    }
}

int main() {

    stdio_init_all();

    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G, GPIO_IN);
    gpio_pull_up(BTN_PIN_G);

    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);

    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);

    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);

    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    gpio_set_irq_enabled_with_callback(BTN_PIN_G, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true);

    int rodada = 1;
    int input_index = 0;

    while (true) {
        if (state == ESPERANDO) {
            if (flag_btn_g) {
                flag_btn_g = false;
                state = MOSTRANDO;
            }
        }

        if (state == MOSTRANDO){
            for (int i = 0; i < rodada; i++) {
                gpio_put(sequencia[i], 1);
                sleep_ms(300);
                gpio_put(sequencia[i], 0);
                sleep_ms(300);
            }
            state = INPUT;
        }

        if (state == INPUT) {
            if (flag_btn_g) {
                flag_btn_g = false;
                if (sequencia[input_index] == LED_PIN_G) {
                    input_index++;
                } else {
                    printf("Points %d\n", rodada - 1);
                    rodada = 1;
                    input_index = 0;
                    state = ESPERANDO;
                }
            }

            if (flag_btn_y) {
                flag_btn_y = false;
                if (sequencia[input_index] == LED_PIN_Y) {
                    input_index++;
                } else {
                    printf("Points %d\n", rodada - 1);
                    rodada = 1;
                    input_index = 0;
                    state = ESPERANDO;
                }
            }

            if (flag_btn_r) {
                flag_btn_r = false;
                if (sequencia[input_index] == LED_PIN_R) {
                    input_index++;
                } else {
                    printf("Points %d\n", rodada - 1);
                    rodada = 1;
                    input_index = 0;
                    state = ESPERANDO;
                }
            }

            if (input_index == rodada) {
                input_index = 0;
                rodada++;
                if (rodada > 10) {
                    printf("Points %d\n", 10);
                    rodada = 1;
                    state = ESPERANDO;
                } else {
                    state = MOSTRANDO;
                }
            }

        }

    }
}
