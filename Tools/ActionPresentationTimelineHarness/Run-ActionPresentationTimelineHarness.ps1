param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug'
)

$ErrorActionPreference = 'Stop'
$executable = Join-Path $PSScriptRoot `
    "Bin\$Configuration\ActionPresentationTimelineHarness.exe"

if (-not (Test-Path -LiteralPath $executable -PathType Leaf)) {
    throw "ActionPresentationTimelineHarness was not built: $executable"
}

Push-Location ([IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..')))
$previousNativeErrorAction = $ErrorActionPreference
$nativeExitCode = -1
try {
    # Preserve stderr diagnostics without treating progress as process failure.
    $ErrorActionPreference = 'Continue'
    $global:LASTEXITCODE = -1
    & $executable
    $nativeExitCode = $global:LASTEXITCODE
} finally {
    $ErrorActionPreference = $previousNativeErrorAction
    Pop-Location
}
if ($nativeExitCode -ne 0) {
    throw "ActionPresentationTimelineHarness failed with exit code $nativeExitCode"
}
