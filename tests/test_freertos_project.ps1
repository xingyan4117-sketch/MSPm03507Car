$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot
$sdkRoot = Split-Path -Parent $projectRoot

function Require-File([string] $Path, [string] $Description) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "Missing ${Description}: $Path"
    }
}

function Require-Token([string] $Path, [string] $Token) {
    $content = Get-Content -LiteralPath $Path -Raw
    if ($content -notmatch [regex]::Escape($Token)) {
        throw "Missing token '$Token' in $Path"
    }
}

function Require-Regex([string] $Path, [string] $Pattern, [string] $Description) {
    $content = Get-Content -LiteralPath $Path -Raw
    if ($content -notmatch $Pattern) {
        throw "Missing ${Description} in $Path"
    }
}

function Require-Macro([string] $Path, [string] $Name, [string] $Value) {
    $content = Get-Content -LiteralPath $Path -Raw
    if ($content -notmatch "(?m)^\s*#define\s+$Name\s+$Value") {
        throw "Missing macro '$Name $Value' in $Path"
    }
}

$projectFile = Join-Path $projectRoot 'keil\car_freertos_LP_MSPM0G3507.uvprojx'
$projectOptionsFile = Join-Path $projectRoot 'keil\car_freertos_LP_MSPM0G3507.uvoptx'
$configFile = Join-Path $projectRoot 'config\FreeRTOSConfig.h'
$carConfigFile = Join-Path $projectRoot 'config\car_config.h'
$syscfgSourceFile = Join-Path $projectRoot 'generated\empty.syscfg'
$generatedHeaderFile = Join-Path $projectRoot 'generated\ti_msp_dl_config.h'
$generatedSourceFile = Join-Path $projectRoot 'generated\ti_msp_dl_config.c'
$startupFile = Join-Path $projectRoot 'keil\startup_mspm0g350x_uvision.s'
$scatterFile = Join-Path $projectRoot 'keil\mspm0g3507.sct'
$mainFile = Join-Path $projectRoot 'main.c'
$appInitFile = Join-Path $projectRoot 'application\app_init.c'
$freertosBspFile = Join-Path $projectRoot 'bsp\freertos\bsp_freertos.c'
$pwmBspFile = Join-Path $projectRoot 'bsp\pwm\bsp_pwm.c'

Require-File $projectFile 'Keil project'
Require-File $projectOptionsFile 'Keil user options'
Require-File $configFile 'FreeRTOS configuration'
Require-File $carConfigFile 'car hardware configuration'
Require-File $syscfgSourceFile 'SysConfig source'
Require-File $generatedHeaderFile 'generated DriverLib header'
Require-File $generatedSourceFile 'generated DriverLib source'
Require-File $startupFile 'startup assembly'
Require-File $scatterFile 'scatter file'
Require-File $mainFile 'smoke application'
Require-File $appInitFile 'heartbeat application'
Require-File $freertosBspFile 'FreeRTOS BSP'
Require-File $pwmBspFile 'PWM BSP'

Require-Token $configFile 'configCPU_CLOCK_HZ'
Require-Token $configFile '32000000'
Require-Token $configFile 'configTICK_RATE_HZ'
Require-Token $configFile '1000'
Require-Macro $configFile 'configUSE_PREEMPTION' '1'
Require-Macro $configFile 'configSUPPORT_STATIC_ALLOCATION' '1'
Require-Macro $configFile 'configCHECK_FOR_STACK_OVERFLOW' '2'
Require-Token $configFile 'configASSERT'

Require-Token $projectFile 'car_freertos_LP_MSPM0G3507'
Require-Token $projectFile 'TexasInstruments.MSPM0G1X0X_G3X0X_DFP.1.3.1'
Require-Token $projectFile 'cmd.exe /C "$P../../tools/keil/syscfg.bat ''$P../generated'' empty.syscfg"'
Require-Token $projectFile '../generated/empty.syscfg'
Require-Token $projectFile '../generated/ti_msp_dl_config.c'
Require-Token $projectFile 'startup_mspm0g350x_uvision.s'
Require-Token $projectFile './mspm0g3507.sct'

foreach ($token in @('PB13', 'PB4', 'PB5', 'PA9', 'PA12', 'PA17', 'PB15', 'PB16', 'PA21', 'PA24', 'PB8', 'PB9', 'PB10', 'PB11', 'PB14', 'PB26', 'PB22', 'PB21')) {
    Require-Token $carConfigFile $token
}

foreach ($token in @(
    '@cliArgs --device "MSPM0G3507"',
    '@v2CliArgs --device "MSPM0G3507"',
    'GPIO_HEARTBEAT',
    'pin.$assign = "PB22"',
    'SYSCTL.clockTreeEn = true'
)) {
    Require-Token $syscfgSourceFile $token
}

foreach ($token in @(
    'CONFIG_MSPM0G3507',
    'CPUCLK_FREQ',
    '32000000',
    'GPIO_HEARTBEAT_HEARTBEAT_PIN',
    'GPIO_HEARTBEAT_HEARTBEAT_IOMUX',
    'void SYSCFG_DL_init(void);'
)) {
    Require-Token $generatedHeaderFile $token
}

foreach ($token in @(
    'SYSCFG_DL_initPower();',
    'SYSCFG_DL_GPIO_init();',
    'DL_GPIO_initDigitalOutput(GPIO_HEARTBEAT_HEARTBEAT_IOMUX);',
    'DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);'
)) {
    Require-Token $generatedSourceFile $token
}
Require-Regex $generatedSourceFile 'DL_GPIO_enableOutput\s*\(\s*GPIOB\s*,[\s\S]*GPIO_HEARTBEAT_HEARTBEAT_PIN' 'generated heartbeat output enable'

foreach ($token in @('__Vectors', 'Reset_Handler', 'SVC_Handler', 'PendSV_Handler', 'SysTick_Handler', '__main')) {
    Require-Token $startupFile $token
}

foreach ($token in @('LR_IROM1', 'ER_IROM1', 'RW_IRAM2', 'LR_BCR', 'LR_BSL')) {
    Require-Token $scatterFile $token
}

foreach ($token in @(
    'BIN\CMSIS_AGDI.dll',
    'CMSIS_AGDI',
    'MAIN_128KB.FLM',
    '-L00(0)'
)) {
    Require-Token $projectOptionsFile $token
}
Require-Regex $projectOptionsFile '<Name>[\s\S]*?(?:-X"CMSIS-DAP"|ARM CoreSight SW-DP)[\s\S]*?</Name>' 'compatible DAPLink debugger selection'
Require-Regex $projectOptionsFile '<Protocol>\s*2\s*</Protocol>' 'SWD protocol selection'
Require-Regex $projectOptionsFile '<DbgClock>\s*1000000\s*</DbgClock>' '1 MHz debug clock'

foreach ($token in @(
    '../../kernel/freertos/Source/croutine.c',
    '../../kernel/freertos/Source/event_groups.c',
    '../../kernel/freertos/Source/list.c',
    '../../kernel/freertos/Source/queue.c',
    '../../kernel/freertos/Source/stream_buffer.c',
    '../../kernel/freertos/Source/tasks.c',
    '../../kernel/freertos/Source/timers.c',
    '../../kernel/freertos/Source/portable/MemMang/heap_4.c',
    '../../kernel/freertos/Source/portable/TI_ARM_CLANG/ARM_CM0/port.c',
    '../../kernel/freertos/Source/portable/TI_ARM_CLANG/ARM_CM0/portasm.c',
    '../../kernel/freertos/dpl/AppHooks_freertos.c',
    '../../kernel/freertos/dpl/ClockP_freertos.c',
    '../../kernel/freertos/dpl/DebugP_freertos.c',
    '../../kernel/freertos/dpl/MutexP_freertos.c',
    '../../kernel/freertos/dpl/SemaphoreP_freertos.c',
    '../../kernel/freertos/dpl/SystemP_freertos.c',
    '../../kernel/freertos/dpl/StaticAllocs_freertos.c',
    '../../kernel/freertos/dpl/TaskP_freertos.c',
    '../../kernel/freertos/dpl/HwiPMSPM0_freertos.c'
)) {
    Require-Token $projectFile $token
}

Require-Token $appInitFile 'xTaskCreateStatic'
Require-Token $freertosBspFile 'xQueueCreateStatic'
Require-Token $mainFile 'vTaskStartScheduler'
Require-Token $mainFile 'vApplicationStackOverflowHook'
Require-Regex $pwmBspFile 'void BspPwm_StopAll\s*\(\s*void\s*\)\s*\{[\s\S]*DL_TimerA_setCaptureCompareValue\s*\(\s*CAR_MOTOR_B_PWM_TIMER\s*,\s*0U\s*,\s*CAR_MOTOR_B_PWM_COMPARE_INDEX\s*\)' 'B motor compare clear in BspPwm_StopAll'
Require-Regex $pwmBspFile 'void BspPwm_StopAll\s*\(\s*void\s*\)\s*\{[\s\S]*DL_TimerA_setCaptureCompareValue\s*\(\s*CAR_MOTOR_C_PWM_TIMER\s*,\s*0U\s*,\s*CAR_MOTOR_C_PWM_COMPARE_INDEX\s*\)' 'C motor compare clear in BspPwm_StopAll'
Require-Token $pwmBspFile 'DL_TimerA_stopCounter(CAR_MOTOR_B_PWM_TIMER);'
Require-Token $pwmBspFile 'DL_TimerA_stopCounter(CAR_MOTOR_C_PWM_TIMER);'
Require-Token $pwmBspFile 'DL_GPIO_initDigitalOutput(CAR_MOTOR_B_PWM_IOMUX);'
Require-Token $pwmBspFile 'DL_GPIO_initDigitalOutput(CAR_MOTOR_C_PWM_IOMUX);'
Require-Token $pwmBspFile 'DL_GPIO_clearPins(GPIO_PWM_B_C3_PORT, GPIO_PWM_B_C3_PIN);'
Require-Token $pwmBspFile 'DL_GPIO_clearPins(GPIO_PWM_C_C0_PORT, GPIO_PWM_C_C0_PIN);'
Require-Token $pwmBspFile 'BspGpio_SetMotorDirection(BSP_GPIO_MOTOR_B, 0);'
Require-Token $pwmBspFile 'BspGpio_SetMotorDirection(BSP_GPIO_MOTOR_C, 0);'

Write-Host 'PASS: FreeRTOS MSPM0G3507 project structure is complete.'
