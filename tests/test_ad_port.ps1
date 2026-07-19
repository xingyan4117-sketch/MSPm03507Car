$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$syscfg = Join-Path $root 'generated\empty.syscfg'
$config = Join-Path $root 'config\car_config.h'
$encoder = Join-Path $root 'bsp\encoder\bsp_encoder.c'
$pwm = Join-Path $root 'bsp\pwm\bsp_pwm.h'
$gpio = Join-Path $root 'bsp\gpio\bsp_gpio.h'
$motor = Join-Path $root 'module\tb6612_motor.h'
$types = Join-Path $root 'application\app_types.h'
$ui = Join-Path $root 'module\st7735_ui.c'
$readme = Join-Path $root 'README.md'
$directionDesign = Join-Path $root 'docs\superpowers\specs\2026-07-19-motor-direction-and-channel-enable-design.md'

function Require-Token([string] $path, [string] $token, [string] $description) {
    if ((Get-Content -LiteralPath $path -Raw) -notmatch [regex]::Escape($token)) {
        throw "Missing ${description}: $token"
    }
}

foreach ($mapping in @(
    @{ Name = 'PWM_A'; Token = 'PWM_A.peripheral.ccp2Pin.$assign = "PB12"' },
    @{ Name = 'PWM_A_TIMER'; Token = 'PWM_A.peripheral.$assign = "TIMA0"' },
    @{ Name = 'PWM_B'; Token = 'PWM_A.peripheral.ccp3Pin.$assign = "PB13"' },
    @{ Name = 'PWM_C'; Token = 'PWM_C.peripheral.ccp0Pin.$assign = "PA17"' },
    @{ Name = 'PWM_D'; Token = 'PWM_D.peripheral.ccp1Pin.$assign = "PA13"' },
    @{ Name = 'PWM_D_TIMER'; Token = 'PWM_D.peripheral.$assign = "TIMG0"' },
    @{ Name = 'DIR_A'; Token = 'DIR_A.associatedPins[0].pin.$assign = "PB0"' },
    @{ Name = 'DIR_A2'; Token = 'DIR_A.associatedPins[1].pin.$assign = "PB1"' },
    @{ Name = 'DIR_B'; Token = 'DIR_B.associatedPins[0].pin.$assign = "PB4"' },
    @{ Name = 'DIR_B2'; Token = 'DIR_B.associatedPins[1].pin.$assign = "PB5"' },
    @{ Name = 'DIR_C'; Token = 'DIR_C.associatedPins[0].pin.$assign = "PB15"' },
    @{ Name = 'DIR_C2'; Token = 'DIR_C.associatedPins[1].pin.$assign = "PB16"' },
    @{ Name = 'DIR_D'; Token = 'DIR_D.associatedPins[0].pin.$assign = "PA0"' },
    @{ Name = 'DIR_D2'; Token = 'DIR_D.associatedPins[1].pin.$assign = "PA1"' },
    @{ Name = 'ENCODER_A'; Token = 'ENCODER_A.associatedPins[0].pin.$assign = "PA7"' },
    @{ Name = 'ENCODER_A2'; Token = 'ENCODER_A.associatedPins[1].pin.$assign = "PA8"' },
    @{ Name = 'ENCODER_B'; Token = 'ENCODER_B.associatedPins[0].pin.$assign = "PA9"' },
    @{ Name = 'ENCODER_B2'; Token = 'ENCODER_B.associatedPins[1].pin.$assign = "PA12"' },
    @{ Name = 'ENCODER_C'; Token = 'ENCODER_C.associatedPins[0].pin.$assign = "PA21"' },
    @{ Name = 'ENCODER_C2'; Token = 'ENCODER_C.associatedPins[1].pin.$assign = "PA24"' },
    @{ Name = 'ENCODER_D_A'; Token = 'ENCODER_D_A.associatedPins[0].pin.$assign = "PA28"' },
    @{ Name = 'ENCODER_D_B'; Token = 'ENCODER_D_B.associatedPins[0].pin.$assign = "PB23"' }
)) {
    Require-Token $syscfg $mapping.Token "$($mapping.Name) A/D SysConfig mapping"
}

foreach ($token in @(
    'CAR_MOTOR_A_PWM_TIMER', 'CAR_MOTOR_B_PWM_TIMER',
    'CAR_MOTOR_C_PWM_TIMER', 'CAR_MOTOR_D_PWM_TIMER',
    'CAR_MOTOR_A_DIRECTION_PORT', 'CAR_MOTOR_B_DIRECTION_PORT',
    'CAR_MOTOR_C_DIRECTION_PORT', 'CAR_MOTOR_D_DIRECTION_PORT',
    'CAR_MOTOR_A_ENCODER_A_PORT', 'CAR_MOTOR_A_ENCODER_B_PORT',
    'CAR_MOTOR_B_ENCODER_A_PORT', 'CAR_MOTOR_B_ENCODER_B_PORT',
    'CAR_MOTOR_C_ENCODER_A_PORT', 'CAR_MOTOR_C_ENCODER_B_PORT',
    'CAR_MOTOR_D_ENCODER_A_PORT', 'CAR_MOTOR_D_ENCODER_B_PORT'
)) {
    Require-Token $config $token "A/D config contract"
}

Require-Token $encoder 'BspEncoder_GPIOB_IRQHandler' 'GPIOB encoder interrupt handler'
Require-Token $encoder 'GPIOB_IRQHandler' 'GPIOB vector wrapper'
foreach ($token in @('BspPwm_SetDutyA', 'BspPwm_SetDutyB', 'BspPwm_SetDutyC', 'BspPwm_SetDutyD')) {
    Require-Token $pwm $token 'four-channel PWM API'
}
foreach ($token in @('BSP_GPIO_MOTOR_A', 'BSP_GPIO_MOTOR_B', 'BSP_GPIO_MOTOR_C', 'BSP_GPIO_MOTOR_D')) {
    Require-Token $gpio $token 'four-channel direction API'
}
foreach ($token in @('Tb6612Motor_SetDutyA', 'Tb6612Motor_SetDutyB', 'Tb6612Motor_SetDutyC', 'Tb6612Motor_SetDutyD')) {
    Require-Token $motor $token 'four-channel motor API'
}
foreach ($token in @('targetRpmA', 'targetRpmB', 'targetRpmC', 'targetRpmD', 'rpmA', 'rpmB', 'rpmC', 'rpmD')) {
    Require-Token $types $token 'four-channel application data'
}
foreach ($token in @('"MOTOR"', '"SPEED:"', '"BUZZER"', '"MODE"', '"LED"', '"COUNT:"', '"BRI:"')) {
    Require-Token $ui $token 'menu screen display and four-channel status'
}

Require-Token $readme 'independent A/B/C/D channel enable switches' 'direct channel menu documentation'
foreach ($token in @(
    'A, B, C, and D always name their corresponding software and D24A channels.',
    'Positive target RPM means vehicle forward for every channel.',
    'B/D encoder deltas and RPM values are sign-normalized'
)) {
    Require-Token $directionDesign $token 'direct motor channel contract'
}

Write-Host 'PASS: A/B/C/D port contracts are complete.'
