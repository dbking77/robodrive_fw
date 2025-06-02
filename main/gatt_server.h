#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "robodrive_comm.h"

void GattServer_Init();

bool GattServer_GetCommand(struct robodrive_command* cmd);
void GattServer_SetBatteryPercent(int percentage); 

bool GattServer_IsConnected();