# Motor Direction and Per-Channel Enable Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Normalize mirrored motor feedback to vehicle-forward coordinates and add independent A/B/C/D enable controls to the existing motor menu.

**Architecture:** Keep channel identity direct from command through BSP. Normalize B/D encoder samples in `encoder_ab`, carry a four-bit enable mask through command and status objects, and gate PID/output in `MotorControl_Process`; hardware stop remains zero duty. Remove obsolete swapped-channel wording from active documentation.

**Tech Stack:** C11, MSPM0 DriverLib, FreeRTOS static tasks/queues, PowerShell/MSVC host tests, Keil MDK.

## Global Constraints

- A/B/C/D directly name their software and D24A channels.
- Positive target RPM means vehicle forward; mirrored A/C and B/D shafts may physically rotate oppositely.
- B/D forward encoder feedback is normalized to positive; A/C is unchanged.
- Default enable mask is all four channels, while startup remains READY.
- KEY1 enables, KEY2 disables, and KEY3 exits edit mode.
- Disabled channels emit zero duty and reset only their own PID state.
- Do not change 20 kHz PWM timing or established ESTOP/FAULT behavior.

---

### Task 1: Normalize mirrored encoder feedback

**Files:**
- Modify: `tests/module_behavior_harness.c`
- Modify: `module/encoder_ab.c`

**Interfaces:**
- Consumes: `BspEncoder_ReadAndClearA/B/C/D()` raw signed deltas.
- Produces: `EncoderAb_ReadA/C()` unchanged and `EncoderAb_ReadB/D()` sign-inverted with saturating `INT32_MIN` handling.

- [ ] **Step 1: Write the failing host behavior test**

Set B to `123`, C to `-222`, D to `40`; assert B is `-123`, C remains `-222`, D is `-40`, and invalid-transition counts are unchanged.

- [ ] **Step 2: Run the module test and verify RED**

Run: `./tests/test_module_contracts.ps1`

Expected: FAIL because B/D deltas currently pass through unchanged.

- [ ] **Step 3: Implement channel direction normalization**

Add a local saturating sign helper and pass direction `+1` for A/C and `-1` for B/D when converting `BspEncoderReadout` to `EncoderAbSnapshot`.

- [ ] **Step 4: Run the module test and verify GREEN**

Run: `./tests/test_module_contracts.ps1`

Expected: PASS.

### Task 2: Add command-level A/B/C/D enable controls

**Files:**
- Modify: `application/app_types.h`
- Modify: `application/app_init.c`
- Modify: `application/command_task.c`
- Modify: `application/safety_task.c`
- Modify: `tests/command_button_harness.c`
- Modify: `tests/host_stub/command_button_host_stub.c`

**Interfaces:**
- Produces: `APP_MOTOR_ENABLE_A/B/C/D/ALL`, `AppMotorCommand.motorEnableMask`, and `AppMotorStatus.motorEnableMask`.
- Produces motor submenu indices `RUN`, `SPEED`, `ENABLE_A`, `ENABLE_B`, `ENABLE_C`, `ENABLE_D`, `BACK`.

- [ ] **Step 1: Write failing command tests**

Assert `App_DefaultCommand().motorEnableMask == APP_MOTOR_ENABLE_ALL`; in menu edit mode assert KEY2 clears only the selected channel bit, KEY1 restores it, and KEY3 returns to selection mode.

- [ ] **Step 2: Run command tests and verify RED**

Run: `./tests/test_command_buttons.ps1`

Expected: compilation fails because enable-mask fields and submenu constants do not exist.

- [ ] **Step 3: Implement the command/status contract**

Add four bit constants and the mask fields, default to `APP_MOTOR_ENABLE_ALL`, expand the motor submenu, and edit one selected bit with KEY1/KEY2. Copy the mask into both motor and safety status publications.

- [ ] **Step 4: Run command tests and verify GREEN**

Run: `./tests/test_command_buttons.ps1`

Expected: PASS.

### Task 3: Gate PID and output independently

**Files:**
- Modify: `application/motor_control_task.c`
- Modify: `tests/application_behavior_harness.c`
- Modify: `tests/host_stub/application_host_stub.c`

**Interfaces:**
- Consumes: `AppMotorCommand.motorEnableMask`.
- Produces: zero duty plus reset PID for disabled channels while enabled channels continue through `SpeedPid_Update`.

- [ ] **Step 1: Write the failing MotorControl behavior test**

Use a RUN command with A/C enabled and B/D disabled. Seed all PID state, call `MotorControl_Process`, then assert B/D duty and PID history are zero while A/C produce their normal outputs and status carries the same mask.

- [ ] **Step 2: Run application tests and verify RED**

Run: `./tests/test_application_contracts.ps1`

Expected: FAIL because the current controller updates all four PID instances.

- [ ] **Step 3: Implement per-channel gating**

For each channel, test its enable bit. Call `SpeedPid_Update` only when enabled; otherwise call `SpeedPid_Reset` and leave the zero-initialized status duty. Continue calling all four motor setters so disabled hardware receives explicit zero duty.

- [ ] **Step 4: Run application tests and verify GREEN**

Run: `./tests/test_application_contracts.ps1`

Expected: PASS.

### Task 4: Render enable states and remove obsolete mapping text

**Files:**
- Modify: `module/st7735_ui.c`
- Modify: `tests/screen_render_harness.c`
- Modify: `tests/test_screen_render_contracts.ps1`
- Modify: `README.md`
- Modify: `D:/TI/引脚分布/智能小车总体接线方案.md`
- Modify: `D:/TI/docs/superpowers/specs/2026-07-19-screen-silkscreen-channel-mapping-design.md`

**Interfaces:**
- Consumes: `AppMotorStatus.motorEnableMask`.
- Produces: motor menu lines `A:ON/OFF`, `B:ON/OFF`, `C:ON/OFF`, `D:ON/OFF` and direct-channel documentation.

- [ ] **Step 1: Write failing screen and documentation tests**

Render the motor submenu with alternating enable bits and assert the frame contains all four expected channel states. Add checks that active docs no longer contain the old pair-swap mapping or pair-swapped display contract.

- [ ] **Step 2: Run focused tests and verify RED**

Run: `./tests/test_screen_render_contracts.ps1; ./tests/test_ad_port.ps1`

Expected: FAIL because enable lines are absent and obsolete mapping text remains.

- [ ] **Step 3: Implement display and documentation corrections**

Expand the motor menu renderer using the existing 2x font and `St7735Ui_AppendMenuLine`. Replace old mapping prose with direct A/B/C/D naming and the mirrored-installation forward-direction rule.

- [ ] **Step 4: Run focused tests and verify GREEN**

Run: `./tests/test_screen_render_contracts.ps1; ./tests/test_ad_port.ps1`

Expected: both PASS.

### Task 5: Full verification and delivery

**Files:**
- Verify: `keil/Objects/car_freertos_LP_MSPM0G3507.hex`

- [ ] **Step 1: Run complete verification**

Run: `./tests/run_final_verification.ps1`

Expected: all PowerShell tests pass and Keil reports `0 Error(s), 0 Warning(s)`.

- [ ] **Step 2: Inspect final diff and firmware**

Run: `git diff --check; git status --short` and inspect HEX size/timestamp.

Expected: no whitespace errors and only planned files modified.

- [ ] **Step 3: Commit and push**

Commit implementation as `feat: add per-channel motor enables`, commit external documentation changes where repository ownership permits, and push project `main` to `origin`.
