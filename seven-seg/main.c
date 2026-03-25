#include "pico/stdlib.h"
#include <stdio.h>


#include "hardware/gpio.h"

#define BTN_PIN 28
#define A 2
#define B 3
#define C 4
#define D 5
#define E 6
#define F 7
#define G 8

void seven_seg_init() {
    gpio_init(A);
    gpio_set_dir(A, GPIO_OUT);

    gpio_init(B);
    gpio_set_dir(B, GPIO_OUT);

    gpio_init(C);
    gpio_set_dir(C, GPIO_OUT);

    gpio_init(D);
    gpio_set_dir(D, GPIO_OUT);

    gpio_init(E);
    gpio_set_dir(E, GPIO_OUT);

    gpio_init(F);
    gpio_set_dir(F, GPIO_OUT);

    gpio_init(G);
    gpio_set_dir(G, GPIO_OUT);
}

void seven_seg_display(int val) {
    static const int numeros[10][7] = {
        {1, 1, 1, 1, 1, 1, 0}, // 0
        {0, 1, 1, 0, 0, 0, 0}, // 1
        {1, 1, 0, 1, 1, 0, 1}, // 2
        {1, 1, 1, 1, 0, 0, 1}, // 3
        {0, 1, 1, 0, 0, 1, 1}, // 4
        {1, 0, 1, 1, 0, 1, 1}, // 5
        {1, 0, 1, 1, 1, 1, 1}, // 6
        {1, 1, 1, 0, 0, 0, 0}, // 7
        {1, 1, 1, 1, 1, 1, 1}, // 8
        {1, 1, 1, 1, 0, 1, 1}  // 9
    };

    gpio_put(A, numeros[val][0]);
    gpio_put(B, numeros[val][1]);
    gpio_put(C, numeros[val][2]);
    gpio_put(D, numeros[val][3]);
    gpio_put(E, numeros[val][4]);
    gpio_put(F, numeros[val][5]);
    gpio_put(G, numeros[val][6]);
}

volatile int flag_btn = 0;

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BTN_PIN) {
        flag_btn = 1;
    }
}


int main() {

    gpio_init(BTN_PIN);
    gpio_set_dir(BTN_PIN, GPIO_IN);
    gpio_pull_up(BTN_PIN);

    seven_seg_init();

    gpio_set_irq_enabled_with_callback(BTN_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);



    stdio_init_all();

    int contador = 0;

    while (true) {
        if (flag_btn){
            flag_btn = 0;
            contador++;
        }

        if(contador > 9) {
            contador = 0;
        }

        seven_seg_display(contador);
    }
}
