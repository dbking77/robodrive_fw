#include "motor_drv8871.h"
#include "robodrive_mcpwm.h"
#include "gpio.h"

// Drivers
#include "driver/gpio.h"
#include "driver/mcpwm_prelude.h"

// ESP
#include "esp_log.h"
#include "esp_check.h"
#define MCPWM_TAG "PWM"
#define MOTOR_Drv8871_TAG "MOTOR"


void Motor_Drv8871_Stop(struct Motor_Drv8871* motor);
void Motor_Drv8871_SetDuty(struct MCPWM_ChannelPair* channel_pair, float duty_ratio);


void Motor_Drv8871_Init(struct Motor_Drv8871* motor)
{
    struct Motor_Drv8871 init_motor =
    {
        .state = Motor_Drv8871_RESET,
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
    MCPWM_Servo_Init(&motor->weapon_servo);
    ESP_LOGE(MOTOR_Drv8871_TAG, "Motor Init Complete");
}

void Motor_Drv8871_Update(struct Motor_Drv8871* motor, bool enable, float duty_ratio_a, float duty_ratio_b, bool weapon_enable, float weapon_duty_ratio)
{
    switch (motor->state)
    {
        case Motor_Drv8871_RESET:
            motor->state = Motor_Drv8871_ENABLING;
            motor->count = 0;
            break;
        case Motor_Drv8871_ENABLING:
            if (++motor->count > 10)
            {
                motor->state = Motor_Drv8871_RUNNING;
                motor->count = 0;
            }
            break;
        case Motor_Drv8871_RUNNING:
            break;
        default:
             ESP_LOGI(MOTOR_Drv8871_TAG, "Bad State %d", motor->state);
             motor->state = Motor_Drv8871_RESET;
             motor->count = 0;
             Motor_Drv8871_Stop(motor);
    }
    ESP_LOGI(MOTOR_Drv8871_TAG, "Motor State %d", motor->state);

    if (motor->state == Motor_Drv8871_RUNNING)
    {
        Motor_Drv8871_SetDuty(&motor->channel_pair_a, duty_ratio_a);
        Motor_Drv8871_SetDuty(&motor->channel_pair_b, duty_ratio_b);
    }
    else
    {
        Motor_Drv8871_Stop(motor);
    }

    bool weapon_servo_enable = weapon_enable && (motor->state == Motor_Drv8871_RUNNING);
    MCPWM_setServoDuty(&motor->weapon_servo, weapon_servo_enable, weapon_duty_ratio);
}

void Motor_Drv8871_SetDuty(struct MCPWM_ChannelPair* channel_pair, float duty_ratio)
{
    float duty_ratio_ch1 = 1.0f;
    float duty_ratio_ch2 = 1.0f;
    if (duty_ratio >= 0.0f)
    {
        duty_ratio_ch1 = 1.0f - duty_ratio;
    }
    else
    {
        duty_ratio_ch2 = 1.0f + duty_ratio;
    }
    MCPWM_SetDuty(channel_pair, duty_ratio_ch1, duty_ratio_ch2);
}

void Motor_Drv8871_Stop(struct Motor_Drv8871* motor)
{
    MCPWM_SetDuty(&motor->channel_pair_a, 1.0f, 1.0f);
    MCPWM_SetDuty(&motor->channel_pair_b, 1.0f, 1.0f);
}

