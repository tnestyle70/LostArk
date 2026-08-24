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
try {
    & $executable
    if ($LASTEXITCODE -ne 0) {
        throw "ActionPresentationTimelineHarness failed with exit code $LASTEXITCODE"
    }
} finally {
    Pop-Location
}
