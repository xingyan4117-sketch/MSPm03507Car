$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$syscfg = Join-Path $root 'generated\empty.syscfg'
$config = Join-Path $root 'config\car_config.h'
$buzzer = Join-Path $root 'bsp\buzzer\bsp_buzzer.h'
$ws2812 = Join-Path $root 'bsp\ws2812\bsp_ws2812.h'
$command = Join-Path $root 'application\command_task.c'
$types = Join-Path $root 'application\app_types.h'
$init = Join-Path $root 'application\app_init.c'
$ui = Join-Path $root 'module\st7735_ui.c'

function Require-Token([string] $path, [string] $token, [string] $description) {
    if ((Get-Content -LiteralPath $path -Raw) -notmatch [regex]::Escape($token)) {
        throw "Missing ${description}: $token"
    }
}

Require-Token $syscfg 'BUZZER.associatedPins[0].pin.$assign = "PB27"' 'PB27 buzzer SysConfig mapping'
Require-Token $syscfg 'WS2812.associatedPins[0].pin.$assign = "PA29"' 'PA29 WS2812 SysConfig mapping'
Require-Token $config 'CAR_BUZZER_PORT' 'buzzer configuration contract'
Require-Token $config 'CAR_WS2812_PORT' 'WS2812 configuration contract'
foreach ($token in @('BspBuzzer_SetConfig', 'BspBuzzer_Tick', 'BspBuzzer_Stop')) {
    Require-Token $buzzer $token 'buzzer BSP API'
}
foreach ($token in @('BspWs2812_SetConfig', 'BspWs2812_Tick', 'BspWs2812_Init')) {
    Require-Token $ws2812 $token 'WS2812 BSP API'
}
foreach ($token in @('menuItem', 'menuLevel', 'menuSubItem', 'editMode', 'ledColor', 'ledBrightness')) {
    Require-Token $command $token 'menu command state'
}
foreach ($token in @('buzzerMode', 'buzzerVolume', 'ledCount', 'ledColor[3]', 'APP_BUZZER_MODE_FAST')) {
    Require-Token $types $token 'expanded menu data contract'
}
foreach ($token in @('buzzerMode', 'buzzerVolume', 'ledCount', 'ledBrightness')) {
    Require-Token $init $token 'expanded default command state'
}
foreach ($token in @('MOTOR', 'SPEED', 'BUZZER', 'MODE', 'LED', 'COUNT', 'LED1', 'LED2', 'LED3', 'BRI')) {
    Require-Token $ui $token 'menu display label'
}

Write-Host 'PASS: menu, buzzer, and WS2812 contracts are complete.'
