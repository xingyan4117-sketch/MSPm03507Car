#ifndef TASK5_HOST_SUPPORT_H
#define TASK5_HOST_SUPPORT_H

#include <stdint.h>

#include "app_types.h"

void Task5Host_Reset(void);
const AppMotorStatus *Task5Host_GetPublishedStatus(void);
const AppMotorStatus *Task5Host_GetPublishedSafetyStatus(void);
uint32_t Task5Host_GetStopCalls(void);
uint32_t Task5Host_StopEventOrder(void);
uint32_t Task5Host_PublishEventOrder(void);
uint32_t Task5Host_SafetyPublishEventOrder(void);

#endif
