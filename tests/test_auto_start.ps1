$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$source = Join-Path $root 'application\app_init.c'
$content = Get-Content -LiteralPath $source -Raw

if ($content -notmatch '\.state\s*=\s*APP_STATE_RUN') {
    throw 'Default command is not configured for automatic RUN state.'
}

foreach ($channel in 'A', 'B', 'C', 'D') {
    if ($content -notmatch ("\.targetRpm{0}\s*=\s*COMMAND_START_RPM" -f $channel)) {
        throw "Default command does not start motor channel $channel at COMMAND_START_RPM."
    }
}

Write-Host 'PASS: automatic four-channel startup contract is complete.'
