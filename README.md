# MSPM0G3507 小车 FreeRTOS 工程

本工程是 MSPM0G3507 小车底盘的软件框架。它面向 D24A A/B/C/D 四路电机、MG513X 编码器、ST7735S 显示、PA14/PA15/PA16 扩展板按键和 PB21 急停；构建成功或主机测试通过不等同于实物已通过验收。

> **拓展板丝印勘误：**物理接口丝印与软件/D24A 通道的对应关系为 `A→B`、`B→A`、`C→D`、`D→C`。代码中的 A/B/C/D 始终表示实际 D24A 逻辑通道，不表示拓展板接口旁当前印刷的字母。

## 分层职责

| 层 | 职责 | 不应承担的职责 |
|---|---|---|
| BSP | 对 DriverLib、SysConfig、PWM、GPIO、编码器中断、SPI、时间、日志和 FreeRTOS 启动做板级封装 | 速度策略、界面业务和状态机决策 |
| Module | 电机、AB 编码器、速度 PID、ST7735 UI 与安全状态机的可复用逻辑 | 直接依赖 `ti_msp_dl_config.h` 或调度任务 |
| Application | 创建静态任务/队列，组织命令、控制、安全、UI 和日志数据流 | 直接操作寄存器或在 1 kHz 路径刷新屏幕 |

依赖只允许从 `Application -> Module -> BSP -> DriverLib/SysConfig` 向下。

## 任务与数据

| 任务 | 周期 | 优先级 | 责任 |
|---|---:|---:|---|
| `MotorControlTask` | 1 ms | 高 | 四路编码器快照、PID 和 A/B/C/D PWM |
| `SafetyTask` | 10 ms | 最高 | 急停、故障锁存、命令超时与安全停机 |
| `CommandTask` | 20 ms | 中 | 读取 PA14/PA15/PA16 按键，发布四路目标转速与运行/停止命令 |
| `UITask` | 100 ms | 低 | 读取状态快照，以 ST7735S 固定字库和固定区域刷新状态 |
| `LogTask` | 低频 | 最低 | 排空受限日志缓冲 |
| `HeartbeatTask` | 500 ms | 1 | 翻转 PB22 心跳 |

`App_Init()` 独占创建五个长度为 1 的静态队列及其静态缓冲：命令队列、Safety 命令镜像队列、电机状态队列、安全状态队列和 UI 状态队列。发布端使用 `xQueueOverwrite()` 保留最新快照，消费者通过对应 `App_ReadLatest*()` 接口读取；任务使用 `xTaskCreateStatic()`，工程不依赖 Application/Module 层动态内存。

## 按键与显示

- `PA14 / KEY1`：每次按下使四路目标速度增加 50 RPM，最高 300 RPM。
- `PA15 / KEY2`：每次按下使四路目标速度减少 50 RPM，最低 0 RPM。
- `PA16 / KEY3`：运行/停止切换；静止时首次运行以 100 RPM 起步，停止后四路目标均归零。
- 上电后默认发布 `RUN` 命令，四路目标速度为 100 RPM；安全状态机经过 `INIT -> READY -> RUN` 后开始输出，PB21 急停和故障锁存仍优先阻止启动。
- `PB21`：独立的 active-low 急停输入，只由 `SafetyTask` 处理；`ESTOP` 或 `FAULT` 会清零 PWM 并禁止陈旧的运行命令重新输出。
- ST7735S 每 100 ms 用静态 5x7 字库填充固定的 128x56 RGB565 状态区域。初始化时才全屏清屏，运行期间不再把 ASCII 文本直接作为像素数据发送，也不在 1 kHz 电机任务中刷新屏幕。

## 安全状态

```text
INIT -> READY -> RUN
                 |  \
                 |   -> ESTOP
                 -> FAULT
```

`INIT`、`ESTOP` 和 `FAULT` 必须令 PWM 为零并调用安全停机。急停、外部故障、命令超时或编码器异常不得被界面刷新或日志输出延后。`ESTOP`/`FAULT` 的解除必须按安全模块定义的复位条件进行，不能用重新下载固件代替故障分析。

## 构建与刷写

在工程根目录运行完整验证：

```powershell
powershell -ExecutionPolicy Bypass -File tests\run_final_verification.ps1
```

只执行 Keil 全量重建：

```powershell
& 'D:\keli5\UV4\UV4.exe' -cr keil\car_freertos_LP_MSPM0G3507.uvprojx
```

DAPLink 使用 SWD、1 MHz 和 MSPM0G1X0X/G3X0X 128 KB Flash 算法。Keil 可将调试目标序列化为 `CMSIS-DAP` 或 `ARM CoreSight SW-DP`，两者均是本工程总验证接受的合法表达。连接目标板、确认公共地和供电稳定后，再在 Keil 中执行 Download；下载 Verify 成功只证明本次烧写/校验，不证明电机、编码器或显示正常。

## 第一阶段限制

- 未实现电流采样或电流环，不能把软件状态当作过流保护。
- D24A 12 V 物理测试、A/B/C/D 实物编码器、PA14/PA15/PA16 实物按键、ST7735S 实物显示和整机急停仍须按硬件清单记录。
- 现阶段未包含 IMU、循迹、上位机通信、完整运动学或闭环电流控制。
- 1 kHz 电机任务不得执行阻塞 SPI、显示刷新或阻塞日志。
-
## Current Menu Control Reference

The local menu uses the three expansion buttons: KEY1/PA14 moves up or increases a value, KEY2/PA15 moves down or decreases a value, and KEY3/PA16 enters, confirms, or returns. The main menu contains MOTOR, BUZZER, and LED submenus.

MOTOR controls all four logical motor channels together from 0 to 600 RPM in 10 RPM steps. BUZZER controls the active buzzer on PB27 with OFF, CONT, SLOW, and FAST gating modes plus 10% to 100% gate volume; it does not change the buzzer's internal tone frequency. LED controls three serial WS2812B devices on PA29 with count 0 to 3, brightness 0% to 100%, and independent LED1/LED2/LED3 colors. PB22 remains the onboard LED.
