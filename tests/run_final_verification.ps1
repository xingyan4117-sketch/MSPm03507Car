$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot
$sdkRoot = Split-Path -Parent $projectRoot
$projectFile = Join-Path $projectRoot 'keil\car_freertos_LP_MSPM0G3507.uvprojx'
$projectOptionsFile = Join-Path $projectRoot 'keil\car_freertos_LP_MSPM0G3507.uvoptx'
$objectRoot = Join-Path $projectRoot 'keil\Objects'
$artifactRoot = Join-Path $PSScriptRoot 'artifacts\final_verification'
$timestamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$logFile = Join-Path $artifactRoot ("UV4-cr-$timestamp.log")
$verificationLog = Join-Path $artifactRoot ("verification-$timestamp.log")
$optionsBackup = Join-Path $artifactRoot ("uvoptx-before-UV4-$timestamp.bak")
$screenTest = Join-Path $projectRoot 'tests\test_screen_render_contracts.ps1'

New-Item -ItemType Directory -Force -Path $artifactRoot | Out-Null

$tests = @(
    (Join-Path $projectRoot 'tests\test_button_contracts.ps1'),
    (Join-Path $projectRoot 'tests\test_auto_start.ps1'),
    (Join-Path $projectRoot 'tests\test_command_buttons.ps1'),
    (Join-Path $projectRoot 'tests\test_ad_port.ps1'),
    (Join-Path $projectRoot 'tests\test_module_contracts.ps1'),
    (Join-Path $projectRoot 'tests\test_application_contracts.ps1'),
    (Join-Path $projectRoot 'tests\test_menu_led_buzzer.ps1'),
    (Join-Path $projectRoot 'tests\test_ws2812_host.ps1'),
    $screenTest
)

$failures = [System.Collections.Generic.List[string]]::new()
foreach ($test in $tests) {
    Write-Host "RUN: $test"
    $testOutput = & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $test 2>&1
    $testOutput | Tee-Object -FilePath $verificationLog -Append | ForEach-Object { Write-Host $_ }
    $testExit = $LASTEXITCODE
    if ($testExit -ne 0) {
        $failures.Add("test failed: $test (exit $testExit)")
    }
}

if (-not (Test-Path -LiteralPath $projectOptionsFile -PathType Leaf)) {
    $failures.Add("missing DAPLink options file: $projectOptionsFile")
} else {
    $dapLinkText = Get-Content -LiteralPath $projectOptionsFile -Raw
    foreach ($token in @('BIN\CMSIS_AGDI.dll', '<Protocol>2</Protocol>', '<DbgClock>1000000</DbgClock>', 'MSPM0G1X0X_G3X0X_MAIN_128KB.FLM')) {
        if ($dapLinkText -notmatch [regex]::Escape($token)) {
            $failures.Add("missing DAPLink setting: $token")
        }
    }
    if (($dapLinkText -notmatch [regex]::Escape('CMSIS-DAP')) -and
        ($dapLinkText -notmatch [regex]::Escape('ARM CoreSight SW-DP'))) {
        $failures.Add('missing compatible DAPLink debugger target: CMSIS-DAP or ARM CoreSight SW-DP')
    }
}

$testArtifacts = Join-Path $PSScriptRoot 'artifacts'
if (-not (Test-Path -LiteralPath $testArtifacts -PathType Container) -or
    @(Get-ChildItem -LiteralPath $testArtifacts -Recurse -File).Count -eq 0) {
    $failures.Add("test artifacts are missing or empty: $testArtifacts")
}

$uv4Candidates = @(@(
    $env:KEIL_UV4,
    'D:\keli5\UV4\UV4.exe',
    'C:\Keil_v5\UV4\UV4.exe'
) | Where-Object { $_ -and (Test-Path -LiteralPath $_ -PathType Leaf) })
if ($uv4Candidates.Count -eq 0) {
    $failures.Add('UV4.exe was not found. Set KEIL_UV4 to the full executable path.')
} else {
    $uv4 = $uv4Candidates[0]
    Write-Host "RUN: $uv4 -cr $projectFile"
    $uv4Command = '"{0}" -cr "{1}" -o"{2}"' -f $uv4, $projectFile, $logFile
    Copy-Item -LiteralPath $projectOptionsFile -Destination $optionsBackup -Force
    $optionsBackupHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $optionsBackup).Hash
    try {
        & cmd.exe /d /c $uv4Command
        $buildExit = $LASTEXITCODE
        if (Test-Path -LiteralPath $logFile -PathType Leaf) {
            $buildText = Get-Content -LiteralPath $logFile -Raw
        } else {
            $buildText = ''
            $failures.Add("UV4 did not create the requested build log: $logFile")
        }
    } finally {
        Copy-Item -LiteralPath $optionsBackup -Destination $projectOptionsFile -Force
    }
    if ((Get-FileHash -Algorithm SHA256 -LiteralPath $projectOptionsFile).Hash -ne $optionsBackupHash) {
        $failures.Add("UV4 changed DAPLink options after restoration: $projectOptionsFile")
    }
    if ($buildExit -ne 0) {
        $failures.Add("UV4 -cr failed with exit $buildExit; see $logFile")
    }
    if ($buildText -notmatch '0 Error\(s\), 0 Warning\(s\)') {
        $failures.Add("UV4 output does not contain '0 Error(s), 0 Warning(s)'; see $logFile")
    }
}

foreach ($artifact in @(
    (Join-Path $objectRoot 'car_freertos_LP_MSPM0G3507.axf'),
    (Join-Path $objectRoot 'car_freertos_LP_MSPM0G3507.hex')
)) {
    if (-not (Test-Path -LiteralPath $artifact -PathType Leaf) -or (Get-Item -LiteralPath $artifact).Length -eq 0) {
        $failures.Add("missing or empty build artifact: $artifact")
    }
}

if ($failures.Count -gt 0) {
    $failures | ForEach-Object { Write-Error $_ }
    exit 1
}

Write-Host "PASS: all $($tests.Count) PowerShell tests, artifacts, compatible DAPLink settings, and UV4 -cr rebuild verified."
Write-Host "Verification log: $verificationLog"
Write-Host "Build log: $logFile"
