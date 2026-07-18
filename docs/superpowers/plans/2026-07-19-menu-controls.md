# 电机、蜂鸣器和 WS2812B 菜单控制 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在 MSPM0G3507 FreeRTOS 小车工程中实现滚动列表式主菜单、三个独立子菜单、2 倍字体显示，以及电机速度、PB27 有源蜂鸣器和 PA29 三颗 WS2812B 的完整本地控制。

**Architecture:** CommandTask 维护菜单状态并通过 `AppMotorCommand` 发布；SafetyTask 复制安全允许显示的配置；UITask 通过 `AppMotorStatus` 渲染滚动菜单。蜂鸣器和 WS2812B 只在 BSP 层接触 DriverLib，电机继续复用四路 PID/PWM 和 PB21 急停链路。

**Tech Stack:** MSPM0 SDK 2.11.00.07、TI SysConfig、Keil ARMClang、FreeRTOS、C、PowerShell 主机测试。

## Global Constraints

- 速度范围固定为 `0~600 RPM`，步进 `10 RPM`。
- PB27 连接有源蜂鸣器，只实现 `OFF/CONT/SLOW/FAST` 和 `10%~100%` 门控响度，不实现真实音调频率调节。
- WS2812B DIN 固定为 `PA29`；`PB22` 保留为板载 LED。
- WS2812B 支持 `COUNT=0~3`、亮度 `0~100%`、`LED1/LED2/LED3` 独立颜色。
- KEY1/PA14 上移或增加，KEY2/PA15 下移或减少，KEY3/PA16 进入/确认/返回；PB21 只用于急停。
- ST7735S 使用 2 倍固定字体和滚动窗口，所有动态文本使用固定尺寸布局。
- 急停、故障和命令超时必须关闭电机 PWM、蜂鸣器并发送 WS2812B 全黑帧。
- Keil 重编译必须达到 `0 Error(s), 0 Warning(s)`。

---

### Task 1: 扩展应用数据契约

**Files:**
- Modify: `application/app_types.h`
- Modify: `application/app_init.c`
- Test: `tests/test_menu_led_buzzer.ps1`

**Interfaces:**
- `AppMotorCommand` 产生 `menuItem`、`editMode`、`buzzerMode`、`buzzerVolume`、`ledCount`、`ledBrightness`、`ledColor[3]` 和四路统一目标转速。
- `AppMotorStatus` 产生同名只读显示字段。
- `App_DefaultCommand()` 默认 `RUN`、`100 RPM`、蜂鸣器 `OFF`、灯珠 `COUNT=0`、亮度 `30%`、颜色 `RED/RED/RED`。

- [ ] **Step 1: 写失败契约测试**

在 `tests/test_menu_led_buzzer.ps1` 中增加以下检查：`buzzerMode`、`buzzerVolume`、`ledCount`、`ledBrightness`、`ledColor[3]`、`APP_BUZZER_MODE_FAST`、`APP_LED_COLOR_COUNT` 和默认值字段。

- [ ] **Step 2: 运行测试确认失败**

运行：

```powershell
powershell -ExecutionPolicy Bypass -File tests/test_menu_led_buzzer.ps1
```

预期：因新字段或枚举不存在而失败。

- [ ] **Step 3: 实现最小数据结构变更**

在 `app_types.h` 增加：

```c
enum { APP_BUZZER_MODE_OFF, APP_BUZZER_MODE_CONT, APP_BUZZER_MODE_SLOW, APP_BUZZER_MODE_FAST };
enum { APP_BUZZER_VOLUME_MIN = 0U, APP_BUZZER_VOLUME_MAX = 100U };
```

将 WS2812 配置从单个 `ledColor` 扩展为 `uint8_t ledCount; uint8_t ledBrightness; uint8_t ledColor[3];`，并在命令和状态结构中保持字段顺序一致。

- [ ] **Step 4: 运行测试确认通过**

运行同一 PowerShell 测试，预期输出 `PASS: menu, buzzer, and WS2812 contracts are complete.`。

### Task 2: 重写菜单状态机和按键行为

**Files:**
- Modify: `application/command_task.h`
- Modify: `application/command_task.c`
- Modify: `application/app_init.c`
- Modify: `tests/host_stub/command_button_host_stub.c`
- Test: `tests/command_button_harness.c`

**Interfaces:**
- 保留 `CommandTask_ApplyButtonEvents()` 兼容旧测试。
- 新增可主机测试的 `CommandTask_ApplyMenuEvents()` 或等价公开测试入口，输入三个按键边沿和 `AppMotorCommand *`，不直接访问 DriverLib。
- 菜单层只修改命令数据；BSP 外设由发布后的配置驱动。

- [ ] **Step 1: 写失败状态机测试**

覆盖：主菜单 MOTOR/BUZZER/LED 循环，速度 `0` 和 `600` 饱和，10 RPM 步进，蜂鸣器四模式和音量边界，灯珠 COUNT 0/3、亮度边界、三颗颜色独立切换，以及 KEY3 进入/确认/返回。

- [ ] **Step 2: 运行主机测试确认失败**

运行：

```powershell
powershell -ExecutionPolicy Bypass -File tests/test_command_buttons.ps1
```

预期：新菜单状态断言失败。

- [ ] **Step 3: 实现状态机**

浏览态由 KEY1/KEY2 移动 `menuItem`；编辑态按当前字段修改值。LED 子菜单使用 `ledSubItem=COUNT/BRI/LED1/LED2/LED3/BACK`，颜色修改只写对应的 `ledColor[index]`。所有数值通过显式 `min/max` 裁剪。

- [ ] **Step 4: 运行主机测试确认通过**

运行 `tests/test_command_buttons.ps1`，预期旧测试和新增菜单状态机测试全部通过。

### Task 3: 实现有源蜂鸣器节奏和门控音量

**Files:**
- Modify: `bsp/buzzer/bsp_buzzer.h`
- Modify: `bsp/buzzer/bsp_buzzer.c`
- Modify: `application/command_task.c`
- Modify: `keil/car_freertos_LP_MSPM0G3507.uvprojx`
- Test: `tests/test_menu_led_buzzer.ps1`

**Interfaces:**
- `void BspBuzzer_Init(void);`
- `void BspBuzzer_SetConfig(uint8_t mode, uint8_t volume, uint32_t nowMs);`
- `void BspBuzzer_Tick(uint32_t nowMs);`
- `void BspBuzzer_Stop(void);`

- [ ] **Step 1: 写 BSP 契约测试**

检查 PB27 配置、四种模式、音量裁剪、`SetConfig/Tick/Stop` 接口及项目文件注册。

- [ ] **Step 2: 运行契约测试确认失败**

运行 `tests/test_menu_led_buzzer.ps1`，预期缺少新 API。

- [ ] **Step 3: 实现最小驱动**

使用 PB27 GPIO 输出；CONT 按音量门控，SLOW 使用约 500 ms 周期，FAST 使用约 100 ms 周期。门控采用周期内通断比例，明确不改变有源蜂鸣器内部振荡频率。配置未变化时不重复初始化 GPIO。

- [ ] **Step 4: 在 CommandTask 中调用并验证**

每周期调用 `BspBuzzer_SetConfig()` 和 `BspBuzzer_Tick()`，安全状态调用 `BspBuzzer_Stop()`。运行契约测试和 Keil 编译。

### Task 4: 扩展 WS2812B 三颗独立颜色、数量和亮度

**Files:**
- Modify: `bsp/ws2812/bsp_ws2812.h`
- Modify: `bsp/ws2812/bsp_ws2812.c`
- Modify: `config/car_config.h`
- Modify: `generated/empty.syscfg`
- Test: `tests/test_ws2812_host.ps1`

**Interfaces:**
- `void BspWs2812_Init(void);`
- `void BspWs2812_SetConfig(uint8_t count, uint8_t brightness, const uint8_t colors[3]);`
- `void BspWs2812_Tick(uint32_t nowMs);`
- `void BspWs2812_Off(void);`

- [ ] **Step 1: 写帧编码主机测试**

验证 COUNT=0/1/2/3 的黑灯数量，三颗颜色的 GRB 顺序，亮度缩放 `255*brightness/100` 和颜色索引越界回 RED。

- [ ] **Step 2: 运行测试确认失败**

运行 `tests/test_ws2812_host.ps1`，预期新配置接口和帧捕获桩不存在。

- [ ] **Step 3: 实现配置和帧编码**

保持 PA29 的 bit-bang 时序；每颗 LED 根据 `index < count` 发送对应颜色，否则发送 `0,0,0`。只在配置变化或动态 tick 时发送，保持发送期间的短暂中断屏蔽。

- [ ] **Step 4: 运行主机测试和 Keil 编译**

运行 `tests/test_ws2812_host.ps1`，然后执行：

```powershell
& 'D:\keli5\UV4\UV4.exe' -cr keil\car_freertos_LP_MSPM0G3507.uvprojx -o keil\ws2812_build.log
```

预期：主机测试通过，Keil 日志包含 `0 Error(s), 0 Warning(s)`。

### Task 5: 实现 2 倍字体滚动菜单渲染

**Files:**
- Modify: `module/st7735_ui.c`
- Modify: `module/st7735_ui.h`
- Modify: `application/ui_task.c`
- Modify: `tests/screen_render_harness.c`
- Modify: `tests/test_screen_render_contracts.ps1`

**Interfaces:**
- `St7735Ui_RenderStatus(const AppMotorStatus *status)` 保持调用接口不变。
- 渲染器内部使用 `scale=2` 的固定字模和菜单窗口偏移。

- [ ] **Step 1: 写屏幕失败测试**

检查 2 倍字模像素块、主菜单三个项目、三个子菜单字段、光标、滚动窗口和长颜色名不越界。

- [ ] **Step 2: 运行屏幕测试确认失败**

运行：

```powershell
powershell -ExecutionPolicy Bypass -File tests/test_screen_render_contracts.ps1
```

预期：旧的单倍字体或旧菜单断言失败。

- [ ] **Step 3: 实现固定尺寸渲染**

将每个 5x7 glyph 的每个像素扩展为 2x2，字符 advance 设为 12 像素、行 advance 设为 16 像素；依据当前子菜单和滚动偏移只渲染可见行。显示文本使用 ASCII 字段名和已确认值名，避免中文字库缺失。

- [ ] **Step 4: 运行屏幕测试确认通过**

运行屏幕测试并检查输出帧的白色像素数量、边界和字符串内容。

### Task 6: 安全链路、状态发布和完整回归

**Files:**
- Modify: `application/motor_control_task.c`
- Modify: `application/safety_task.c`
- Modify: `application/app_init.c`
- Modify: `tests/run_final_verification.ps1`
- Modify: `README.md`
- Modify: `HARDWARE_BRINGUP_CHECKLIST_zh-CN.md`

**Interfaces:**
- SafetyTask 在 ESTOP/FAULT/超时路径统一调用电机停止、`BspBuzzer_Stop()` 和 `BspWs2812_Off()`。
- UI 状态必须从 SafetyTask 发布的 `AppMotorStatus` 获得，而不是直接读取命令队列。

- [ ] **Step 1: 写安全回归断言**

扩展 Task 5 主机测试，断言安全状态时 PWM 为零、蜂鸣器停止、WS2812B 全黑，正常 RUN 状态保留菜单配置。

- [ ] **Step 2: 实现状态复制和安全关闭**

在 MotorControl/SafetyTask 的 designated initializer 和发布路径中完整复制新字段；对 NULL 命令使用安全默认值，避免状态显示未初始化。

- [ ] **Step 3: 更新验证清单和说明**

把 README 的旧按键/心跳任务说明改为三级菜单、PB27、PA29 和 PB22 的正确映射；在完整验证脚本中加入 WS2812 主机测试和菜单测试。

- [ ] **Step 4: 运行完整验证**

运行：

```powershell
powershell -ExecutionPolicy Bypass -File tests/run_final_verification.ps1
```

预期：所有 PowerShell 测试通过，DAPLink 配置保持不变，Keil 重建为 `0 Error(s), 0 Warning(s)`，生成 AXF 和 HEX。

- [ ] **Step 5: 硬件分阶段验证**

先不接电机负载烧录；确认 2 倍字体菜单、PA14/PA15/PA16 导航、PB27 四种节奏、PA29 三颗灯珠数量/独立颜色/亮度；最后接 12 V 电机驱动并用 PB21 急停验证安全链路。

## Review Checklist

- [ ] 速度确实覆盖 0~600 RPM 且步进 10 RPM。
- [ ] 有源蜂鸣器未被错误宣传为可调真实频率。
- [ ] LED1、LED2、LED3 颜色独立保存并按 GRB 顺序发送。
- [ ] COUNT 小于 3 时后续灯珠始终发送全黑。
- [ ] 2 倍字体下没有文字重叠或写出屏幕边界。
- [ ] PA29、PB27、PB22 的硬件映射与代码一致。
- [ ] 急停和故障路径关闭所有相关执行器。
