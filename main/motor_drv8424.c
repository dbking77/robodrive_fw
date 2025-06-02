#include "motor_drv8424.h"
#include "robodrive_mcpwm.h"
#include "gpio.h"

// Drivers
#include "driver/gpio.h"
#include "driver/mcpwm_prelude.h"

// ESP
#include "esp_log.h"
#include "esp_check.h"
#define MCPWM_TAG "PWM"
#define MOTOR_DRV8424_TAG "MOTOR"

void Motor_Drv8424_Stop(struct Motor_Drv8424* motor);
void Motor_Drv8424_SetDuty(struct MCPWM_ChannelPair* channel_pair, float duty_ratio);

void Motor_Drv8424_Init(struct Motor_Drv8424* motor)
{
    // Put motor driver to sleep at start
    ESP_ERROR_CHECK(gpio_set_level(NSLEEP_GPIO, 0));
    ESP_ERROR_CHECK(gpio_set_direction(NSLEEP_GPIO, GPIO_MODE_OUTPUT));

    // Fault pin is an input
    ESP_ERROR_CHECK(gpio_set_direction(NFAULT_GPIO, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_input_enable(NFAULT_GPIO));
    ESP_ERROR_CHECK(gpio_set_pull_mode(NFAULT_GPIO, GPIO_PULLUP_ONLY));

    struct Motor_Drv8424 init_motor =
    {
        .state = Motor_Drv8424_RESET,
        .count = 0,
        .channel_pair_a = {
            .group_id = 0,
            .name = 'A',
            .ch1 = {.gpio = MCPWM_A1_GPIO},
            .ch2 = {.gpio = MCPWM_A2_GPIO}
        },
        .channel_pair_b = {
            .group_id = 0,
            .name = 'B',
            .ch1 = {.gpio = MCPWM_B1_GPIO},
            .ch2 = {.gpio = MCPWM_B2_GPIO}
        },
        .weapon_servo = {
            .group_id = 0,
            .name = 'W',
            .gpio = WEAPON_SERVO_GPIO
        }
    };
    *motor = init_motor;

    MCPWM_ChannelPair_Init(&motor->channel_pair_a);
    MCPWM_ChannelPair_Init(&motor->channel_pair_b);
    Motor_Drv8424_Stop(motor);
    MCPWM_Servo_Init(&motor->weapon_servo);
    ESP_LOGE(MOTOR_DRV8424_TAG, "Motor Init Complete");
}

void Motor_Drv8424_Update(struct Motor_Drv8424* motor, bool enable, float duty_ratio_a, float duty_ratio_b, bool weapon_enable, float weapon_duty_ratio)
{
    // Stay in reset state
    if (!enable && (motor->state != Motor_Drv8424_ERROR))
    {
        motor->state = Motor_Drv8424_RESET;
    }

    switch (motor->state)
    {
        case Motor_Drv8424_RESET:
            ESP_ERROR_CHECK(gpio_set_level(NSLEEP_GPIO, 0));
            motor->state = Motor_Drv8424_ENABLING;
            break;
        case Motor_Drv8424_ENABLING:
            ESP_ERROR_CHECK(gpio_set_level(NSLEEP_GPIO, 1));
            motor->state = Motor_Drv8424_RUNNING;
            motor->count = 0;
            break;
        case Motor_Drv8424_RUNNING:
            ++motor->count;
            int nfault = gpio_get_level(NFAULT_GPIO);
            if (nfault == 0)
            {
                ESP_LOGI(MOTOR_DRV8424_TAG, "NFault level %d after", nfault);
                motor->state = Motor_Drv8424_ERROR;
                motor->count = 0;
                Motor_Drv8424_Stop(motor);
            }
            break;
        case Motor_Drv8424_ERROR:
            if (++motor->count > 100)
            {
                ESP_LOGI(MOTOR_DRV8424_TAG, "Clear error after %d cycles", motor->count);
                motor->state = Motor_Drv8424_RESET;
            }
            break;
        default:
             ESP_LOGI(MOTOR_DRV8424_TAG, "Bad State %d", motor->state);
             motor->state = Motor_Drv8424_ERROR;
             motor->count = 0;
             Motor_Drv8424_Stop(motor);
    }
    //ESP_LOGI(MOTOR_DRV8424_TAG, "Motor State %d", motor->state);

    if (motor->state == Motor_Drv8424_RUNNING)
    {
        Motor_Drv8424_SetDuty(&motor->channel_pair_a, duty_ratio_a);
        Motor_Drv8424_SetDuty(&motor->channel_pair_b, duty_ratio_b);
    }
    else
    {
        Motor_Drv8424_Stop(motor);
    }

    bool weapon_servo_enable = weapon_enable && (motor->state == Motor_Drv8424_RUNNING);
    MCPWM_setServoDuty(&motor->weapon_servo, weapon_servo_enable, weapon_duty_ratio);
}

void Motor_Drv8424_SetDuty(struct MCPWM_ChannelPair* channel_pair, float duty_ratio)
{
    MCPWM_SetDuty(channel_pair, 0.5f - 0.5f * duty_ratio, 0.5f + 0.5f * duty_ratio);
}

void Motor_Drv8424_Stop(struct Motor_Drv8424* motor)
{
    MCPWM_SetDuty(&motor->channel_pair_a, 0.0f, 0.0f);
    MCPWM_SetDuty(&motor->channel_pair_b, 0.0f, 0.0f);
}