$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot

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

function Reject-Regex([string] $Path, [string] $Pattern, [string] $Description) {
    $content = Get-Content -LiteralPath $Path -Raw
    if ($content -match $Pattern) {
        throw "Unexpected ${Description} in $Path"
    }
}

function Format-Hex32([uint32] $Value) {
    return ('0x{0:X8}' -f $Value)
}

function Require-Equal([string] $Description, $Actual, $Expected) {
    if ($Actual -ne $Expected) {
        throw "Expected ${Description} to be '$Expected' but was '$Actual'"
    }
}

function Require-HexEqual([string] $Description, [uint32] $Actual, [uint32] $Expected) {
    if ($Actual -ne $Expected) {
        throw "Expected ${Description} to be $(Format-Hex32 $Expected) but was $(Format-Hex32 $Actual)"
    }
}

function Get-GpioPinNumber([string] $Path, [string] $MacroName) {
    $content = Get-Content -LiteralPath $Path -Raw
    $match = [regex]::Match($content, "(?m)^\s*#define\s+$([regex]::Escape($MacroName))\s+\(DL_GPIO_PIN_(\d+)\)")
    if (-not $match.Success) {
        throw "Missing GPIO pin define '$MacroName' in $Path"
    }

    return [int] $match.Groups[1].Value
}

function Get-EncoderPolarityMask([uint32] $Pins, [int] $FirstPin) {
    $polarity = [uint32] 0

    for ($pin = $FirstPin; $pin -lt ($FirstPin + 16); $pin++) {
        $pinMask = ([uint32] 1 -shl $pin)
        if (($Pins -band $pinMask) -ne 0) {
            $shift = ($pin - $FirstPin) * 2
            $polarity = [uint32] ($polarity -bor ([uint32] (0x03 -shl $shift)))
        }
    }

    return $polarity
}

$configFile = Join-Path $projectRoot 'config\car_config.h'
$syscfgFile = Join-Path $projectRoot 'generated\empty.syscfg'
$generatedHeader = Join-Path $projectRoot 'generated\ti_msp_dl_config.h'
$projectFile = Join-Path $projectRoot 'keil\car_freertos_LP_MSPM0G3507.uvprojx'
$startupFile = Join-Path $projectRoot 'keil\startup_mspm0g350x_uvision.s'
$mapFile = Join-Path $projectRoot 'keil\car_freertos_LP_MSPM0G3507.map'
$mainFile = Join-Path $projectRoot 'main.c'
$pwmHeader = Join-Path $projectRoot 'bsp\pwm\bsp_pwm.h'
$pwmSource = Join-Path $projectRoot 'bsp\pwm\bsp_pwm.c'
$gpioHeader = Join-Path $projectRoot 'bsp\gpio\bsp_gpio.h'
$gpioSource = Join-Path $projectRoot 'bsp\gpio\bsp_gpio.c'
$encoderHeader = Join-Path $projectRoot 'bsp\encoder\bsp_encoder.h'
$encoderSource = Join-Path $projectRoot 'bsp\encoder\bsp_encoder.c'
$timeHeader = Join-Path $projectRoot 'bsp\time\bsp_time.h'
$timeSource = Join-Path $projectRoot 'bsp\time\bsp_time.c'
$spiHeader = Join-Path $projectRoot 'bsp\spi\bsp_spi.h'
$spiSource = Join-Path $projectRoot 'bsp\spi\bsp_spi.c'
$logHeader = Join-Path $projectRoot 'bsp\log\bsp_log.h'
$logSource = Join-Path $projectRoot 'bsp\log\bsp_log.c'
$freertosSource = Join-Path $projectRoot 'bsp\freertos\bsp_freertos.c'

foreach ($entry in @(
    @($configFile, 'car resource contract'), @($syscfgFile, 'SysConfig source'),
    @($generatedHeader, 'generated GPIO header'),
    @($projectFile, 'Keil project'), @($startupFile, 'startup assembly'), @($mainFile, 'main entry'),
    @($mapFile, 'Keil map file'), @($pwmHeader, 'PWM BSP header'), @($pwmSource, 'PWM BSP source'),
    @($gpioHeader, 'GPIO BSP header'), @($gpioSource, 'GPIO BSP source'),
    @($encoderHeader, 'encoder BSP header'), @($encoderSource, 'encoder BSP source'),
    @($timeHeader, 'time BSP header'), @($timeSource, 'time BSP source'),
    @($spiHeader, 'SPI BSP header'), @($spiSource, 'SPI BSP source'),
    @($logHeader, 'log BSP header'), @($logSource, 'log BSP source')
)) {
    Require-File $entry[0] $entry[1]
}

foreach ($token in @(
    'CAR_MOTOR_B_PWM_TIMER', 'CAR_MOTOR_C_PWM_TIMER', 'CAR_MOTOR_B_ENCODER_PORT',
    'CAR_MOTOR_C_ENCODER_PORT', 'CAR_ENCODER_INTERRUPT_PORT',
    'CAR_ENCODER_INTERRUPT_PINS', 'CAR_ENCODER_MOTOR_B_PINS_MASK',
    'CAR_ENCODER_MOTOR_C_PINS_MASK', 'CAR_KEY_PORT', 'CAR_HEARTBEAT_PORT',
    'CAR_ST7735_SPI_INSTANCE', 'CAR_ST7735_DC_PIN', 'CAR_ST7735_CS_PIN',
    'CAR_ST7735_RES_PIN', 'CAR_ST7735_BLK_PIN', 'CAR_PWM_PERIOD_TICKS'
)) {
    Require-Token $configFile $token
}

foreach ($token in @(
    'GPIO_HEARTBEAT', 'KEY_INPUT', 'DIR_B', 'DIR_C', 'ENCODER_B', 'ENCODER_C',
    'PWM_B', 'PWM_C', 'SPI_LCD', 'PB13', 'PA17', 'PA9', 'PA12', 'PA21', 'PA24',
    'PB21', 'PB22', 'PB8', 'PB9', 'PB10', 'PB11', 'PB14', 'PB26'
)) {
    Require-Token $syscfgFile $token
}

foreach ($api in @(
    'BspPwm_SetDutyB', 'BspPwm_SetDutyC', 'BspPwm_StopAll',
    'BspGpio_SetMotorDirection', 'BspGpio_ReadKeyDebounced', 'BspGpio_SetHeartbeat',
    'BspGpio_DisplaySetChipSelect', 'BspEncoder_ReadAndClearB', 'BspEncoder_ReadAndClearC',
    'BspEncoder_GPIOA_IRQHandler', 'BspTime_GetMs', 'BspSpi_Write', 'BspLog_Write'
)) {
    $headers = @($pwmHeader, $gpioHeader, $encoderHeader, $timeHeader, $spiHeader, $logHeader)
    if (-not (($headers | ForEach-Object { Get-Content -Raw $_ }) -match [regex]::Escape($api))) {
        throw "Missing BSP API declaration '$api'"
    }
}

Require-Regex $pwmSource 'BspPwm_ClampDuty|clamp' 'PWM clamp helper'
Require-Token $pwmSource 'DL_TimerA_stopCounter'
Require-Token $pwmSource 'DL_TimerA_setCaptureCompareValue'
Require-Token $startupFile 'GROUP1_IRQHandler'
Require-Regex $startupFile 'DCD\s+GROUP1_IRQHandler' 'GROUP1 vector entry'
Require-Regex $startupFile 'EXPORT\s+GROUP1_IRQHandler\s+\[WEAK\]' 'weak GROUP1 export'
Require-Token $mainFile '#include "bsp_encoder.h"'
Require-Regex $mainFile 'int\s+main\s*\(\s*void\s*\)\s*\{[\s\S]*SYSCFG_DL_init\s*\(\s*\)\s*;[\s\S]*BspPwm_Init\s*\(\s*\)\s*;[\s\S]*BspGpio_Init\s*\(\s*\)\s*;[\s\S]*BspEncoder_Init\s*\(\s*\)\s*;[\s\S]*if\s*\(\s*!\s*BspFreeRTOS_Init\s*\(\s*\)\s*\|\|\s*!\s*App_Init\s*\(\s*\)\s*\)' 'main init order through BspEncoder_Init before scheduler setup'
Require-Regex $mainFile 'int\s+main\s*\(\s*void\s*\)\s*\{[\s\S]*BspEncoder_Init\s*\(\s*\)\s*;[\s\S]*vTaskStartScheduler\s*\(\s*\)\s*;' 'BspEncoder_Init before scheduler start'
Require-Token $encoderSource 'GROUP1_IRQHandler'
Require-Token $encoderSource 'GPIOA_IRQHandler'
Require-Token $encoderSource 'DL_Interrupt_getPendingGroup'
Require-Token $encoderSource 'DL_INTERRUPT_GROUP_1'
Require-Token $encoderSource 'DL_INTERRUPT_GROUP1_IIDX_GPIOA'
Require-Token $encoderSource 'DL_GPIO_setLowerPinsPolarity'
Require-Token $encoderSource 'DL_GPIO_setUpperPinsPolarity'
Require-Token $encoderSource 'CAR_ENCODER_INTERRUPT_PORT'
Require-Token $encoderSource 'CAR_ENCODER_INTERRUPT_PINS'
Require-Token $encoderSource 'DL_GPIO_getEnabledInterruptStatus'
Require-Token $encoderSource 'invalidTransitions'
Require-Regex $encoderSource 'uint32_t\s+lowerPins\s*=\s*\(CAR_ENCODER_INTERRUPT_PINS\s*&\s*0x0000FFFFU\)' 'encoder lower interrupt split'
Require-Regex $encoderSource 'uint32_t\s+upperPins\s*=\s*\(CAR_ENCODER_INTERRUPT_PINS\s*&\s*0xFFFF0000U\)' 'encoder upper interrupt split'
Require-Regex $encoderSource 'DL_GPIO_setLowerPinsPolarity\s*\(\s*CAR_ENCODER_INTERRUPT_PORT\s*,[\s\S]*BspEncoder_BuildPolarityMask\s*\(\s*lowerPins\s*,\s*0U\s*\)\s*\)' 'lower-pin both-edge polarity setup'
Require-Regex $encoderSource 'DL_GPIO_setUpperPinsPolarity\s*\(\s*CAR_ENCODER_INTERRUPT_PORT\s*,[\s\S]*BspEncoder_BuildPolarityMask\s*\(\s*upperPins\s*,\s*16U\s*\)\s*\)' 'upper-pin both-edge polarity setup'
Require-Token $timeSource 'xTaskGetTickCount'
Require-Token $spiSource 'DL_SPI_transmitData8'
Require-Regex $logSource 'xQueueSend\s*\([^\r\n]*,\s*0\s*\)' 'non-blocking log queue send'
Require-Token $freertosSource 'xQueueCreateStatic'

Require-Token $syscfgFile 'ENCODER_B.associatedPins[0].pin.$assign = "PA9"'
Require-Token $syscfgFile 'ENCODER_B.associatedPins[1].pin.$assign = "PA12"'
Require-Token $syscfgFile 'ENCODER_C.associatedPins[0].pin.$assign = "PA21"'
Require-Token $syscfgFile 'ENCODER_C.associatedPins[1].pin.$assign = "PA24"'

$encoderBPinA = Get-GpioPinNumber $generatedHeader 'ENCODER_B_E2A_PIN'
$encoderBPinB = Get-GpioPinNumber $generatedHeader 'ENCODER_B_E2B_PIN'
$encoderCPinA = Get-GpioPinNumber $generatedHeader 'ENCODER_C_E3A_PIN'
$encoderCPinB = Get-GpioPinNumber $generatedHeader 'ENCODER_C_E3B_PIN'

Require-Equal 'ENCODER_B_E2A pin number' $encoderBPinA 9
Require-Equal 'ENCODER_B_E2B pin number' $encoderBPinB 12
Require-Equal 'ENCODER_C_E3A pin number' $encoderCPinA 21
Require-Equal 'ENCODER_C_E3B pin number' $encoderCPinB 24

$encoderMaskB = [uint32] ((([uint32] 1 -shl $encoderBPinA) -bor ([uint32] 1 -shl $encoderBPinB)))
$encoderMaskC = [uint32] ((([uint32] 1 -shl $encoderCPinA) -bor ([uint32] 1 -shl $encoderCPinB)))
$encoderInterruptPins = [uint32] ($encoderMaskB -bor $encoderMaskC)
$lowerEncoderPins = [uint32] ($encoderInterruptPins -band 0x0000FFFF)
$upperEncoderPins = [uint32] ($encoderInterruptPins -band 0xFFFF0000)
$lowerEncoderPolarity = Get-EncoderPolarityMask $lowerEncoderPins 0
$upperEncoderPolarity = Get-EncoderPolarityMask $upperEncoderPins 16

Require-HexEqual 'encoder motor B mask' $encoderMaskB 0x00001200
Require-HexEqual 'encoder motor C mask' $encoderMaskC 0x01200000
Require-HexEqual 'encoder interrupt aggregate mask' $encoderInterruptPins 0x01201200
Require-HexEqual 'encoder lower-pin split mask' $lowerEncoderPins 0x00001200
Require-HexEqual 'encoder upper-pin split mask' $upperEncoderPins 0x01200000
Require-HexEqual 'encoder lower-pin both-edge polarity mask' $lowerEncoderPolarity 0x030C0000
Require-HexEqual 'encoder upper-pin both-edge polarity mask' $upperEncoderPolarity 0x00030C00

foreach ($header in @($pwmHeader, $gpioHeader, $encoderHeader, $timeHeader, $spiHeader, $logHeader)) {
    $content = Get-Content -Raw $header
    if ($content -match 'ti/driverlib|driverlib\.h') {
        throw "DriverLib leaked through BSP header: $header"
    }
}

foreach ($directory in @('module', 'application')) {
    $path = Join-Path $projectRoot $directory
    if (Test-Path -LiteralPath $path) {
        $matches = Get-ChildItem -LiteralPath $path -Recurse -File -Include '*.c', '*.h' |
            Select-String -Pattern 'ti/driverlib|driverlib\.h'
        if ($null -ne $matches) {
            throw "DriverLib include outside BSP/generated source: $($matches[0].Path)"
        }
    }
}

$bspSources = @(
    '../bsp/pwm/bsp_pwm.c', '../bsp/gpio/bsp_gpio.c', '../bsp/encoder/bsp_encoder.c',
    '../bsp/time/bsp_time.c', '../bsp/spi/bsp_spi.c', '../bsp/log/bsp_log.c',
    '../bsp/freertos/bsp_freertos.c'
)
foreach ($source in $bspSources) {
    Require-Token $projectFile $source
}
foreach ($includePath in @('../bsp/encoder', '../bsp/time', '../bsp/spi', '../bsp/log')) {
    Require-Token $projectFile $includePath
}

$newBspSources = @($pwmSource, $gpioSource, $encoderSource, $timeSource, $spiSource, $logSource)
foreach ($source in $newBspSources) {
    if ((Get-Content -Raw $source) -match '\b(malloc|calloc|realloc|free)\s*\(') {
        throw "Dynamic allocation found in BSP source: $source"
    }
}

Require-Regex $mapFile 'startup_mspm0g350x_uvision\.o\(RESET\) refers to bsp_encoder\.o\(.text\.GROUP1_IRQHandler\) for GROUP1_IRQHandler' 'GROUP1 startup reference'
Require-Regex $mapFile 'bsp_encoder\.o\(.text\.GROUP1_IRQHandler\) refers to bsp_encoder\.o\(.text\.(GPIOA_IRQHandler|BspEncoder_GPIOA_IRQHandler)\) for (GPIOA_IRQHandler|BspEncoder_GPIOA_IRQHandler)' 'GROUP1 to encoder ISR bridge'
Require-Regex $mapFile 'main\.o\(.text\.main\) refers to bsp_encoder\.o\(.text\.BspEncoder_Init\) for BspEncoder_Init' 'main to BspEncoder_Init call chain'
Require-Regex $mapFile 'BspEncoder_Init\s+0x[0-9a-fA-F]+\s+Thumb Code' 'retained BspEncoder_Init symbol'
Require-Regex $mapFile 'BspEncoder_GPIOA_IRQHandler\s+0x[0-9a-fA-F]+\s+Thumb Code' 'retained BSP encoder ISR symbol'
Reject-Regex $mapFile 'Removing bsp_encoder\.o\(.text\.GROUP1_IRQHandler\)' 'removed GROUP1 handler'
Reject-Regex $mapFile 'Removing bsp_encoder\.o\(.text\.BspEncoder_Init\)' 'removed BspEncoder_Init executable code section'
Reject-Regex $mapFile 'Removing bsp_encoder\.o\(.text\.BspEncoder_GPIOA_IRQHandler\)' 'removed BSP encoder handler'

$mapContent = Get-Content -LiteralPath $mapFile -Raw
if ($mapContent -match 'Removing bsp_encoder\.o\(\.ARM\.exidx\.text\.BspEncoder_Init\)') {
    Write-Host 'NOTE: map trims .ARM.exidx.text.BspEncoder_Init unwind metadata while retaining .text.BspEncoder_Init executable code.'
}

Write-Host 'PASS: Task 2 BSP contracts are complete.'
