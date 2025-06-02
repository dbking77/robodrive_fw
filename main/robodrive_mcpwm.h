#pragma once

#include "driver/mcpwm_timer.h"

struct MCPWM_Channel
{
    mcpwm_gen_handle_t generator;
    mcpwm_cmpr_handle_t comparator;
    int gpio;
};

struct MCPWM_ChannelPair
{
    int group_id;
    char name;
    struct MCPWM_Channel ch1;
    struct MCPWM_Channel ch2;
    mcpwm_timer_handle_t timer;
    mcpwm_oper_handle_t operator;
};

struct MCPWM_Servo
{
    int group_id;
    char name;
    int gpio;
    mcpwm_timer_handle_t timer;
    mcpwm_oper_handle_t operator;
    mcpwm_gen_handle_t generator;
    mcpwm_cmpr_handle_t comparator;
};

void MCPWM_Init();
void MCPWM_Channel_Init(struct MCPWM_Channel* channel, struct MCPWM_ChannelPair* parent);
void MCPWM_Servo_Init(struct MCPWM_Servo* servo);
void MCPWM_ChannelPair_Init(struct MCPWM_ChannelPair* channel_pair);
void MCPWM_SetDuty(struct MCPWM_ChannelPair* channel_pair, float duty_ratio_ch1, float duty_ratio_ch2);
void MCPWM_Stop(struct MCPWM_ChannelPair* channel_pair);

void MCPWM_setServoDuty(struct MCPWM_Servo* servo, bool servo_enable, float servo_duty_ratio);
