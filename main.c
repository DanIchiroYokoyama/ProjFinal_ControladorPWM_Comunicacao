// SEL0433 - Aplicação de Microprocessadores
// Projeto Final Parte 1: Controle PWM de LED RGB
// Daniel Ichiro Pacheco Yokoyama Pereira - 15635969
// Aljan Almeida Dias - 15509165

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"

// Pinos do LED RGB (catodo comum)
#define PIN_R 25
#define PIN_G 26
#define PIN_B 27

// Canais LEDC
#define CH_R LEDC_CHANNEL_0
#define CH_G LEDC_CHANNEL_1
#define CH_B LEDC_CHANNEL_2

#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define PWM_FREQ_HZ 5000 // 5 kHz
#define PWM_RES_BITS LEDC_TIMER_8_BIT // 8 bits -> 0-255

// Incrementos individuais (em %)
static const int incGreen = 5; // verde: 5 em 5 %
static const int incBlue = incGreen * 2; // azul : 10 em 10 %
static const int incRed = incGreen * 3; // vermelho: 15 em 15 %

// Duty cycle atual de cada canal (em %)
static int dutyR = 0;
static int dutyG = 0;
static int dutyB = 0;

// Converte porcentagem (0-100) para valor de 8 bits (0-255)
static int percent_to_duty8(int percent){
  return (percent * 255) / 100;
}

// Configura um canal LEDC associado a um pino GPIO
static void configura_canal(ledc_channel_t canal, int gpio_num){
    ledc_channel_config_t ch_conf = {
      .gpio_num = gpio_num,
      .speed_mode = LEDC_MODE,
      .channel = canal,
      .intr_type = LEDC_INTR_DISABLE,
      .timer_sel = LEDC_TIMER,
      .duty = 0,
      .hpoint = 0
    };
  ESP_ERROR_CHECK(ledc_channel_config(&ch_conf));
}

void app_main(void){
  // Configura o timer LEDC (compartilhado pelos 3 canais)
  ledc_timer_config_t timer_conf = {
    .speed_mode = LEDC_MODE,
    .timer_num = LEDC_TIMER,
    .duty_resolution = PWM_RES_BITS,
    .freq_hz = PWM_FREQ_HZ,
    .clk_cfg = LEDC_AUTO_CLK
  };
  ESP_ERROR_CHECK(ledc_timer_config(&timer_conf));

  // Configura os 3 canais PWM (R, G, B)
  configura_canal(CH_R, PIN_R);
  configura_canal(CH_G, PIN_G);
  configura_canal(CH_B, PIN_B);

  // o console padrão do ESP-IDF já opera em 115200 baud por padrão, então
  //  o printf() já atende ao requisito de comunicação serial UART em 115200
  printf("Projeto 3 - Parte 1: PWM RGB com LEDC (ESP-IDF)\n");

  while (1) {
    dutyR += incRed; if (dutyR > 100) dutyR = incRed;
    dutyG += incGreen; if (dutyG > 100) dutyG = incGreen;
    dutyB += incBlue; if (dutyB > 100) dutyB = incBlue;

    ledc_set_duty(LEDC_MODE, CH_R, percent_to_duty8(dutyR));
    ledc_update_duty(LEDC_MODE, CH_R);

    ledc_set_duty(LEDC_MODE, CH_G, percent_to_duty8(dutyG));
    ledc_update_duty(LEDC_MODE, CH_G);

    ledc_set_duty(LEDC_MODE, CH_B, percent_to_duty8(dutyB));
    ledc_update_duty(LEDC_MODE, CH_B);

    printf("VERMELHO -> incremento: %d%% | duty cycle: %d%%\n", incRed, dutyR);
    printf("VERDE -> incremento: %d%% | duty cycle: %d%%\n", incGreen, dutyG);
    printf("AZUL -> incremento: %d%% | duty cycle: %d%%\n", incBlue, dutyB);

    vTaskDelay(pdMS_TO_TICKS(400));
  }
}