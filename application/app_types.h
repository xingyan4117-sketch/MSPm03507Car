#ifndef APP_TYPES_H
#define APP_TYPES_H

#include <stdint.h>

typedef uint8_t AppState;

enum {
    APP_BUZZER_MODE_OFF = 0U,
    APP_BUZZER_MODE_CONT = 1U,
    APP_BUZZER_MODE_SLOW = 2U,
    APP_BUZZER_MODE_FAST = 3U
};

enum {
    APP_BUZZER_VOLUME_MIN = 0U,
    APP_BUZZER_VOLUME_MAX = 100U
};

enum {
    APP_STATE_INIT = 0U,
    APP_STATE_READY = 1U,
    APP_STATE_RUN = 2U,
    APP_STATE_ESTOP = 3U,
    APP_STATE_FAULT = 4U
};

enum {
    APP_MENU_MOTOR = 0U,
    APP_MENU_BUZZER = 1U,
    APP_MENU_LED = 2U,
    APP_MENU_COUNT = 3U
};

enum {
    APP_MOTOR_SUBITEM_RUN = 0U,
    APP_MOTOR_SUBITEM_SPEED = 1U,
    APP_MOTOR_SUBITEM_BACK = 2U,
    APP_MOTOR_SUBITEM_COUNT = 3U
};

enum {
    APP_BUZZER_SUBITEM_MODE = 0U,
    APP_BUZZER_SUBITEM_VOLUME = 1U,
    APP_BUZZER_SUBITEM_BACK = 2U,
    APP_BUZZER_SUBITEM_COUNT = 3U
};

enum {
    APP_LED_SUBITEM_COUNT = 0U,
    APP_LED_SUBITEM_BRIGHTNESS = 1U,
    APP_LED_SUBITEM_LED1 = 2U,
    APP_LED_SUBITEM_LED2 = 3U,
    APP_LED_SUBITEM_LED3 = 4U,
    APP_LED_SUBITEM_BACK = 5U,
    APP_LED_SUBITEM_COUNT_TOTAL = 6U
};

enum {
    APP_LED_MODE_OFF = 0U,
    APP_LED_MODE_SOLID = 1U,
    APP_LED_MODE_RAIN = 2U
};

enum {
    APP_LED_COLOR_RED = 0U,
    APP_LED_COLOR_GREEN = 1U,
    APP_LED_COLOR_BLUE = 2U,
    APP_LED_COLOR_WHITE = 3U,
    APP_LED_COLOR_COUNT = 4U
};

typedef struct {
    AppState state;
    uint8_t clearRequest;
    uint8_t externalFault;
    uint8_t reserved;
    uint8_t menuItem;
    uint8_t menuLevel;
    uint8_t menuSubItem;
    uint8_t editMode;
    uint8_t buzzerMode;
    uint8_t buzzerVolume;
    uint8_t ledCount;
    uint8_t ledBrightness;
    uint8_t ledColor[3];
    uint32_t sequence;
    uint32_t issuedAtMs;
    int32_t targetRpmA;
    int32_t targetRpmB;
    int32_t targetRpmC;
    int32_t targetRpmD;
} AppMotorCommand;

typedef struct {
    int32_t deltaCountsA;
    int32_t deltaCountsB;
    int32_t deltaCountsC;
    int32_t deltaCountsD;
    int32_t rpmA;
    int32_t rpmB;
    int32_t rpmC;
    int32_t rpmD;
    uint32_t invalidTransitionsA;
    uint32_t invalidTransitionsB;
    uint32_t invalidTransitionsC;
    uint32_t invalidTransitionsD;
} AppEncoderSnapshot;

typedef struct {
    AppState state;
    uint8_t commandTimedOut;
    uint16_t reserved;
    uint8_t menuItem;
    uint8_t menuLevel;
    uint8_t menuSubItem;
    uint8_t editMode;
    uint32_t commandSequence;
    uint32_t observedAtMs;
    int32_t targetRpmA;
    int32_t rpmA;
    int32_t rpmB;
    int32_t rpmC;
    int32_t rpmD;
    int32_t dutyA;
    int32_t dutyB;
    int32_t dutyC;
    int32_t dutyD;
    uint32_t invalidTransitionsA;
    uint32_t invalidTransitionsB;
    uint32_t invalidTransitionsC;
    uint32_t invalidTransitionsD;
    uint8_t buzzerMode;
    uint8_t buzzerVolume;
    uint8_t ledCount;
    uint8_t ledBrightness;
    uint8_t ledColor[3];
} AppMotorStatus;

typedef struct {
    uint8_t latched;
    uint8_t reason;
    uint16_t reserved;
    uint32_t sequence;
} AppSafetyLatch;

typedef struct {
    AppState state;
    uint8_t safetyLatched;
    uint16_t reserved;
    uint32_t commandSequence;
    int32_t rpmA;
    int32_t rpmB;
    int32_t rpmC;
    int32_t rpmD;
} AppUiSnapshot;

#endif
