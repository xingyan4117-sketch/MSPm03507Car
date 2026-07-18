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

function Get-FirstExistingPath([string[]] $Candidates) {
    foreach ($candidate in $Candidates) {
        if ([string]::IsNullOrWhiteSpace($candidate)) {
            continue
        }
        if (Test-Path -LiteralPath $candidate -PathType Leaf) {
            return $candidate
        }
    }

    return $null
}

function Get-CommandPath([string] $Name) {
    $command = Get-Command $Name -ErrorAction SilentlyContinue
    if ($null -ne $command) {
        return $command.Source
    }

    return $null
}

function Get-HostCompiler() {
    $clangPath = Get-FirstExistingPath @(
        (Get-CommandPath 'clang'),
        'C:\Program Files\LLVM\bin\clang.exe'
    )
    if ($null -ne $clangPath) {
        return [pscustomobject] @{
            Name = 'clang'
            Kind = 'clang'
            Path = $clangPath
            EnvScript = $null
        }
    }

    $gccPath = Get-FirstExistingPath @(
        (Get-CommandPath 'gcc'),
        'C:\msys64\ucrt64\bin\gcc.exe',
        'C:\msys64\mingw64\bin\gcc.exe',
        'C:\MinGW\bin\gcc.exe',
        'C:\TDM-GCC-64\bin\gcc.exe'
    )
    if ($null -ne $gccPath) {
        return [pscustomobject] @{
            Name = 'gcc'
            Kind = 'gcc'
            Path = $gccPath
            EnvScript = $null
        }
    }

    $clPath = Get-CommandPath 'cl'
    if ($null -eq $clPath) {
        $vswhere = 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe'
        if (Test-Path -LiteralPath $vswhere -PathType Leaf) {
            $clPath = (& $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find 'VC\Tools\MSVC\**\bin\Hostx64\x64\cl.exe' |
                Select-Object -First 1)
        }
    }
    if ($null -eq $clPath) {
        $clPath = Get-FirstExistingPath @(
            'D:\develop\vs2022\Professional\VC\Tools\MSVC\14.29.30133\bin\Hostx64\x64\cl.exe',
            'C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\MSVC\14.29.30133\bin\Hostx64\x64\cl.exe',
            'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.29.30133\bin\Hostx64\x64\cl.exe',
            'C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.29.30133\bin\Hostx64\x64\cl.exe'
        )
    }
    if ($null -ne $clPath) {
        $envScript = $null
        $clText = [string] $clPath
        $marker = '\VC\Tools\MSVC\'
        $markerIndex = $clText.IndexOf($marker, [System.StringComparison]::OrdinalIgnoreCase)
        if ($markerIndex -ge 0) {
            $installRoot = $clText.Substring(0, $markerIndex)
            $candidate = Join-Path $installRoot 'Common7\Tools\VsDevCmd.bat'
            if (Test-Path -LiteralPath $candidate -PathType Leaf) {
                $envScript = $candidate
            }
        }

        return [pscustomobject] @{
            Name = 'cl'
            Kind = 'msvc'
            Path = $clPath
            EnvScript = $envScript
        }
    }

    return $null
}

function Invoke-External([string] $FilePath, [string[]] $ArgumentList, [string] $WorkingDirectory) {
    $previous = Get-Location
    try {
        Set-Location -LiteralPath $WorkingDirectory
        $output = & $FilePath @ArgumentList 2>&1
        return [pscustomobject] @{
            ExitCode = $LASTEXITCODE
            Output = @($output)
        }
    }
    finally {
        Set-Location -LiteralPath $previous
    }
}

function Invoke-HostBehaviorHarness([string] $ProjectRoot, [string] $TestsRoot, [string] $HarnessSource, [string[]] $HarnessSupportFiles) {
    $compiler = Get-HostCompiler
    if ($null -eq $compiler) {
        throw 'No suitable Windows host C compiler found (looked for clang, gcc, and cl).'
    }

    $buildRoot = Join-Path $TestsRoot 'artifacts\module_behavior'
    $null = New-Item -ItemType Directory -Path $buildRoot -Force
    Remove-Item -LiteralPath (Join-Path $buildRoot '*') -Force -ErrorAction SilentlyContinue

    $moduleRoot = Join-Path $ProjectRoot 'module'
    $exePath = Join-Path $buildRoot 'module_behavior_harness.exe'
    $commonSources = @(
        $HarnessSource
        $HarnessSupportFiles
        (Join-Path $moduleRoot 'tb6612_motor.c')
        (Join-Path $moduleRoot 'encoder_ab.c')
        (Join-Path $moduleRoot 'speed_pid.c')
    )

    Write-Host ("INFO: Host behavior compiler: {0} ({1})" -f $compiler.Name, $compiler.Path)

    if (($compiler.Kind -eq 'clang') -or ($compiler.Kind -eq 'gcc')) {
        $compileArgs = @(
            '-std=c11', '-Wall', '-Wextra', '-pedantic',
            '-I', (Join-Path $TestsRoot 'host_stub'),
            '-I', $moduleRoot,
            '-I', $TestsRoot
            $commonSources
            '-o', $exePath
        )
        $compile = Invoke-External $compiler.Path $compileArgs $buildRoot
    } else {
        $compileParts = @(
            ('"{0}"' -f $compiler.Path),
            '/nologo',
            '/TC',
            '/W4',
            ('/I"{0}"' -f (Join-Path $TestsRoot 'host_stub')),
            ('/I"{0}"' -f $moduleRoot),
            ('/I"{0}"' -f $TestsRoot)
        )
        foreach ($source in $commonSources) {
            $compileParts += ('"{0}"' -f $source)
        }
        $compileParts += ('/Fe:"{0}"' -f $exePath)

        $commandText = $compileParts -join ' '
        if (-not [string]::IsNullOrWhiteSpace($compiler.EnvScript)) {
            $commandText = ('call "{0}" -arch=x64 -host_arch=x64 >nul && {1}' -f $compiler.EnvScript, $commandText)
        }

        $compile = Invoke-External 'cmd.exe' @('/d', '/s', '/c', $commandText) $buildRoot
    }

    if ($compile.ExitCode -ne 0) {
        $message = @(
            'Host behavior harness compilation failed.'
            ($compile.Output -join [Environment]::NewLine)
        ) -join [Environment]::NewLine
        throw $message.TrimEnd()
    }

    $run = Invoke-External $exePath @() $buildRoot
    if ($run.ExitCode -ne 0) {
        $message = @(
            ('Host behavior harness failed with exit code {0}.' -f $run.ExitCode)
            ($run.Output -join [Environment]::NewLine)
        ) -join [Environment]::NewLine
        throw $message.TrimEnd()
    }

    if ($run.Output.Count -gt 0) {
        $run.Output | ForEach-Object { Write-Host $_ }
    }
}

$moduleRoot = Join-Path $projectRoot 'module'
$motorHeader = Join-Path $moduleRoot 'tb6612_motor.h'
$motorSource = Join-Path $moduleRoot 'tb6612_motor.c'
$encoderHeader = Join-Path $moduleRoot 'encoder_ab.h'
$encoderSource = Join-Path $moduleRoot 'encoder_ab.c'
$pidHeader = Join-Path $moduleRoot 'speed_pid.h'
$pidSource = Join-Path $moduleRoot 'speed_pid.c'
$projectFile = Join-Path $projectRoot 'keil\car_freertos_LP_MSPM0G3507.uvprojx'
$testsRoot = Join-Path $projectRoot 'tests'
$hostStubRoot = Join-Path $testsRoot 'host_stub'
$behaviorHarness = Join-Path $testsRoot 'module_behavior_harness.c'
$hostStubHeaderPwm = Join-Path $hostStubRoot 'bsp_pwm.h'
$hostStubHeaderEncoder = Join-Path $hostStubRoot 'bsp_encoder.h'
$hostStubSupportHeader = Join-Path $hostStubRoot 'host_stub_support.h'
$hostStubSource = Join-Path $hostStubRoot 'host_stub.c'

foreach ($entry in @(
    @($motorHeader, 'TB6612 motor header'), @($motorSource, 'TB6612 motor source'),
    @($encoderHeader, 'encoder module header'), @($encoderSource, 'encoder module source'),
    @($pidHeader, 'speed PID header'), @($pidSource, 'speed PID source'),
    @($projectFile, 'Keil project'),
    @($behaviorHarness, 'Module behavior harness source'),
    @($hostStubHeaderPwm, 'host-side PWM stub header'),
    @($hostStubHeaderEncoder, 'host-side encoder stub header'),
    @($hostStubSupportHeader, 'host-side stub support header'),
    @($hostStubSource, 'host-side stub source')
)) {
    Require-File $entry[0] $entry[1]
}

foreach ($api in @(
    'Tb6612Motor_Init', 'Tb6612Motor_SetDutyB', 'Tb6612Motor_SetDutyC',
    'Tb6612Motor_StopCoast', 'TB6612_MOTOR_DEFAULT_MAX_DUTY'
)) {
    Require-Token $motorHeader $api
}
Require-Token $motorSource 'BspPwm_SetDutyB'
Require-Token $motorSource 'BspPwm_SetDutyC'
Require-Token $motorSource 'BspPwm_StopAll'
Require-Regex $motorSource 'static\s+Tb6612MotorInstance\s+g_motorB' 'static B motor instance'
Require-Regex $motorSource 'static\s+Tb6612MotorInstance\s+g_motorC' 'static C motor instance'
Require-Regex $motorSource 'Tb6612Motor_ClampDuty' 'signed motor clamp helper'
Require-Regex $motorSource 'Tb6612Motor_SetDutyB\s*\(\s*int32_t\s+signedDuty\s*\)' 'signed B duty API'
Require-Regex $motorSource 'Tb6612Motor_SetDutyC\s*\(\s*int32_t\s+signedDuty\s*\)' 'signed C duty API'
Require-Regex $motorSource 'Tb6612Motor_StopCoast\s*\(\s*void\s*\)[\s\S]*BspPwm_StopAll\s*\(\s*\)' 'safe coast stop path'
Reject-Regex $motorHeader '\b(MotorA|MotorD|MotorBC|SetDutyA|SetDutyD)\b' 'legacy A/D or BC motor API'

foreach ($api in @(
    'EncoderAbSnapshot', 'EncoderAbRpmConfig', 'EncoderAb_ReadB', 'EncoderAb_ReadC',
    'EncoderAb_CountsToRpm', 'deltaCounts', 'invalidTransitions',
    'countsPerRevolution', 'samplePeriodMs'
)) {
    Require-Token $encoderHeader $api
}
Require-Regex $encoderSource 'EncoderAb_ReadB\s*\(\s*void\s*\)[\s\S]*BspEncoder_ReadAndClearB\s*\(\s*\)' 'B encoder mapping'
Require-Regex $encoderSource 'EncoderAb_ReadC\s*\(\s*void\s*\)[\s\S]*BspEncoder_ReadAndClearC\s*\(\s*\)' 'C encoder mapping'
Require-Regex $encoderSource 'snapshot\.invalidTransitions\s*=\s*readout\.invalidTransitions' 'invalid transition propagation'
Require-Regex $encoderSource 'countsPerRevolution\s*==\s*0U\s*\|\|\s*config->samplePeriodMs\s*==\s*0U' 'explicit RPM configuration validation'
Reject-Regex $encoderHeader '\b(EncoderA|EncoderD|EncoderBC)\b' 'legacy A/D or BC encoder API'

foreach ($api in @(
    'SpeedPid', 'SpeedPidConfig', 'SpeedPidStatus', 'SpeedPid_Init',
    'SpeedPid_Reset', 'SpeedPid_Update', 'SPEED_PID_DEFAULT_OUTPUT_MIN',
    'SPEED_PID_DEFAULT_OUTPUT_MAX'
)) {
    Require-Token $pidHeader $api
}
Require-Token $pidHeader '(-800)'
Require-Token $pidHeader '(800)'
Require-Regex $pidSource 'SpeedPid_Reset\s*\(\s*SpeedPid\s*\*\s*pid\s*\)[\s\S]*integral\s*=\s*0' 'deterministic PID reset'
Require-Regex $pidSource 'SpeedPid_Update\s*\(\s*SpeedPid\s*\*\s*pid\s*,\s*int32_t\s+setpoint\s*,\s*int32_t\s+measurement\s*\)' 'signed setpoint and measurement update'
Require-Regex $pidSource 'SpeedPid_Clamp' 'PID clamp helper'
Require-Regex $pidSource 'integralMin[\s\S]*integralMax' 'integral bounds'
Require-Regex $pidSource 'outputMin[\s\S]*outputMax' 'output bounds'
Require-Regex $pidSource 'config\.scale\s*<=\s*0' 'non-positive scale normalization'

foreach ($file in @($motorHeader, $motorSource, $encoderHeader, $encoderSource, $pidHeader, $pidSource)) {
    Reject-Regex $file 'ti/driverlib|driverlib\.h' 'DriverLib include'
    Reject-Regex $file '\b(malloc|calloc|realloc|free)\s*\(' 'dynamic allocation'
}

foreach ($source in @('../module/tb6612_motor.c', '../module/encoder_ab.c', '../module/speed_pid.c')) {
    Require-Token $projectFile $source
}
Require-Regex $projectFile '<GroupName>Module</GroupName>' 'Keil Module group'
Require-Token $projectFile '../module'
Reject-Regex $projectFile 'tests/(host_stub|module_behavior_harness)|tests\\(host_stub|module_behavior_harness)' 'host-only test artifacts in Keil project'

Invoke-HostBehaviorHarness $projectRoot $testsRoot $behaviorHarness @($hostStubSource)

Write-Host 'PASS: Task 3 Module contracts are complete.'
