[CmdletBinding()]
param(
    [string]$EngineRoot,
    [switch]$Native,
    [switch]$PlanOnly,
    [ValidateRange(1, 86400)][int]$Timeout = 1800
)
$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    $python = Get-Command py -ErrorAction SilentlyContinue
    $prefix = @('-3')
    if (-not $python) { $python = Get-Command python -ErrorAction SilentlyContinue; $prefix = @() }
    if (-not $python) { throw 'Python 3.11+ is required. No installer was run.' }
    $action = 'onboard'
    if ($PlanOnly) { $action = 'plan' }
    $arguments = @('tools/wyrm.py', $action, '--timeout', "$Timeout")
    if ($Native) { $arguments += '--native' }
    if ($EngineRoot) { $arguments += @('--engine-root', $EngineRoot) }
    & $python.Source @prefix @arguments
    if ($LASTEXITCODE -ne 0) {
        throw "WYRMFALL stopped (exit $LASTEXITCODE). Read the log and Saved/Onboarding report; later steps were not run."
    }
    Write-Host 'Requested command checks finished. Native checks require -Native; PIE and cooking are never implied.'
} finally {
    Pop-Location
}
