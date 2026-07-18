#ifndef SAFETY_H
#define SAFETY_H

#include <stdbool.h>

#include "app_types.h"

typedef struct {
    bool keyPressed;
    bool faultPresent;
    bool commandUsable;
    bool commandRequestsRun;
    bool clearRequest;
    bool commandIsZero;
} SafetyInput;

typedef struct {
    AppState state;
    bool latched;
} SafetyController;

SafetyController Safety_ControllerInit(void);
AppState Safety_Advance(SafetyController *controller, const SafetyInput *input);
bool Safety_StateIsUnsafe(AppState state);

#endif
