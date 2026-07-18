$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot
$applicationRoot = Join-Path $projectRoot 'application'
$testsRoot = Join-Path $projectRoot 'tests'
$hostStubRoot = Join-Path $testsRoot 'host_stub'
$commandHeader = Join-Path $applicationRoot 'command_task.h'
$commandSource = Join-Path $applicationRoot 'command_task.c'
$appInitSource = Join-Path $applicationRoot 'app_init.c'
$projectFile = Join-Path $projectRoot 'keil\car_freertos_LP_MSPM0G3507.uvprojx'
$harnessSource = Join-Path $testsRoot 'command_button_harness.c'
$stubSource = Join-Path $hostStubRoot 'command_button_host_stub.c'

function Require-File([string] $Path, [string] $Description) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "Missing ${Description}: $Path"
    }
}

function Require-Regex([string] $Path, [string] $Pattern, [string] $Description) {
    if ((Get-Content -LiteralPath $Path -Raw) -notmatch $Pattern) {
        throw "Missing ${Description} in $Path"
    }
}

function Reject-Regex([string] $Path, [string] $Pattern, [string] $Description) {
    if ((Get-Content -LiteralPath $Path -Raw) -match $Pattern) {
        throw "Unexpected ${Description} in $Path"
    }
}

function Get-HostCompiler() {
    $candidate = 'D:\develop\vs2022\Professional\VC\Tools\MSVC\14.29.30133\bin\Hostx64\x64\cl.exe'
    if (-not (Test-Path -LiteralPath $candidate -PathType Leaf)) {
        throw 'MSVC host compiler was not found.'
    }
    return $candidate
}

foreach ($entry in @(
    @($commandHeader, 'CommandTask header'),
    @($commandSource, 'CommandTask source'),
    @($harnessSource, 'command button harness'),
    @($stubSource, 'command button host stub')
)) {
    Require-File $entry[0] $entry[1]
}

foreach ($constant in @('COMMAND_SPEED_STEP_RPM', 'COMMAND_START_RPM', 'COMMAND_MAX_RPM')) {
    Require-Regex $commandHeader ("(?m)^\s*#define\s+{0}\b" -f $constant) "$constant declaration"
}
Require-Regex $commandHeader 'void\s+CommandTask_ApplyButtonEvents\s*\(\s*AppMotorCommand\s*\*command\s*,\s*bool\s+speedUp\s*,\s*bool\s+speedDown\s*,\s*bool\s+toggleRun\s*\)\s*;' 'pure button function declaration'
Require-Regex $commandSource 'void\s+CommandTask_ApplyButtonEvents\s*\(' 'pure button function implementation'
foreach ($reader in @('BspGpio_ReadKey1Debounced', 'BspGpio_ReadKey2Debounced', 'BspGpio_ReadKey3Debounced')) {
    Require-Regex $commandSource ([regex]::Escape($reader)) "$reader command input"
}
Reject-Regex $commandSource 'BspGpio_ReadKeyDebounced' 'PB21 command-task handling'
Require-Regex $commandSource 'command\.issuedAtMs\s*=' 'command timestamp refresh'
Require-Regex $commandSource 'command\.sequence\s*\+\+' 'command sequence increment'
Require-Regex $commandSource 'App_PublishCommand\s*\(\s*&command\s*\)' 'command/safety queue publication'
Require-Regex $appInitSource 'xQueueOverwrite\(g_commandQueue, command\)' 'motor command queue overwrite'
Require-Regex $appInitSource 'xQueueOverwrite\(g_safetyCommandQueue, command\)' 'safety command queue overwrite'
Reject-Regex $commandSource '#include\s*[<"](?:ti_msp_dl_config|dl_|driverlib)' 'DriverLib dependency'
Reject-Regex $commandSource '\b(?:pvPortMalloc|malloc|calloc|realloc|free)\s*\(' 'dynamic allocation'
Reject-Regex $projectFile 'tests/(host_stub|command_button_harness)|tests\\(host_stub|command_button_harness)' 'host-only command test in Keil project'

$compiler = Get-HostCompiler
$artifactRoot = Join-Path $testsRoot 'artifacts\command_buttons'
New-Item -ItemType Directory -Force -Path $artifactRoot | Out-Null
$exePath = Join-Path $artifactRoot 'command_button_harness.exe'
$vsDevCmd = 'D:\develop\vs2022\Professional\Common7\Tools\VsDevCmd.bat'
$commandText = ('call "{0}" -arch=x64 -host_arch=x64 >nul && "{1}" /nologo /TC /W4 /I"{2}" /I"{3}" "{4}" "{5}" "{6}" /Fe:"{7}"' -f $vsDevCmd, $compiler, $hostStubRoot, $applicationRoot, $harnessSource, $stubSource, $commandSource, $exePath)
& cmd.exe /d /s /c $commandText
if ($LASTEXITCODE -ne 0) {
    throw 'Command button host harness compilation failed.'
}
& $exePath
if ($LASTEXITCODE -ne 0) {
    throw 'Command button host harness execution failed.'
}

Write-Host 'PASS: CommandTask button contracts are complete.'
