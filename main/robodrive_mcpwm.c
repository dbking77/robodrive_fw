#include "robodrive_mcpwm.h"

// Drivers
#include "driver/gpio.h"
#include "driver/mcpwm_prelude.h"

// ESP
#include "esp_log.h"
#include "esp_check.h"
#define MCPWM_TAG "PWM"

#define MCPWM_TIMER_RESOLUTION_HZ 10000000  // 10MHz, 0.1us per tick
#define MCPWM_TIMER_PERIOD        1000      // 1000 ticks, 0.1ms = 10kHz

#define MCPWM_SERVO_RESOLUTION_HZ 1000000  // 1MHz, 1us per tick
#define MCPWM_SERVO_PERIOD        20000    // 20000 ticks = 20ms
#define MCPWM_MAX_PULSE_PERIOD    2000    // 2000 ticks = 2ms
#define MCPWM_MIN_PULSE_PERIOD    1000    // 1000 ticks = 1ms
#define MCPWM_OFF_PULSE_PERIOD    0    // 1000 ticks = 0ms (no pulse)

void MCPWM_Channel_Init(struct MCPWM_Channel* channel, struct MCPWM_ChannelPair* parent)
{
    ESP_LOGI(MCPWM_TAG, "Create comparator operator %c", parent->name);
    mcpwm_comparator_config_t comparator_config = {
        .flags.update_cmp_on_tez = true,
    };
    ESP_ERROR_CHECK(mcpwm_new_comparator(parent->operator, &comparator_config, &channel->comparator));

            // set the initial compare value, so that the servo will spin to the center position
    ESP_LOGI(MCPWM_TAG, "Have generator control GPIO%d", channel->gpio);
    mcpwm_generator_config_t generator_config = {
        .gen_gpio_num = channel->gpio,
    };
    ESP_ERROR_CHECK(mcpwm_new_generator(parent->operator, &generator_config, &channel->generator));

    // set the initial compare value, so that the servo will spin to the center position
    int compare_value = 0;
    ESP_LOGI(MCPWM_TAG, "Set generator %c action on timer and compare value %d", parent->name, compare_value);
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(channel->comparator, compare_value));

    // go high on counter empty
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(
        channel->generator, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, channel->comparator, MCPWM_GEN_ACTION_HIGH)));

    // go low on compare threshold
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(
        channel->generator, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, channel->comparator, MCPWM_GEN_ACTION_LOW)));
}

void MCPWM_ChannelPair_Init(struct MCPWM_ChannelPair* channel_pair)
{
    mcpwm_timer_config_t timer_config = {
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .group_id = channel_pair->group_id,
        .resolution_hz = MCPWM_TIMER_RESOLUTION_HZ,
        .period_ticks = MCPWM_TIMER_PERIOD,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP_DOWN,
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &channel_pair->timer));

    channel_pair->operator = NULL;
    mcpwm_operator_config_t operator_config = {
        .group_id = channel_pair->group_id,
    };
    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &channel_pair->operator));

    ESP_LOGI(MCPWM_TAG, "Connect timer and operator %c", channel_pair->name);
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(channel_pair->operator, channel_pair->timer));

    MCPWM_Channel_Init(&channel_pair->ch1, channel_pair);
    MCPWM_Channel_Init(&channel_pair->ch2, channel_pair);

    ESP_LOGI(MCPWM_TAG, "Enable and start timer %c", channel_pair->name);
    ESP_ERROR_CHECK(mcpwm_timer_enable(channel_pair->timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(channel_pair->timer, MCPWM_TIMER_START_NO_STOP));
}

int MCPWM_ComputeCompareValueFromDutyRatio(float duty_ratio)
{
    int max_duty = MCPWM_TIMER_PERIOD / 2;
    int duty = duty_ratio * max_duty;
    if (duty > max_duty)
    {
        duty = max_duty;
    }
    else if (duty < 0)
    {
        duty = 0;
    }
    return duty;
}

void MCPWM_SetDuty(struct MCPWM_ChannelPair* channel_pair, float duty_ratio_ch1, float duty_ratio_ch2)
{
    int duty_ch1 = MCPWM_ComputeCompareValueFromDutyRatio(duty_ratio_ch1);
    int duty_ch2 = MCPWM_ComputeCompareValueFromDutyRatio(duty_ratio_ch2);
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(channel_pair->ch1.comparator, duty_ch1));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(channel_pair->ch2.comparator, duty_ch2));
}

void MCPWM_Servo_Init(struct MCPWM_Servo* servo)
{
    mcpwm_timer_config_t timer_config = {
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .group_id = servo->group_id,
        .resolution_hz = MCPWM_SERVO_RESOLUTION_HZ,
        .period_ticks = MCPWM_SERVO_PERIOD,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &servo->timer));

    servo->operator = NULL;
    mcpwm_operator_config_t operator_config = {
        .group_id = servo->group_id,
    };
    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &servo->operator));

    ESP_LOGI(MCPWM_TAG, "Connect timer and operator %c", servo->name);
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(servo->operator, servo->timer));

    ESP_LOGI(MCPWM_TAG, "Create servo PWM");
    mcpwm_comparator_config_t comparator_config = {
        .flags.update_cmp_on_tez = true,
    };
    ESP_ERROR_CHECK(mcpwm_new_comparator(servo->operator, &comparator_config, &servo->comparator));

            // set the initial compare value, so that the servo will spin to the center position
    ESP_LOGI(MCPWM_TAG, "Have generator control GPIO%d", servo->gpio);
    mcpwm_generator_config_t generator_config = {
        .gen_gpio_num = servo->gpio,
    };
    ESP_ERROR_CHECK(mcpwm_new_generator(servo->operator, &generator_config, &servo->generator));

    // set the initial compare value, so that the servo will spin to the center position
    int compare_value = 0;
    ESP_LOGI(MCPWM_TAG, "Set generator %c action on timer and compare value %d", servo->name, compare_value);
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(servo->comparator, compare_value));

    // go high on counter empty
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(
            servo->generator,  MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));

    // go low on compare threshold
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(
        servo->generator, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, servo->comparator, MCPWM_GEN_ACTION_LOW)));

    ESP_LOGI(MCPWM_TAG, "Enable and start timer %c", servo->name);
    ESP_ERROR_CHECK(mcpwm_timer_enable(servo->timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(servo->timer, MCPWM_TIMER_START_NO_STOP));
}


void MCPWM_setServoDuty(struct MCPWM_Servo* servo, bool servo_enable, float servo_duty_ratio)
{
    if (servo_enable)
    {
        int duty = servo_duty_ratio * (MCPWM_MAX_PULSE_PERIOD - MCPWM_MIN_PULSE_PERIOD) + MCPWM_MIN_PULSE_PERIOD;
        if (duty > MCPWM_MAX_PULSE_PERIOD)
        {
            duty = MCPWM_MAX_PULSE_PERIOD;
        }
        else if (duty < MCPWM_MIN_PULSE_PERIOD)
        {
            duty = MCPWM_MIN_PULSE_PERIOD;
        }
        mcpwm_comparator_set_compare_value(servo->comparator, duty);
    }
    else
    {
        mcpwm_comparator_set_compare_value(servo->comparator, MCPWM_OFF_PULSE_PERIOD);
    }
}