$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$source = Join-Path $root 'bsp\ws2812\bsp_ws2812.c'
$header = Join-Path $root 'bsp\ws2812\bsp_ws2812.h'

function Require-Token([string] $path, [string] $token, [string] $description) {
    if ((Get-Content -LiteralPath $path -Raw) -notmatch [regex]::Escape($token)) {
        throw "Missing ${description}: $token"
    }
}

foreach ($token in @('BspWs2812_SetConfig', 'const uint8_t colors[3]', 'ledCount',
    'index >= g_ledCount', 'g_ledColors[index]', 'Ws2812_Scale')) {
    Require-Token $source $token 'WS2812 independent LED frame contract'
}
foreach ($token in @('WS2812_FRAME_BITS', 'WS2812_PWM_PERIOD_TICKS',
    'WS2812_T0H_TICKS', 'WS2812_T1H_TICKS', 'DL_DMA_setTransferSize',
    'DL_TimerG_startCounter', 'DL_DMA_getTransferSize')) {
    Require-Token $source $token 'WS2812B hardware-timed DMA contract'
}
if ((Get-Content -LiteralPath $source -Raw) -match 'Ws2812_DelayCycles|Ws2812_SendBit') {
    throw 'WS2812 output must not depend on compiler-generated delay loops.'
}
foreach ($token in @('BspWs2812_SetConfig', 'BspWs2812_Tick', 'BspWs2812_Off')) {
    Require-Token $header $token 'WS2812 API'
}

Write-Host 'PASS: WS2812 count, brightness, and independent-color contracts are complete.'
