#include <stdio.h>        // Biblioteca padrão de entrada e saída
#include "hardware/adc.h" // Biblioteca para manipulação do ADC no RP2040
#include "hardware/pwm.h" // Biblioteca para controle de PWM no RP2040
#include "pico/stdlib.h"  // Biblioteca padrão do Raspberry Pi Pico

// Definição dos pinos usados para o joystick e LEDs
const int VRX = 26;          // Pino de leitura do eixo X do joystick (conectado ao ADC)
const int VRY = 27;          // Pino de leitura do eixo Y do joystick (conectado ao ADC)
const int ADC_CHANNEL_0 = 0; // Canal ADC para o eixo X do joystick
const int ADC_CHANNEL_1 = 1; // Canal ADC para o eixo Y do joystick
const int SW = 22;           // Pino de leitura do botão do joystick
const int BUTTON = 5;        // Pino do botão de interrupção
const int LED_TOGGLE = 12;   // Pino do LED controlado por interrupção

const int LED_B = 13;                    // Pino para controle do LED azul via PWM
const int LED_R = 11;                    // Pino para controle do LED vermelho via PWM
const float DIVIDER_PWM = 16.0;          // Divisor fracional do clock para o PWM
const uint16_t PERIOD = 4096;            // Período do PWM (valor máximo do contador)
uint16_t led_b_level, led_r_level = 100; // Inicialização dos níveis de PWM para os LEDs
uint slice_led_b, slice_led_r;           // Variáveis para armazenar os slices de PWM correspondentes aos LEDs
volatile bool led_toggle_state = false;  // Estado do LED que será alternado

// Função de callback para interrupção do botão
type void gpio_callback(uint gpio, uint32_t events) {
  if (gpio == BUTTON) {
    led_toggle_state = !led_toggle_state; // Alterna o estado do LED
    gpio_put(LED_TOGGLE, led_toggle_state);
  }
}

// Função para configurar o joystick (pinos de leitura e ADC)
void setup_joystick() {
  adc_init();         // Inicializa o módulo ADC
  adc_gpio_init(VRX); // Configura o pino VRX (eixo X) para entrada ADC
  adc_gpio_init(VRY); // Configura o pino VRY (eixo Y) para entrada ADC

  gpio_init(SW);             // Inicializa o pino do botão
  gpio_set_dir(SW, GPIO_IN); // Configura o pino do botão como entrada
  gpio_pull_up(SW);          // Ativa o pull-up no pino do botão para evitar flutuações
}

// Função para configurar o PWM de um LED (genérica para azul e vermelho)
void setup_pwm_led(uint led, uint *slice, uint16_t level) {
  gpio_set_function(led, GPIO_FUNC_PWM);
  *slice = pwm_gpio_to_slice_num(led);
  pwm_set_clkdiv(*slice, DIVIDER_PWM);
  pwm_set_wrap(*slice, PERIOD);
  pwm_set_gpio_level(led, level);
  pwm_set_enabled(*slice, true);
}

// Configuração da interrupção no botão
void setup_interrupt_button() {
  gpio_init(BUTTON);
  gpio_set_dir(BUTTON, GPIO_IN);
  gpio_pull_up(BUTTON);
  gpio_set_irq_edge_fall(BUTTON, true);
  gpio_set_irq_callback(gpio_callback);
  irq_set_enabled(IO_IRQ_BANK0, true);

  gpio_init(LED_TOGGLE);
  gpio_set_dir(LED_TOGGLE, GPIO_OUT);
}

// Função de configuração geral
void setup() {
  stdio_init_all();
  setup_joystick();
  setup_pwm_led(LED_B, &slice_led_b, led_b_level);
  setup_pwm_led(LED_R, &slice_led_r, led_r_level);
  setup_interrupt_button();
}

// Função para ler os valores dos eixos do joystick (X e Y)
void joystick_read_axis(uint16_t *vrx_value, uint16_t *vry_value) {
  adc_select_input(ADC_CHANNEL_0);
  sleep_us(2);
  *vrx_value = adc_read();

  adc_select_input(ADC_CHANNEL_1);
  sleep_us(2);
  *vry_value = adc_read();
}

// Função principal
int main() {
  uint16_t vrx_value, vry_value;
  setup();
  printf("Joystick-PWM\n");
  
  while (1) {
    joystick_read_axis(&vrx_value, &vry_value);
    pwm_set_gpio_level(LED_B, vrx_value);
    pwm_set_gpio_level(LED_R, vry_value);
    sleep_ms(100);
  }
}
