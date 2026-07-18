#include "safety.h"

#include <stddef.h>

SafetyController Safety_ControllerInit(void)
{
    SafetyController controller = { APP_STATE_INIT, false };

    return controller;
}

bool Safety_StateIsUnsafe(AppState state)
{
    return state != APP_STATE_RUN;
}

AppState Safety_Advance(SafetyController *controller, const SafetyInput *input)
{
    if ((controller == NULL) || (input == NULL)) {
        return APP_STATE_FAULT;
    }
    if (input->faultPresent) {
        controller->state = APP_STATE_FAULT;
        controller->latched = true;
        return controller->state;
    }
    if (input->keyPressed) {
        controller->state = APP_STATE_ESTOP;
        controller->latched = true;
        return controller->state;
    }

    switch (controller->state) {
    case APP_STATE_INIT:
        controller->state = APP_STATE_READY;
        break;
    case APP_STATE_READY:
        if (input->commandUsable && input->commandRequestsRun) {
            controller->state = APP_STATE_RUN;
        }
        break;
    case APP_STATE_RUN:
        if (!input->commandUsable || !input->commandRequestsRun) {
            controller->state = APP_STATE_READY;
        }
        break;
    case APP_STATE_ESTOP:
    case APP_STATE_FAULT:
        if (input->clearRequest && input->commandIsZero) {
            controller->state = APP_STATE_READY;
            controller->latched = false;
        }
        break;
    default:
        controller->state = APP_STATE_FAULT;
        controller->latched = true;
        break;
    }
    return controller->state;
}
