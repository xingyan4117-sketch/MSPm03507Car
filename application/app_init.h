#ifndef APP_INIT_H
#define APP_INIT_H

#include <stdbool.h>

#include "app_types.h"

bool App_Init(void);
AppMotorCommand App_DefaultCommand(void);
bool App_PublishCommand(const AppMotorCommand *command);
bool App_ReadLatestCommand(AppMotorCommand *command);
bool App_ReadLatestSafetyCommand(AppMotorCommand *command);
bool App_PublishMotorStatus(const AppMotorStatus *status);
bool App_ReadLatestMotorStatus(AppMotorStatus *status);
bool App_PublishSafetyStatus(const AppMotorStatus *status);
bool App_ReadLatestSafetyStatus(AppMotorStatus *status);
bool App_PublishStatus(const AppMotorStatus *status);
bool App_ReadLatestStatus(AppMotorStatus *status);

#endif
