#include <stdio.h>

// ESP
#include "esp_log.h"
#include "esp_check.h"
#define ROBODRIVE_TAG "ROBO"


// FreeRTOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "gpio.h"
#include "motor_drv8424.h"
#include "motor_drv8871.h"
#include "analog.h"
#include "gatt_server.h"

#include "driver/gpio.h"

#if ROBODRIVE_BOARD_REV == 'A'
    struct Motor_Drv8424 g_motor;
    #define Motor_Init Motor_Drv8424_Init
    #define Motor_Update Motor_Drv8424_Update
#elif ROBODRIVE_BOARD_REV == 'B'
    struct Motor_Drv8871 g_motor;
    #define Motor_Init Motor_Drv8871_Init
    #define Motor_Update Motor_Drv8871_Update
#else
    #error "Only Support ROBODRIVE_BOARD_REV A or B"
#endif

void app_main(void)
{
    ESP_LOGW(ROBODRIVE_TAG, "Robodrive board rev %c", ROBODRIVE_BOARD_REV);
    GPIO_Init();
    AdcContinuous_Init();
    void AdcContinuous_Read();
    float AdcContinuous_GetBatteryVoltage();    
    Motor_Init(&g_motor);
    GattServer_Init();

    esp_log_level_set("*", ESP_LOG_ERROR);

    uint32_t led_flash_counter = 0;
    uint32_t adc_count = 0;

    const float fwd_full_scale = 1.0f;
    const float turn_full_scale = 1.0f;
    const float weapon_full_scale = 1.0f;

    const float fwd_scale = 0.01f * fwd_full_scale;
    const float turn_scale = 0.01f * turn_full_scale;
    const float weapon_scale = 0.01f * weapon_full_scale;

    float battery_voltage = 0.0f;

    int fake_battery_voltage_percent = 0;

    struct robodrive_command cmd;
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(10));

        float duty_ratio_a = 0.0f;
        float duty_ratio_b = 0.0f;
        float duty_ratio_weapon = 0.0f;
        bool weapon_enable = false;
        if (GattServer_GetCommand(&cmd))
        {
            //ESP_LOGE("CMD", "fwd %d right %d weapon %d", (int)cmd.r_joy_fwd, (int)cmd.r_joy_right, (int)cmd.l_joy_fwd);
            float fwd = cmd.r_joy_fwd * fwd_scale;
            float right = cmd.r_joy_right * turn_scale;
            duty_ratio_a = (fwd - right);
            duty_ratio_b = (fwd + right);
            weapon_enable = true;
            duty_ratio_weapon = cmd.l_joy_fwd * weapon_scale;
            //ESP_LOGE("CMD", "duty_a %f duty_b %f weapon %f", duty_ratio_a, duty_ratio_b, duty_ratio_weapon);
        }

        Motor_Update(&g_motor, true, duty_ratio_a, duty_ratio_b, weapon_enable, duty_ratio_weapon);
        AdcContinuous_Read();

        ++led_flash_counter;
        ESP_ERROR_CHECK(gpio_set_level(INTERNAL_LED_GPIO, GattServer_IsConnected() ? 0 : (led_flash_counter >> 5) & 1));
        #ifdef EXTERNAL_LEG_GPIO
            ESP_ERROR_CHECK(gpio_set_level(EXTERNAL_LED_GPIO, GattServer_IsConnected() ? 0 : (led_flash_counter >> 5) & 1));
        #endif

        ++adc_count;
        if (adc_count >= 100)
        {
            adc_count = 0;
            //float raw = AdcContinuous_GetBatteryAvgRaw();
            battery_voltage = AdcContinuous_GetBatteryVoltage();
            const float full_battery_voltage = 12.6f;
            const float empty_battery_voltage = 9.0f;
            float battery_percent = 100.0f * (battery_voltage - empty_battery_voltage) / (full_battery_voltage - empty_battery_voltage);
            //int battery_voltage_decimal = battery_voltage * 10.0f;
            //ESP_LOGE("VBAT", "battery_voltage %f (%d): %.1f %%", battery_voltage, battery_voltage_decimal, battery_percent);
            //ESP_LOGE("VBAT", "battery_raw %f", raw);

            ++fake_battery_voltage_percent;
            if (fake_battery_voltage_percent > 100)
            {
                fake_battery_voltage_percent = 0;
            }
            GattServer_SetBatteryPercent(battery_percent);
            // 12 -  1.81 - 1812    
            // 11 -  1.661 - 1658
            // 10 -  1.5  - 1506
            // 9  -  1.36 - 1355 
        }
    }
}
