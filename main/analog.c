#include "analog.h"

#include "robodrive_config.h"

#include "esp_check.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "esp_adc/adc_continuous.h"

#define ADC_TAG "ADC"

#if ROBODRIVE_BOARD_REV == 'A'
    #define V_BATT_ADC_CH ADC_CHANNEL_0
#elif ROBODRIVE_BOARD_REV == 'B'
    #define V_BATT_ADC_CH ADC_CHANNEL_2
#else
    #error "Only Support ROBODRIVE_BOARD_REV A or B"
#endif

#define ADC_CHANNEL_COUNT 1
#define ADC_READ_LEN (ADC_CHANNEL_COUNT * SOC_ADC_DIGI_DATA_BYTES_PER_CONV * 4)

static adc_continuous_handle_t adc_handle = NULL;

struct analog_ch_avg
{
    uint32_t sum;
    uint32_t count;
};

static struct analog_ch_avg analog_ch_avgs[ADC_CHANNEL_COUNT];


void AdcContinuous_Init()
{
    //ESP_LOGI(ADC_TAG, "SOC_ADC_DIGI_DATA_BYTES_PER_CONV %d", SOC_ADC_DIGI_DATA_BYTES_PER_CONV);

    adc_continuous_handle_t handle = NULL;

    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = ADC_READ_LEN*4,
        .conv_frame_size = ADC_READ_LEN,
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &handle));

    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = 1000,
        .conv_mode = ADC_CONV_SINGLE_UNIT_1,
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE2,
    };

    adc_channel_t channels[ADC_CHANNEL_COUNT] = {
        V_BATT_ADC_CH,
    };

    adc_digi_pattern_config_t adc_pattern[ADC_CHANNEL_COUNT] = {0};
    dig_cfg.pattern_num = ADC_CHANNEL_COUNT;
    for (int i = 0; i < ADC_CHANNEL_COUNT; i++) {
        adc_pattern[i].atten = ADC_ATTEN_DB_12;
        adc_pattern[i].channel = channels[i] & 0x7;
        adc_pattern[i].unit = ADC_UNIT_1;
        adc_pattern[i].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH; //ADC_BITWIDTH_DEFAULT; //SOC_ADC_DIGI_MAX_BITWIDTH;

        ESP_LOGI(ADC_TAG, "adc_pattern[%d].atten is :%"PRIx8, i, adc_pattern[i].atten);
        ESP_LOGI(ADC_TAG, "adc_pattern[%d].channel is :%"PRIx8, i, adc_pattern[i].channel);
        ESP_LOGI(ADC_TAG, "adc_pattern[%d].unit is :%"PRIx8, i, adc_pattern[i].unit);
    }
    dig_cfg.adc_pattern = adc_pattern;
    ESP_ERROR_CHECK(adc_continuous_config(handle, &dig_cfg));

    ESP_ERROR_CHECK(adc_continuous_start(handle));

    adc_handle = handle;
}

void AdcContinuous_Read()
{
    uint8_t adc_result_buf[ADC_READ_LEN];
    uint32_t actual_read_len = 0;
    uint32_t timeout_ms = 0;
    //uint64_t start_us = esp_timer_get_time();
    esp_err_t ret = adc_continuous_read(adc_handle, adc_result_buf, ADC_READ_LEN, &actual_read_len, timeout_ms);
    if (ret != ESP_OK)
    {
        return;
    }
    //uint64_t stop_us = esp_timer_get_time();
    //ESP_LOGI(ADC_TAG, "Read %u bytes in %u usec", (unsigned)actual_read_len, (unsigned)(stop_us-start_us));
    for (int offset = 0; offset < actual_read_len; offset += SOC_ADC_DIGI_RESULT_BYTES) {
        adc_digi_output_data_t *sample = (adc_digi_output_data_t*)&adc_result_buf[offset];
        uint32_t channel = sample->type2.channel;
        uint32_t value = sample->type2.data;
        struct analog_ch_avg* ch_avg = NULL;
        switch (channel)
        {
        case V_BATT_ADC_CH:
            ch_avg = &analog_ch_avgs[0];
            break;
        }
        if (ch_avg != NULL)
        {
            ++ch_avg->count;
            ch_avg->sum += value;
        }
    }
}

float getAverage(struct analog_ch_avg* ch_avg)
{
    float avg = ch_avg->count ? ((float)ch_avg->sum) / ch_avg->count : 0.0;
    return avg;
}

float AdcContinuous_GetBatteryAvgRaw()
{
    struct analog_ch_avg* ch_avg = &analog_ch_avgs[0];
    float avg = ch_avg->count ? ((float)ch_avg->sum) / ch_avg->count : 0.0;
    return avg;
}

float AdcContinuous_GetBatteryVoltage()
{
    struct analog_ch_avg* ch_avg = &analog_ch_avgs[0];
    float avg = ch_avg->count ? ((float)ch_avg->sum) / ch_avg->count : 0.0;
    ch_avg->count = 0;
    ch_avg->sum = 0;
    // (56.2k + 10k) / 10k
    // with 12db attenuation = 3.98
    // Manual calibration 12.0V = raw value of 1812
    //float voltage = avg * (1.1vref * 3.98 atten / 4098.0f * 6.62f);
    float voltage = avg * (12.0f / 1812.0f);
    return voltage;
}