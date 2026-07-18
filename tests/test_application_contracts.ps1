$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot
$applicationRoot = Join-Path $projectRoot 'application'
$testsRoot = Join-Path $projectRoot 'tests'
$projectFile = Join-Path $projectRoot 'keil\car_freertos_LP_MSPM0G3507.uvprojx'

function Require-File([string] $Path, [string] $Description) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "Missing ${Description}: $Path"
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

function Get-HostCompiler() {
    foreach ($candidate in @(
        (Get-Command clang -ErrorAction SilentlyContinue).Source,
        (Get-Command gcc -ErrorAction SilentlyContinue).Source,
        'D:\develop\vs2022\Professional\VC\Tools\MSVC\14.29.30133\bin\Hostx64\x64\cl.exe'
    )) {
        if ($candidate -and (Test-Path -LiteralPath $candidate -PathType Leaf)) {
            return $candidate
        }
    }
    throw 'No supported host C compiler was found.'
}

$typesHeader = Join-Path $applicationRoot 'app_types.h'
$initHeader = Join-Path $applicationRoot 'app_init.h'
$initSource = Join-Path $applicationRoot 'app_init.c'
$taskHeader = Join-Path $applicationRoot 'motor_control_task.h'
$taskSource = Join-Path $applicationRoot 'motor_control_task.c'
$harnessSource = Join-Path $testsRoot 'application_behavior_harness.c'
$freeRtosHeader = Join-Path $testsRoot 'host_stub\FreeRTOS.h'
$queueHeader = Join-Path $testsRoot 'host_stub\queue.h'
$taskHeaderStub = Join-Path $testsRoot 'host_stub\task.h'
$appStubSource = Join-Path $testsRoot 'host_stub\application_host_stub.c'

foreach ($entry in @(
    @($typesHeader, 'Application data contract header'),
    @($initHeader, 'Application queue header'),
    @($initSource, 'Application queue source'),
    @($taskHeader, 'MotorControl task header'),
    @($taskSource, 'MotorControl task source'),
    @($harnessSource, 'Application behavior harness'),
    @($freeRtosHeader, 'host FreeRTOS header'),
    @($queueHeader, 'host queue header'),
    @($taskHeaderStub, 'host task header'),
    @($appStubSource, 'Application host stub source')
)) {
    Require-File $entry[0] $entry[1]
}

foreach ($token in @(
    'APP_STATE_INIT', 'APP_STATE_READY', 'APP_STATE_RUN', 'APP_STATE_ESTOP', 'APP_STATE_FAULT',
    'AppMotorCommand', 'AppEncoderSnapshot', 'AppMotorStatus', 'AppSafetyLatch', 'AppUiSnapshot'
)) {
    Require-Regex $typesHeader ([regex]::Escape($token)) "Application contract token '$token'"
}
Reject-Regex $typesHeader 'driverlib|ti_msp_dl_config' 'DriverLib dependency in public Application contract'

Require-Regex $initSource 'static\s+StaticQueue_t\s+g_commandQueueStorage' 'static command queue control block'
Require-Regex $initSource 'static\s+StaticQueue_t\s+g_statusQueueStorage' 'static status queue control block'
Require-Regex $initSource 'static\s+StaticQueue_t\s+g_safetyStatusQueueStorage' 'static safety-status queue control block'
Require-Regex $initSource 'xQueueCreateStatic\s*\(\s*1U' 'length-one static queue creation'
Require-Regex $initSource 'xQueueOverwrite\s*\(' 'latest-value queue overwrite'
Require-Regex $initSource 'g_safetyStatusQueue\s*=\s*xQueueCreateStatic\s*\(\s*1U\s*,\s*sizeof\s*\(\s*AppMotorStatus\s*\)' 'length-one safety-status queue creation'
Require-Regex $initHeader 'App_PublishSafetyStatus\s*\(\s*const\s+AppMotorStatus\s*\*\s*status\s*\)' 'safety-status publish API'
Require-Regex $initHeader 'App_ReadLatestSafetyStatus\s*\(\s*AppMotorStatus\s*\*\s*status\s*\)' 'safety-status read API'
Reject-Regex $initSource '\b(xQueueCreate|pvPortMalloc|malloc|calloc|realloc|free)\s*\(' 'dynamic allocation in Application queue code'

Require-Regex $taskSource 'static\s+StaticTask_t\s+g_motorControlTaskStorage' 'static MotorControl task storage'
Require-Regex $taskSource 'static\s+StackType_t\s+g_motorControlTaskStack' 'static MotorControl task stack'
Require-Regex $taskSource 'xTaskCreateStatic\s*\(' 'static task creation'
Require-Regex $taskSource 'vTaskDelayUntil\s*\([^\)]*pdMS_TO_TICKS\s*\(\s*1U\s*\)' 'one millisecond periodic schedule'
Require-Regex $taskSource 'Tb6612Motor_StopCoast\s*\(' 'deterministic stop path'
Require-Regex $taskSource 'MotorControl_CommandIsUsable' 'timeout/invalid-state command validation'
Require-Regex $taskSource 'AppMotorCommand\s+command\s*=\s*App_DefaultCommand\s*\(\s*\)\s*;\s*[\s\S]*for\s*\(\s*;\s*;\s*\)' 'retained latest command outside the periodic loop'
Require-Regex $taskSource 'if\s*\(\s*App_ReadLatestCommand\s*\(\s*&command\s*\)\s*\)' 'non-blocking latest-command refresh'
Require-Regex $taskSource 'App_ReadLatestSafetyStatus\s*\(\s*&safetyStatus\s*\)' 'per-cycle latest safety-status refresh'
Require-Regex $taskSource 'APP_STATE_ESTOP' 'MotorControl ESTOP interlock'
Require-Regex $taskSource 'APP_STATE_FAULT' 'MotorControl FAULT interlock'
Require-Regex $taskSource 'SpeedPid_Reset\s*\(\s*pidB\s*\)' 'MotorControl PID B reset on interlock'
Require-Regex $taskSource 'SpeedPid_Reset\s*\(\s*pidC\s*\)' 'MotorControl PID C reset on interlock'
Require-Regex $harnessSource 'ApplicationHost_RunMotorControlTaskCycles\s*\(\s*2U\s*\)' 'real MotorControl task-cycle execution'
Require-Regex $harnessSource 'ApplicationHost_GetSafetyReadCalls\s*\(\s*\)\s*==\s*2U' 'safety-status read assertion'
Require-Regex $harnessSource 'ApplicationHost_GetStopCoastCalls\s*\(\s*\)\s*==\s*1U' 'stop stub assertion'
Require-Regex $harnessSource 'ApplicationHost_GetSetDutyBCalls\s*\(\s*\)\s*==\s*1U' 'single PWM B assertion'
Require-Regex $harnessSource 'ApplicationHost_GetSetDutyCCalls\s*\(\s*\)\s*==\s*1U' 'single PWM C assertion'
Require-Regex $harnessSource 'publishedStatus->dutyB\s*==\s*0' 'unsafe duty B zero assertion'
Require-Regex $harnessSource 'publishedStatus->dutyC\s*==\s*0' 'unsafe duty C zero assertion'
Reject-Regex $taskHeader 'driverlib|ti_msp_dl_config' 'DriverLib dependency in public MotorControl header'
Reject-Regex $taskSource '\b(pvPortMalloc|malloc|calloc|realloc|free)\s*\(' 'dynamic allocation in MotorControl task'

foreach ($source in @('../application/app_init.c', '../application/motor_control_task.c')) {
    Require-Regex $projectFile ([regex]::Escape($source)) "Keil registration for $source"
}

$compiler = Get-HostCompiler
$artifactRoot = Join-Path $testsRoot 'artifacts\application_behavior'
New-Item -ItemType Directory -Force -Path $artifactRoot | Out-Null
$exePath = Join-Path $artifactRoot 'application_behavior_harness.exe'
$sources = @(
    $harnessSource,
    $appStubSource,
    (Join-Path $projectRoot 'module\speed_pid.c'),
    $taskSource
)
$includeArgs = @(
    ('/I"{0}"' -f (Join-Path $testsRoot 'host_stub')),
    ('/I"{0}"' -f $applicationRoot),
    ('/I"{0}"' -f (Join-Path $projectRoot 'module'))
)
$sourceArgs = $sources | ForEach-Object { '"{0}"' -f $_ }
$vsDevCmd = 'D:\develop\vs2022\Professional\Common7\Tools\VsDevCmd.bat'
$commandText = ('call "{0}" -arch=x64 -host_arch=x64 >nul && "{1}" /nologo /TC /W4 {2} {3} /Fe:"{4}"' -f $vsDevCmd, $compiler, ($includeArgs -join ' '), ($sourceArgs -join ' '), $exePath)
& cmd.exe /d /s /c $commandText
if ($LASTEXITCODE -ne 0) {
    throw 'Application host behavior harness compilation failed.'
}
& $exePath
if ($LASTEXITCODE -ne 0) {
    throw 'Application host behavior harness execution failed.'
}

Write-Host 'PASS: Task 4 Application contracts are complete.'
