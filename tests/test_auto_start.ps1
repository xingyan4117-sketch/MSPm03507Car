$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$source = Join-Path $root 'application\app_init.c'
$content = Get-Content -LiteralPath $source -Raw

if ($content -notmatch '\.state\s*=\s*APP_STATE_READY') {
    throw 'Default command is not configured for safe READY state.'
}

foreach ($channel in 'A', 'B', 'C', 'D') {
    if ($content -notmatch ("\.targetRpm{0}\s*=\s*0" -f $channel)) {
        throw "Default command does not keep motor channel $channel stopped at power-on."
    }
}

Write-Host 'PASS: power-on motor stop contract is complete.'
