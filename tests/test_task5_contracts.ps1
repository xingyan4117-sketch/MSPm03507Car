$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot
$testsRoot = Join-Path $projectRoot 'tests'
$moduleRoot = Join-Path $projectRoot 'module'
$applicationRoot = Join-Path $projectRoot 'application'
$hostStubRoot = Join-Path $testsRoot 'host_stub'
$projectFile = Join-Path $projectRoot 'keil\car_freertos_LP_MSPM0G3507.uvprojx'
$appTypesHeader = Join-Path $applicationRoot 'app_types.h'
$appInitSource = Join-Path $applicationRoot 'app_init.c'
$bspSpiHeader = Join-Path $projectRoot 'bsp\spi\bsp_spi.h'
$bspSpiSource = Join-Path $projectRoot 'bsp\spi\bsp_spi.c'

function Require-File([string] $Path, [string] $Description) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "Missing ${Description}: $Path"
    }
}

function Require-Token([string] $Path, [string] $Token) {
    if ((Get-Content -LiteralPath $Path -Raw) -notmatch [regex]::Escape($Token)) {
        throw "Missing token '$Token' in $Path"
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

$requiredFiles = @(
    @((Join-Path $moduleRoot 'safety.h'), 'safety header'),
    @((Join-Path $moduleRoot 'safety.c'), 'safety source'),
    @((Join-Path $moduleRoot 'st7735_ui.h'), 'ST7735 UI header'),
    @((Join-Path $moduleRoot 'st7735_ui.c'), 'ST7735 UI source'),
    @((Join-Path $applicationRoot 'command_task.h'), 'command task header'),
    @((Join-Path $applicationRoot 'command_task.c'), 'command task source'),
    @((Join-Path $applicationRoot 'safety_task.h'), 'safety task header'),
    @((Join-Path $applicationRoot 'safety_task.c'), 'safety task source'),
    @((Join-Path $applicationRoot 'ui_task.h'), 'UI task header'),
    @((Join-Path $applicationRoot 'ui_task.c'), 'UI task source'),
    @((Join-Path $applicationRoot 'log_task.h'), 'log task header'),
    @((Join-Path $applicationRoot 'log_task.c'), 'log task source'),
    @((Join-Path $testsRoot 'task5_behavior_harness.c'), 'Task 5 behavior harness')
)
foreach ($entry in $requiredFiles) { Require-File $entry[0] $entry[1] }

$safetyHeader = Join-Path $moduleRoot 'safety.h'
$safetySource = Join-Path $moduleRoot 'safety.c'
$uiHeader = Join-Path $moduleRoot 'st7735_ui.h'
$uiSource = Join-Path $moduleRoot 'st7735_ui.c'
$safetyTaskSource = Join-Path $applicationRoot 'safety_task.c'

foreach ($token in @('SafetyController', 'SafetyInput', 'Safety_Advance', 'Safety_StateIsUnsafe')) {
    Require-Token $safetyHeader $token
}
Require-Regex $appTypesHeader 'typedef\s+struct\s*\{[\s\S]*uint8_t\s+clearRequest\s*;[\s\S]*uint8_t\s+externalFault\s*;' 'AppMotorCommand clear/fault input fields'
Require-Regex $appInitSource 'AppMotorCommand\s+command\s*=\s*\{[\s\S]*\.clearRequest\s*=\s*0U[\s\S]*\.externalFault\s*=\s*0U' 'safe default clear/fault command initialization'
foreach ($token in @('APP_STATE_INIT', 'APP_STATE_READY', 'APP_STATE_RUN', 'APP_STATE_ESTOP', 'APP_STATE_FAULT')) {
    Require-Token $safetySource $token
}
foreach ($token in @('St7735Ui_Init', 'St7735Ui_Clear', 'St7735Ui_RenderStatus', 'St7735Ui_GetLastFrame')) {
    Require-Token $uiHeader $token
}
Require-Token $uiSource 'ST7735_UI_FRAME_BYTES'
Require-Token $uiSource 'BspSpi_Write'
Require-Token $bspSpiHeader 'BspSpi_SetBacklight'
Require-Regex $bspSpiSource 'void\s+BspSpi_SetBacklight\s*\(\s*bool\s+enabled\s*\)\s*\{[\s\S]*BspGpio_DisplaySetBacklight\s*\(\s*enabled\s*\)\s*;' 'SPI backlight wrapper'
Require-Token $uiSource 'BspSpi_SetBacklight(false)'
Require-Token $uiSource 'BspSpi_SetBacklight(true)'
Require-Token $safetyTaskSource 'Tb6612Motor_StopCoast'
Require-Token $safetyTaskSource 'App_PublishStatus'
Require-Token $safetyTaskSource 'App_PublishSafetyStatus'
Require-Regex $safetyTaskSource 'Tb6612Motor_StopCoast\s*\(\s*\)\s*;[\s\S]*App_PublishSafetyStatus\s*\(\s*&published\s*\)' 'stop before safety-status publication'
Require-Regex $safetyTaskSource 'SafetyTask_ProcessCycle\s*\(\s*&controller\s*,\s*&command\s*,\s*&motorStatus\s*,[\s\S]*command\.externalFault\s*!=\s*0U[\s\S]*command\.clearRequest\s*!=\s*0U' 'SafetyTask live clear/fault command inputs'
Reject-Regex $safetyTaskSource 'SafetyTask_ProcessCycle\s*\(\s*&controller\s*,\s*&command\s*,\s*&motorStatus\s*,[\s\S]*BspGpio_ReadKeyDebounced\s*\(\s*\)\s*,\s*false\s*,\s*false\s*,' 'hard-coded false clear/fault inputs'
Require-Token (Join-Path $applicationRoot 'safety_task.h') 'SAFETY_TASK_PERIOD_MS (10U)'
Require-Token (Join-Path $applicationRoot 'command_task.h') 'COMMAND_TASK_PERIOD_MS (20U)'
Require-Token (Join-Path $applicationRoot 'ui_task.h') 'UI_TASK_PERIOD_MS (100U)'
Require-Token (Join-Path $applicationRoot 'log_task.h') 'LOG_TASK_PRIORITY (0U)'
Require-Token $appInitSource 'xQueueCreateStatic'
Require-Token $appInitSource 'xTaskCreateStatic'
foreach ($file in @($safetyHeader, $safetySource, $uiHeader, $uiSource,
        (Join-Path $applicationRoot 'command_task.c'), $safetyTaskSource,
        (Join-Path $applicationRoot 'ui_task.c'), (Join-Path $applicationRoot 'log_task.c'), $appInitSource)) {
    Reject-Regex $file '\b(?:malloc|calloc|realloc|free)\s*\(|\bxTaskCreate\s*\(|\bxQueueCreate\s*\(' 'dynamic allocation API'
}
foreach ($source in @('../module/safety.c', '../module/st7735_ui.c', '../application/command_task.c',
        '../application/safety_task.c', '../application/ui_task.c', '../application/log_task.c')) {
    Require-Token $projectFile $source
}

$cl = 'D:\develop\vs2022\Professional\VC\Tools\MSVC\14.29.30133\bin\Hostx64\x64\cl.exe'
$vsDevCmd = 'D:\develop\vs2022\Professional\Common7\Tools\VsDevCmd.bat'
if (-not (Test-Path -LiteralPath $cl -PathType Leaf) -or -not (Test-Path -LiteralPath $vsDevCmd -PathType Leaf)) {
    throw 'Task 5 host harness requires the established VS2022 x64 C compiler environment.'
}

$artifactRoot = Join-Path $testsRoot 'artifacts\task5_behavior'
$null = New-Item -ItemType Directory -Path $artifactRoot -Force
Remove-Item -LiteralPath (Join-Path $artifactRoot '*') -Force -ErrorAction SilentlyContinue
$exe = Join-Path $artifactRoot 'task5_behavior_harness.exe'
$sources = @(
    (Join-Path $testsRoot 'task5_behavior_harness.c'),
    (Join-Path $moduleRoot 'safety.c'),
    (Join-Path $moduleRoot 'st7735_ui.c'),
    (Join-Path $applicationRoot 'safety_task.c')
) | ForEach-Object { '"{0}"' -f $_ }
$command = @(
    ('call "{0}" -arch=x64 -host_arch=x64 >nul' -f $vsDevCmd),
    ('"{0}" /nologo /TC /W4 /I"{1}" /I"{2}" /I"{3}" {4} /Fe:"{5}"' -f $cl, $hostStubRoot, $moduleRoot, $applicationRoot, ($sources -join ' '), $exe)
) -join ' && '
$compile = & cmd.exe /d /s /c $command 2>&1
if ($LASTEXITCODE -ne 0) { throw "Task 5 host harness compilation failed.`n$($compile -join [Environment]::NewLine)" }
$run = & $exe 2>&1
if ($LASTEXITCODE -ne 0) { throw "Task 5 host harness failed.`n$($run -join [Environment]::NewLine)" }
$run | ForEach-Object { Write-Host $_ }
Write-Host 'PASS: Task 5 safety/UI/task contracts are complete.'
