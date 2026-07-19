# LED Color Simplification Design

## Goal

Reduce the LED menu to four visually distinct colors and make the white label
render correctly with the existing 2x ST7735 font.

## Behavior

- The selectable colors are RED, GREEN, BLUE, and WHITE, in that order.
- The menu labels are `RED`, `GRN`, `BLU`, and `WHT`.
- Pressing the adjustment buttons wraps through exactly these four colors.
- Existing default color values remain RED.
- WS2812 output remains GRB, with no timing, brightness, or LED-count changes.

## Implementation

- Remove YELLOW, CYAN, and MAGENTA from the application color enum and set
  `APP_LED_COLOR_COUNT` to 4.
- Reduce the WS2812 RGB lookup table to the matching four entries.
- Reduce the display color-name table to the matching four labels.
- Add `W` and `H` glyphs to the fixed 5x7 font. `WHT` is nine characters when
  rendered as `>LED1:WHT`, so it fits within the 128-pixel display at the
  existing 12-pixel character advance.

## Verification

- Host command tests verify forward and backward wrapping across four colors.
- Screen rendering tests verify that both `W` and `H` produce visible pixels
  and that the frame contains `WHT`.
- WS2812 contract tests verify a four-entry lookup table and preserve the
  timer/DMA protocol checks.
- The full Keil project rebuild must complete with zero errors and warnings.
