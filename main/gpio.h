#pragma once

#include "robodrive_config.h"

#if ROBODRIVE_BOARD_REV == 'A'
    // EXTERNAL GPIO
    #undef EXTERNAL_LED_GPIO  // Not available
    #define VBATT_GPIO 0      // A0/D0
    #define NSLEEP_GPIO 1     // D1
    #define NFAULT_GPIO 18    // D10
    #define MCPWM_A1_GPIO 2   // D2
    #define MCPWM_A2_GPIO 21  // D3
    #define MCPWM_B1_GPIO 22  // D4
    #define MCPWM_B2_GPIO 20  // D9
    #define SERVO_PWM1 23     // D5
    #define SERVO_PWM2 19     // D8
    #define WEAPON_SERVO_GPIO SERVO_PWM1
#elif ROBODRIVE_BOARD_REV == 'B'
    // EXTERNAL GPIO
    #define EXTERNAL_LED_GPIO 0 // D0
    #define VBATT_GPIO 2       // A2/D2
    #define MCPWM_A1_GPIO 21   // D3
    #define MCPWM_A2_GPIO 22   // D4
    #define MCPWM_B1_GPIO 20   // D9
    #define MCPWM_B2_GPIO 18   // D10
    #define SERVO_PWM1 23      // D5
    #define SERVO_PWM2 19      // D8
    #define NSLEEP_GPIO -1     // Does not exist
    #define NFAULT_GPIO -1     // Does not exist
    #define WEAPON_SERVO_GPIO SERVO_PWM1
#else
    #error "Only Support ROBODRIVE_BOARD_REV A or B"
#endif

// Internal GPIO
#define WIFI_ENABLE_GPIO 3
#define WIFI_ANT_CONFIG_GPIO 14
#define INTERNAL_LED_GPIO 15


void GPIO_Init();