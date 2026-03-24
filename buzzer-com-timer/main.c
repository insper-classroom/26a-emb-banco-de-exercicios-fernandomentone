#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico/stdlib.h"

#define BUZZER_PIN 6
#define FREQ_STEP 50
#define FREQ_MAX 2000
#define FREQ_MIN 200

volatile bool flag_timer = false;
volatile bool buzzer_state = false;
volatile int toggle_count = 0;

int freq = FREQ_MAX;

bool timer_callback(struct repeating_timer *t) {
    flag_timer = true;
    toggle_count++;
    return true;
}

int main() {

    stdio_init_all();

    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);

    struct repeating_timer timer;

    add_repeating_timer_us(
        1000000/(freq*2),
        timer_callback,
        NULL,
        &timer
    );

    while (true) {

        if (flag_timer) {

            flag_timer = false;
            buzzer_state = !buzzer_state;
            gpio_put(BUZZER_PIN, buzzer_state);

            if (toggle_count >= 200) {
                toggle_count = 0;
                freq -= FREQ_STEP;

                if (freq < FREQ_MIN)
                    freq = FREQ_MAX;

                cancel_repeating_timer(&timer);
                add_repeating_timer_us(
                    1000000/(freq*2),
                    timer_callback,
                    NULL,
                    &timer
                );
            }
        }
    }

    return 0;
}