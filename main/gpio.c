#include "gpio.h"

// Drivers
#include "driver/gpio.h"

// ESP
#include "esp_log.h"
#include "esp_check.h"
#define GPIO_TAG "GPIO"

void GPIO_Init()
{
    // Enable RF switch and select chip-antenna (not external antenna)
    ESP_ERROR_CHECK(gpio_set_level(WIFI_ENABLE_GPIO, 0));
    ESP_ERROR_CHECK(gpio_set_direction(WIFI_ENABLE_GPIO, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_level(WIFI_ANT_CONFIG_GPIO, 0));
    ESP_ERROR_CHECK(gpio_set_direction(WIFI_ANT_CONFIG_GPIO, GPIO_MODE_OUTPUT));

    ESP_ERROR_CHECK(gpio_set_level(WEAPON_SERVO_GPIO, 0));
    ESP_ERROR_CHECK(gpio_set_direction(WEAPON_SERVO_GPIO, GPIO_MODE_OUTPUT));

    ESP_ERROR_CHECK(gpio_set_level(INTERNAL_LED_GPIO, 1));
    ESP_ERROR_CHECK(gpio_set_direction(INTERNAL_LED_GPIO, GPIO_MODE_OUTPUT));

    #ifdef EXTERNAL_LED_GPIO
        ESP_ERROR_CHECK(gpio_set_level(EXTERNAL_LED_GPIO, 1));
        ESP_ERROR_CHECK(gpio_set_direction(EXTERNAL_LED_GPIO, GPIO_MODE_OUTPUT));
    #endif

    ESP_LOGE(GPIO_TAG, "GPIO Setup Complete");
}