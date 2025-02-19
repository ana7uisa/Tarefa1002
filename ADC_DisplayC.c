// Importação das bibliotecas necessárias para funcionamento do código
#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "./lib/ssd1306.h"
#include "./lib/font.h"

// Definição de constantes e pinos utilizados no hardware
#define I2C_BUS i2c1  // Barramento I2C utilizado
#define SDA_PIN 14  // Pino de comunicação SDA do I2C
#define SCL_PIN 15  // Pino de comunicação SCL do I2C
#define OLED_ADDR 0x3C  // Endereço do display OLED SSD1306
#define JOY_X 27  // Pino do eixo X do joystick
#define JOY_Y 26  // Pino do eixo Y do joystick
#define BTN_MAIN 22  // Botão principal do joystick
#define BTN_ALT 5  // Botão adicional
#define LED_R 13  // LED vermelho
#define LED_B 12  // LED azul
#define LED_G 11  // LED verde (liga/desliga)
#define PWM_LIMIT 24999  // Valor máximo do PWM
#define PWM_CLOCK_DIV 100.0  // Divisor de clock do PWM
#define DEBOUNCE_DELAY 200  // Tempo para evitar bounce nos botões

// Variáveis globais usadas no controle dos LEDs e display
volatile bool state_led_g = false;  // Estado do LED verde
volatile bool pwm_active = true;  // Estado de ativação do PWM
volatile bool display_inverted = true;  // Controle da cor do display
volatile uint32_t prev_interrupt_time = 0;  // Armazena tempo da última interrupção
float pwm_value_x, pwm_value_y;  // Valores de PWM para os LEDs
ssd1306_t oled_display;  // Estrutura do display OLED

// Função para configurar os pinos de entrada e saída
void initialize_gpio() {
    gpio_init(BTN_ALT);
    gpio_init(BTN_MAIN);
    gpio_init(LED_B);
    gpio_init(LED_R);
    gpio_init(LED_G);
    adc_gpio_init(JOY_X);
    adc_gpio_init(JOY_Y);

    gpio_set_dir(BTN_ALT, GPIO_IN);
    gpio_set_dir(BTN_MAIN, GPIO_IN);
    gpio_set_dir(LED_B, GPIO_OUT);
    gpio_set_dir(LED_R, GPIO_OUT);
    gpio_set_dir(LED_G, GPIO_OUT);

    gpio_pull_up(BTN_ALT);  // Ativa resistor pull-up no botão
    gpio_pull_up(BTN_MAIN);  // Ativa resistor pull-up no botão
}

// Configuração do display OLED via comunicação I2C
void configure_display() {
    i2c_init(I2C_BUS, 400 * 1000);  // Inicializa I2C com clock de 400kHz
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    ssd1306_init(&oled_display, WIDTH, HEIGHT, false, OLED_ADDR, I2C_BUS);
    ssd1306_config(&oled_display);
    ssd1306_fill(&oled_display, false);
    ssd1306_draw_string(&oled_display, "*", 28, 60);
    ssd1306_send_data(&oled_display);
}

// Configuração dos pinos PWM para controle dos LEDs
void setup_pwm_control() {
    gpio_set_function(LED_B, GPIO_FUNC_PWM);
    uint blue_slice = pwm_gpio_to_slice_num(LED_B);
    pwm_set_clkdiv(blue_slice, PWM_CLOCK_DIV);
    pwm_set_wrap(blue_slice, PWM_LIMIT);

    gpio_set_function(LED_R, GPIO_FUNC_PWM);
    uint red_slice = pwm_gpio_to_slice_num(LED_R);
    pwm_set_clkdiv(red_slice, PWM_CLOCK_DIV);
    pwm_set_wrap(red_slice, PWM_LIMIT);

    pwm_set_enabled(blue_slice, true);
    pwm_set_enabled(red_slice, true);
}

// Função chamada quando um botão é pressionado
void button_interrupt_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if (current_time - prev_interrupt_time > DEBOUNCE_DELAY) {  // Verifica debounce
        prev_interrupt_time = current_time;

        if (gpio == BTN_MAIN) {  // Se o botão principal for pressionado
            state_led_g = !state_led_g;  // Alterna estado do LED verde
            gpio_put(LED_G, state_led_g);
            display_inverted = !display_inverted;  // Alterna cores do display
        } else if (gpio == BTN_ALT) {  // Se o botão adicional for pressionado
            pwm_active = !pwm_active;  // Ativa ou desativa os LEDs PWM
        }
    }
}

// Função principal do programa
int main() {
    stdio_init_all();  // Inicializa comunicação serial
    adc_init();  // Inicializa leitura do joystick
    initialize_gpio();  // Configura os pinos
    configure_display();  // Inicializa o display OLED
    setup_pwm_control();  // Configura os LEDs PWM

    // Configura interrupções dos botões
    gpio_set_irq_enabled_with_callback(BTN_MAIN, GPIO_IRQ_EDGE_FALL, true, &button_interrupt_handler);
    gpio_set_irq_enabled_with_callback(BTN_ALT, GPIO_IRQ_EDGE_FALL, true, &button_interrupt_handler);

    while (1) {
        // Leitura dos valores do joystick
        adc_select_input(0);
        uint16_t joy_y_value = adc_read();
        adc_select_input(1);
        uint16_t joy_x_value = adc_read();

        // Converte a posição para coordenadas do display
        float pos_x = 4 + joy_x_value * 112 / 4095;
        float pos_y = 52 - joy_y_value * 48 / 4095;

        // Atualiza a tela do display
        ssd1306_fill(&oled_display, !display_inverted);
        ssd1306_rect(&oled_display, 3, 3, 122, 58, display_inverted, !display_inverted);
        ssd1306_draw_string(&oled_display, "*", pos_x, pos_y);
        ssd1306_send_data(&oled_display);

        // Calcula valores PWM para os LEDs
        pwm_value_x = PWM_LIMIT - joy_x_value * 50000 / 4095;
        pwm_value_y = PWM_LIMIT - joy_y_value * 50000 / 4095;

        if (fabs(pwm_value_x) < 1000) pwm_value_x = 0;
        if (fabs(pwm_value_y) < 1000) pwm_value_y = 0;

        uint blue_pwm_slice = pwm_gpio_to_slice_num(LED_B);
        uint red_pwm_slice = pwm_gpio_to_slice_num(LED_R);

        pwm_set_enabled(blue_pwm_slice, pwm_active);
        pwm_set_enabled(red_pwm_slice, pwm_active);

        pwm_set_gpio_level(LED_R, fabs(pwm_value_x));
        pwm_set_gpio_level(LED_B, fabs(pwm_value_y));
    }
}