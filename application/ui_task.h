#ifndef UI_TASK_H
#define UI_TASK_H

#include <stdbool.h>

#define UI_TASK_PERIOD_MS (100U)

bool UiTask_Init(void);
const char *UiTask_GetLastFrame(void);

#endif
