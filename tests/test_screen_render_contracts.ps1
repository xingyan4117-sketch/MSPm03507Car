$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot
$testsRoot = Join-Path $projectRoot 'tests'
$moduleRoot = Join-Path $projectRoot 'module'
$applicationRoot = Join-Path $projectRoot 'application'
$hostStubRoot = Join-Path $testsRoot 'host_stub'
$uiSource = Join-Path $moduleRoot 'st7735_ui.c'
$uiHeader = Join-Path $moduleRoot 'st7735_ui.h'
$uiTaskSource = Join-Path $applicationRoot 'ui_task.c'
$uiTaskHeader = Join-Path $applicationRoot 'ui_task.h'
$motorControlSource = Join-Path $applicationRoot 'motor_control_task.c'
$harnessSource = Join-Path $testsRoot 'screen_render_harness.c'
$stubSource = Join-Path $hostStubRoot 'bsp_spi.c'
$stubHeader = Join-Path $hostStubRoot 'bsp_spi.h'

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

foreach ($entry in @(
    @($uiSource, 'ST7735 UI source'), @($uiHeader, 'ST7735 UI header'),
    @($uiTaskSource, 'UI task source'), @($harnessSource, 'screen render harness'),
    @($stubSource, 'screen SPI stub'), @($stubHeader, 'screen SPI stub header')
)) { Require-File $entry[0] $entry[1] }

foreach ($api in @('St7735Ui_Init', 'St7735Ui_Clear', 'St7735Ui_RenderStatus', 'St7735Ui_GetLastFrame')) {
    Require-Regex $uiHeader ("\b{0}\s*\(" -f $api) "$api declaration"
}
Require-Regex $uiTaskHeader 'UI_TASK_PERIOD_MS\s*\(100U\)' '10 Hz UI period'
Require-Regex $uiSource 'BspSpi_SetReset\s*\(' 'ST7735 reset control'
Require-Regex $uiSource 'BspSpi_DelayMs\s*\(' 'ST7735 reset/init delays'
Require-Regex $uiHeader 'ST7735_UI_RENDER_HEIGHT\s*\(160U\)' 'full-height display render contract'
Require-Regex $uiSource 'ST7735_UI_GLYPH_SCALE\s*\(2U\)' '2x glyph scale'
Require-Regex $uiSource 'St7735Ui_AppendModeLine' 'selection/edit mode status display'
Require-Regex $uiSource '"SEL"' 'selection-mode status display'
Require-Regex $uiSource '"EDIT"' 'edit-mode status display'
Require-Regex $uiSource 'glyph_z' 'Z glyph for BUZZER'
Require-Regex $uiSource '"GRN"' 'compact color display'
Reject-Regex $motorControlSource 'BspSpi_|St7735Ui_' 'MotorControl SPI/display dependency'
Reject-Regex $uiSource 'BspSpi_Write\s*\(\s*true\s*,\s*\(const\s+uint8_t\s*\*\)\s*g_lastFrame' 'raw ASCII pixel transmission'

$cl = 'D:\develop\vs2022\Professional\VC\Tools\MSVC\14.29.30133\bin\Hostx64\x64\cl.exe'
$vsDevCmd = 'D:\develop\vs2022\Professional\Common7\Tools\VsDevCmd.bat'
if (-not (Test-Path -LiteralPath $cl -PathType Leaf) -or -not (Test-Path -LiteralPath $vsDevCmd -PathType Leaf)) {
    throw 'Screen renderer host test requires the established VS2022 x64 C compiler environment.'
}

$artifactRoot = Join-Path $testsRoot 'artifacts\screen_render'
New-Item -ItemType Directory -Force -Path $artifactRoot | Out-Null
Remove-Item -LiteralPath (Join-Path $artifactRoot '*') -Force -ErrorAction SilentlyContinue
$exe = Join-Path $artifactRoot 'screen_render_harness.exe'
$command = @(
    ('call "{0}" -arch=x64 -host_arch=x64 >nul' -f $vsDevCmd),
    ('"{0}" /nologo /TC /W4 /I"{1}" /I"{2}" /I"{3}" "{4}" "{5}" "{6}" /Fe:"{7}"' -f $cl, $hostStubRoot, $moduleRoot, $applicationRoot, $harnessSource, $stubSource, $uiSource, $exe)
) -join ' && '
$compile = & cmd.exe /d /s /c $command 2>&1
if ($LASTEXITCODE -ne 0) { throw "Screen renderer harness compilation failed.`n$($compile -join [Environment]::NewLine)" }
$run = & $exe 2>&1
if ($LASTEXITCODE -ne 0) { throw "Screen renderer harness failed.`n$($run -join [Environment]::NewLine)" }
$run | ForEach-Object { Write-Host $_ }
Write-Host 'PASS: ST7735S screen rendering contracts are complete.'
