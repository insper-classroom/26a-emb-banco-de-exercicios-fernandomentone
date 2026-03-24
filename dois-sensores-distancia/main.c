#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

#define TRIG_1 13
#define ECHO_1 12
#define TRIG_2 19
#define ECHO_2 18

volatile int flag_timer = 0;

volatile int flag_echo_1 = 0;
volatile int flag_echo_2 = 0;

volatile int flag_watchdog_1 = 0;
volatile int flag_watchdog_2 = 0;

volatile uint64_t echo_start_1 = 0;
volatile uint64_t echo_start_2 = 0;
volatile uint64_t echo_dt_1 = 0;
volatile uint64_t echo_dt_2 = 0;

static alarm_id_t watchdog_alarm_1;
static alarm_id_t watchdog_alarm_2;

static struct repeating_timer timer;
static bool timer_running = false;

volatile int sensor_atual = 1;

bool timer_callback(struct repeating_timer *t) {
    flag_timer = 1;
    return true;
}

int64_t watchdog_callback_1(alarm_id_t id, void *user_data) {
    flag_watchdog_1 = 1;
    return 0;
}

int64_t watchdog_callback_2(alarm_id_t id, void *user_data) {
    flag_watchdog_2 = 1;
    return 0;
}

void echo_callback(uint gpio, uint32_t events) {

    if (gpio == ECHO_1) {

        if (events & GPIO_IRQ_EDGE_RISE)
            echo_start_1 = time_us_64();

        if (events & GPIO_IRQ_EDGE_FALL) {
            echo_dt_1 = time_us_64() - echo_start_1;
            flag_echo_1 = 1;
        }
    }

    if (gpio == ECHO_2) {

        if (events & GPIO_IRQ_EDGE_RISE)
            echo_start_2 = time_us_64();

        if (events & GPIO_IRQ_EDGE_FALL) {
            echo_dt_2 = time_us_64() - echo_start_2;
            flag_echo_2 = 1;
        }
    }
}

int main() {

    stdio_init_all();

    gpio_init(TRIG_1);
    gpio_set_dir(TRIG_1, GPIO_OUT);
    gpio_put(TRIG_1, 0);

    gpio_init(ECHO_1);
    gpio_set_dir(ECHO_1, GPIO_IN);

    gpio_init(TRIG_2);
    gpio_set_dir(TRIG_2, GPIO_OUT);
    gpio_put(TRIG_2, 0);

    gpio_init(ECHO_2);
    gpio_set_dir(ECHO_2, GPIO_IN);

    gpio_set_irq_enabled_with_callback(
        ECHO_1,
        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
        true,
        &echo_callback
    );

    gpio_set_irq_enabled(
        ECHO_2,
        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
        true
    );

    while (true) {

        if (!timer_running) {
            timer_running = true;
            add_repeating_timer_ms(-3000, timer_callback, NULL, &timer);
        }

        if (flag_timer) {
            flag_timer = 0;

            if (sensor_atual == 1) {
                
                flag_echo_1 = 0;
                flag_watchdog_1 = 0;

                watchdog_alarm_1 = add_alarm_in_ms(
                    60,
                    watchdog_callback_1,
                    NULL,
                    false
                );

                gpio_put(TRIG_1, 1);
                sleep_us(10);
                gpio_put(TRIG_1, 0);

            } else {

                flag_echo_2 = 0;
                flag_watchdog_2 = 0;

                watchdog_alarm_2 = add_alarm_in_ms(
                    60,
                    watchdog_callback_2,
                    NULL,
                    false
                );

                gpio_put(TRIG_2, 1);
                sleep_us(10);
                gpio_put(TRIG_2, 0);
            }
        }

        if (flag_echo_1) {

            flag_echo_1 = 0;

            cancel_alarm(watchdog_alarm_1);

            int distancia_1 = (int)((echo_dt_1 * 0.0343) / 2.0);

            printf("Sensor 1 - %d cm\n", distancia_1);

            sensor_atual = 2;
        }

        if (flag_watchdog_1) {

            flag_watchdog_1 = 0;

            printf("Sensor 1 - falha\n");

            sensor_atual = 2;
        }

        if (flag_echo_2) {

            flag_echo_2 = 0;

            cancel_alarm(watchdog_alarm_2);

            int distancia_2 = (int)((echo_dt_2 * 0.0343) / 2.0);

            printf("Sensor 2 - %d cm\n", distancia_2);

            sensor_atual = 1;
        }

        if (flag_watchdog_2) {

            flag_watchdog_2 = 0;

            printf("Sensor 2 - falha\n");

            sensor_atual = 1;
        }
    }
}