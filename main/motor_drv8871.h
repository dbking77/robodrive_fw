#pragma once

#include "robodrive_mcpwm.h"

enum
{
    Motor_Drv8871_RESET = 0,
    Motor_Drv8871_ENABLING = 1,
    Motor_Drv8871_RUNNING = 2,
};

struct Motor_Drv8871
{
    int state;
    int count;
    struct MCPWM_ChannelPair channel_pair_b;
    struct MCPWM_ChannelPair channel_pair_a;
    struct MCPWM_Servo weapon_servo;
};

void Motor_Drv8871_Init(struct Motor_Drv8871* motor);
void Motor_Drv8871_Update(struct Motor_Drv8871* motor, bool enable, float duty_ratio_a, float duty_ratio_b, bool weapon_enable, float weapon_duty_ratio);
