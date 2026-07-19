# LED Color Simplification Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Limit the WS2812 menu to red, green, blue, and white, and render the `WHT` label correctly.

**Architecture:** Keep the existing application enum as the shared color contract used by command handling, UI, and the WS2812 BSP. Reduce all indexed tables together, then extend the fixed 5x7 font without changing its scale or menu layout.

**Tech Stack:** C11, MSPM0 DriverLib, FreeRTOS, PowerShell host tests, Keil MDK.

## Global Constraints

- Selectable colors are RED, GREEN, BLUE, and WHITE, in that order.
- Menu labels are `RED`, `GRN`, `BLU`, and `WHT`.
- Preserve WS2812 GRB order, timer/DMA timing, brightness, and LED count behavior.
- The final Keil rebuild must report zero errors and zero warnings.

---

### Task 1: Four-color application contract

**Files:**
- Modify: `application/app_types.h`
- Modify: `bsp/ws2812/bsp_ws2812.c`
- Modify: `module/st7735_ui.c`
- Test: `tests/command_button_harness.c`
- Test: `tests/test_ws2812_host.ps1`

**Interfaces:**
- Consumes: `AppMotorCommand.ledColor[3]` and `APP_LED_COLOR_*` values.
- Produces: `APP_LED_COLOR_COUNT == 4` with matching UI and RGB lookup tables.

- [ ] **Step 1: Write failing tests for four-color wrapping and removed colors**

Add forward/backward wrap assertions around `APP_LED_COLOR_WHITE`, require `APP_LED_COLOR_COUNT = 4U`, and reject `YELLOW`, `CYAN`, and `MAGENTA` from the application enum and lookup tables.

- [ ] **Step 2: Run focused tests and verify failure**

Run: `./tests/test_command_buttons.ps1; ./tests/test_ws2812_host.ps1`

Expected: FAIL because the current contract contains seven colors.

- [ ] **Step 3: Implement the four-color contract**

Keep enum values `RED=0`, `GREEN=1`, `BLUE=2`, set `WHITE=3`, and set `COUNT=4`. Reduce both indexed tables to the same order.

- [ ] **Step 4: Run focused tests and verify success**

Run: `./tests/test_command_buttons.ps1; ./tests/test_ws2812_host.ps1`

Expected: both tests PASS.

### Task 2: White-label glyphs and final firmware

**Files:**
- Modify: `module/st7735_ui.c`
- Test: `tests/screen_render_harness.c`
- Test: `tests/test_screen_render_contracts.ps1`

**Interfaces:**
- Consumes: `St7735Ui_ColorName(APP_LED_COLOR_WHITE)` returning `WHT`.
- Produces: visible 5x7 glyphs for `W` and `H` through `St7735Ui_Glyph`.

- [ ] **Step 1: Write a failing screen test**

Render an LED submenu with `APP_LED_COLOR_WHITE`, assert the text frame contains `LED1:WHT`, and require `glyph_w`, `glyph_h`, plus their switch cases.

- [ ] **Step 2: Run the screen test and verify failure**

Run: `./tests/test_screen_render_contracts.ps1`

Expected: FAIL because `W` and `H` currently map to blank glyphs.

- [ ] **Step 3: Add the fixed-font glyphs**

Add 5-column uppercase glyphs for `W` and `H`, and return them from `St7735Ui_Glyph` for characters `'W'` and `'H'`.

- [ ] **Step 4: Run full verification**

Run: `./tests/run_final_verification.ps1`

Expected: all nine PowerShell tests PASS and Keil reports `0 Error(s), 0 Warning(s)`.

- [ ] **Step 5: Commit and push**

Commit the implementation and tests with `fix: simplify LED color selection`, then push `main` to `origin`.
