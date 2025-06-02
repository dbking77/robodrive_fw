#pragma once

#include "robodrive_mcpwm.h"

enum
{
    Motor_Drv8424_RESET = 0,
    Motor_Drv8424_ENABLING = 1,
    Motor_Drv8424_RUNNING = 2,
    Motor_Drv8424_ERROR = 3,
};

struct Motor_Drv8424
{
    int state;
    int count;
    struct MCPWM_ChannelPair channel_pair_b;
    struct MCPWM_ChannelPair channel_pair_a;
    struct MCPWM_Servo weapon_servo;
};

void Motor_Drv8424_Init(struct Motor_Drv8424* motor);
void Motor_Drv8424_Update(struct Motor_Drv8424* motor, bool enable, float duty_ratio_a, float duty_ratio_b, bool weapon_enable, float weapon_duty_ratio);
