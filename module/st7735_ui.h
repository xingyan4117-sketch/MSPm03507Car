#ifndef ST7735_UI_H
#define ST7735_UI_H

#include <stddef.h>

#include "app_types.h"

#define ST7735_UI_FRAME_BYTES (96U)
#define ST7735_UI_DISPLAY_WIDTH (128U)
#define ST7735_UI_DISPLAY_HEIGHT (160U)
#define ST7735_UI_RENDER_HEIGHT (56U)
#define ST7735_UI_RENDER_BYTES \
    (ST7735_UI_DISPLAY_WIDTH * ST7735_UI_RENDER_HEIGHT * 2U)

void St7735Ui_Init(void);
void St7735Ui_Clear(void);
void St7735Ui_RenderStatus(const AppMotorStatus *status);
const char *St7735Ui_GetLastFrame(void);

#endif
