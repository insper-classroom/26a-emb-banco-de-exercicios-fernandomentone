#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/gpio.h"

#define LED_Y 6
#define LED_B 10
#define LED_G 14
#define BTN_Y 28
#define BTN_B 22
#define BTN_G 18

volatile int flag_btn_y = 0;
volatile int flag_btn_b = 0;
volatile int flag_btn_g = 0;

static bool led_y_active = false;
static bool led_b_active = false;
static bool btn_pressed_g = false;

static bool blink_state = false;

void btn_callback(uint gpio, uint32_t events) {
  if (gpio == BTN_Y) {
    if (events & GPIO_IRQ_EDGE_FALL) {
      flag_btn_y = 1;
    }
  }

  if (gpio == BTN_B) {
    if (events & GPIO_IRQ_EDGE_RISE) {
      flag_btn_b = 1;
    }
  }

  if (gpio == BTN_G) {
    if (events & GPIO_IRQ_EDGE_FALL) {
      flag_btn_g = 1;
    }
    if (events & GPIO_IRQ_EDGE_RISE) {
      flag_btn_g = 2;
    }
  }
}

int main() {
  stdio_init_all();

  gpio_init(BTN_Y);
  gpio_set_dir(BTN_Y, GPIO_IN);
  gpio_pull_up(BTN_Y);

  gpio_init(BTN_B);
  gpio_set_dir(BTN_B, GPIO_IN);
  gpio_pull_up(BTN_B);

  gpio_init(BTN_G);
  gpio_set_dir(BTN_G, GPIO_IN);
  gpio_pull_up(BTN_G);

  gpio_init(LED_Y);
  gpio_set_dir(LED_Y, GPIO_OUT);
  
  gpio_init(LED_B);
  gpio_set_dir(LED_B, GPIO_OUT);

  gpio_init(LED_G);
  gpio_set_dir(LED_G, GPIO_OUT);

  gpio_set_irq_enabled_with_callback(BTN_Y, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
  gpio_set_irq_enabled(BTN_B, GPIO_IRQ_EDGE_RISE, true);
  gpio_set_irq_enabled(BTN_G, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);

  while (true) {

    // tratamento dos botões
    if (flag_btn_y == 1) {
      flag_btn_y = 0;
      led_y_active = !led_y_active;
    }

    if (flag_btn_b == 1) {
      flag_btn_b = 0;
      led_b_active = !led_b_active;
    }

    if (flag_btn_g == 1) {
      flag_btn_g = 0;
      btn_pressed_g = true;
    }

    if (flag_btn_g == 2) {
      flag_btn_g = 0;
      btn_pressed_g = false;
    }

    // clock global de 400ms
    blink_state = !blink_state;

    // aplicação sincronizada
    if (led_y_active)
      gpio_put(LED_Y, blink_state);
    else
      gpio_put(LED_Y, 0);

    if (led_b_active)
      gpio_put(LED_B, blink_state);
    else
      gpio_put(LED_B, 0);

    if (btn_pressed_g)
      gpio_put(LED_G, blink_state);
    else
      gpio_put(LED_G, 0);

    sleep_ms(400);
  }
}