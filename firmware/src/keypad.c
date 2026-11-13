/*
 * AIC Pico Keypad (Modificado para Matriz 3x4 + Pulso Único + Antirrebote)
 * WHowe <github.com/whowechina>
 */

#include "keypad.h"

#include <stdint.h>
#include <stdbool.h>

#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/pwm.h"

#include "board_defs.h"

static const uint8_t keypad_rows[] = KEYPAD_ROW_PINS;
static const uint8_t keypad_cols[] = KEYPAD_COL_PINS;

#define ROW_NUM (sizeof(keypad_rows))
#define COL_NUM (sizeof(keypad_cols))
#define KEY_NUM (ROW_NUM * COL_NUM)

// Mapeo de índice físico (teléfono) al índice lógico que espera el firmware
static const uint8_t phys_to_log[12] = {
    0,  1,  2,   // Fila 1 física: 1, 2, 3
    3,  4,  5,   // Fila 2 física: 4, 5, 6
    6,  7,  8,   // Fila 3 física: 7, 8, 9
    10, 9,  11   // Fila 4 física: *, 0, #
};

static bool sw_val[KEY_NUM]; /* true if pressed */
static uint64_t sw_freeze_time[KEY_NUM];
static bool sw_fired[KEY_NUM];         /* Controla si la tecla ya se disparó */
static uint64_t sw_pulse_end[KEY_NUM]; /* Controla la duración del pulso */
static bool keystuck = false;

void keypad_init()
{
    // Inicializar filas como entradas con resistencia pull-up interna
    for (int i = 0; i < ROW_NUM; i++) {
        uint8_t gpio = keypad_rows[i];
        gpio_init(gpio);
        gpio_set_function(gpio, GPIO_FUNC_SIO);
        gpio_set_dir(gpio, GPIO_IN);
        gpio_pull_up(gpio);
    }

    // Inicializar columnas como salidas en estado ALTO (inactivas)
    for (int i = 0; i < COL_NUM; i++) {
        uint8_t gpio = keypad_cols[i];
        gpio_init(gpio);
        gpio_set_function(gpio, GPIO_FUNC_SIO);
        gpio_set_dir(gpio, GPIO_OUT);
        gpio_put(gpio, 1);
    }

    for (int i = 0; i < KEY_NUM; i++) {
        sw_val[i] = false;
        sw_freeze_time[i] = 0;
        sw_fired[i] = false;
        sw_pulse_end[i] = 0;
    }

    // Comprobación de teclado atascado al inicio
    for (int c = 0; c < COL_NUM; c++) {
        gpio_put(keypad_cols[c], 0);
    }
    
    busy_wait_us(10);
    
    for (int r = 0; r < ROW_NUM; r++) {
        if (gpio_get(keypad_rows[r]) == 0) {
            keystuck = true;
            break;
        }
    }
    
    for (int c = 0; c < COL_NUM; c++) {
        gpio_put(keypad_cols[c], 1);
    }
}

uint8_t keypad_key_num()
{
    return KEY_NUM;
}

bool keypad_is_stuck()
{
    return keystuck;
}

static uint16_t keypad_reading;

#define DEBOUNCE_FREEZE_TIME_US 20000

void keypad_update()
{
    if (keystuck) {
        return;
    }

    uint64_t now = time_us_64();
    uint16_t keys = 0;
    bool current_state[KEY_NUM] = {false};

    // 1. Fase de Escaneo (Multiplexación)
    for (int c = 0; c < COL_NUM; c++) {
        gpio_put(keypad_cols[c], 0);
        busy_wait_us(2); 
        for (int r = 0; r < ROW_NUM; r++) {
            int index = r * COL_NUM + c;
            current_state[index] = !gpio_get(keypad_rows[r]);
        }
        gpio_put(keypad_cols[c], 1);
    }

    // 2. Fase de Debounce
    for (int i = 0; i < KEY_NUM; i++) {
        bool sw_pressed = current_state[i];
        
        if (now >= sw_freeze_time[i]) {
            if (sw_pressed != sw_val[i]) {
                sw_val[i] = sw_pressed;
                sw_freeze_time[i] = now + DEBOUNCE_FREEZE_TIME_US;
            }
        }
    }

    // 3. Empaquetado de bits con Traducción, Pulso Único (50ms) y Cooldown (100ms extra)
    for (int i = 0; i < KEY_NUM; i++) {
        if (sw_val[i]) {
            if (!sw_fired[i]) {
                sw_pulse_end[i] = now + 50000;
                sw_fired[i] = true;
            }
            
            if (now < sw_pulse_end[i]) {
                keys |= (1 << phys_to_log[i]);
            }
        } else {
            if (now > (sw_pulse_end[i] + 100000)) {
                sw_fired[i] = false;
            }
        }
    }

    keypad_reading = keys;
}

uint16_t keypad_read()
{
    return keypad_reading;
}