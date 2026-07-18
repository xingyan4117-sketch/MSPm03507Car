$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot
$syscfgFile = Join-Path $projectRoot 'generated\empty.syscfg'
$carConfigFile = Join-Path $projectRoot 'config\car_config.h'
$gpioHeaderFile = Join-Path $projectRoot 'bsp\gpio\bsp_gpio.h'
$gpioSourceFile = Join-Path $projectRoot 'bsp\gpio\bsp_gpio.c'

function Require-Regex([string] $Path, [string] $Pattern, [string] $Description) {
    $content = Get-Content -LiteralPath $Path -Raw
    if ($content -notmatch $Pattern) {
        throw "Missing ${Description} in $Path"
    }
}

foreach ($key in @(
    @{ Name = 'KEY1_INPUT'; Pin = 'PA14' },
    @{ Name = 'KEY2_INPUT'; Pin = 'PA15' },
    @{ Name = 'KEY3_INPUT'; Pin = 'PA16' }
)) {
    $block = [regex]::Escape($key.Name) + '\.\$name\s*=\s*"' + [regex]::Escape($key.Name) + '";[\s\S]*?' +
        [regex]::Escape($key.Name) + '\.associatedPins\[0\]\.direction\s*=\s*"INPUT";[\s\S]*?' +
        [regex]::Escape($key.Name) + '\.associatedPins\[0\]\.internalResistor\s*=\s*"PULL_UP";[\s\S]*?' +
        [regex]::Escape($key.Name) + '\.associatedPins\[0\]\.pin\.\$assign\s*=\s*"' + [regex]::Escape($key.Pin) + '";'
    Require-Regex $syscfgFile $block "$($key.Name) active-low pull-up input"
}


foreach ($macro in @('CAR_KEY1_PORT', 'CAR_KEY1_PIN', 'CAR_KEY2_PORT', 'CAR_KEY2_PIN', 'CAR_KEY3_PORT', 'CAR_KEY3_PIN')) {
    Require-Regex $carConfigFile "(?m)^\s*#define\s+$macro\b" "$macro hardware mapping"
}

foreach ($reader in @('BspGpio_ReadKey1Debounced', 'BspGpio_ReadKey2Debounced', 'BspGpio_ReadKey3Debounced')) {
    Require-Regex $gpioHeaderFile "\bbool\s+$reader\s*\(\s*void\s*\)\s*;" "$reader declaration"
}

foreach ($key in @(
    @{ State = 'g_key1Debounce'; Port = 'CAR_KEY1_PORT'; PinMacro = 'CAR_KEY1_PIN'; Reader = 'BspGpio_ReadKey1Debounced' },
    @{ State = 'g_key2Debounce'; Port = 'CAR_KEY2_PORT'; PinMacro = 'CAR_KEY2_PIN'; Reader = 'BspGpio_ReadKey2Debounced' },
    @{ State = 'g_key3Debounce'; Port = 'CAR_KEY3_PORT'; PinMacro = 'CAR_KEY3_PIN'; Reader = 'BspGpio_ReadKey3Debounced' }
)) {
    Require-Regex $gpioSourceFile "(?m)^\s*static\s+BspGpioDebounceState\s+$($key.State)\s*;" "$($key.State) state storage"
    Require-Regex $gpioSourceFile "BspGpio_InitDebounce\s*\(\s*&$($key.State)\s*,\s*$($key.Port)\s*,\s*$($key.PinMacro)\s*\)\s*;" "$($key.State) init wiring"
    Require-Regex $gpioSourceFile "bool\s+$($key.Reader)\s*\(\s*void\s*\)\s*\{[\s\S]*?return\s+BspGpio_ReadDebounced\s*\(\s*&$($key.State)\s*,\s*$($key.Port)\s*,\s*$($key.PinMacro)\s*\)\s*;[\s\S]*?\}" "$($key.Reader) independent debounce state"
}

Get-ChildItem -Path (Join-Path $projectRoot 'application'), (Join-Path $projectRoot 'module') -Recurse -File -Include *.c,*.h |
    ForEach-Object {
        if ((Get-Content -LiteralPath $_.FullName -Raw) -match '#include\s*[<\"](?:ti_msp_dl_config|dl_|driverlib)') {
            throw "Application/Module DriverLib include found in $($_.FullName)"
        }
    }

Write-Host 'PASS: expansion button contracts are complete.'
