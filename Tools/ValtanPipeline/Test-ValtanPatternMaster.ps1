[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$repositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$projector = Join-Path $PSScriptRoot 'Project-ValtanPatternMaster.ps1'
$pythonSuite = Join-Path $PSScriptRoot 'test_valtan_pattern_master_v2.py'

if (-not [IO.File]::Exists($projector)) {
    throw "Missing Valtan split projector: $projector"
}
if (-not [IO.File]::Exists($pythonSuite)) {
    throw "Missing Valtan split contract suite: $pythonSuite"
}

# The former harness exercised the retired v1 in-place projector and could
# rewrite a rotation-v3 document with v2 patternIds rows. Keep the public
# entrypoint, but route its positive and negative coverage through the active
# split-authoring contract instead of preserving a second runtime path.
& $projector -Mode Validate -RepositoryRoot $repositoryRoot

$legacyPublishRejected = $false
try {
    & $projector -Mode Publish -RepositoryRoot $repositoryRoot
}
catch {
    if ($_.Exception.Message -notlike
        '*Legacy Valtan.pattern.json projection Publish is retired*') {
        throw
    }
    $legacyPublishRejected = $true
}
if (-not $legacyPublishRejected) {
    throw 'Legacy Valtan.pattern.json Publish was not rejected.'
}

$global:LASTEXITCODE = 0
& python -B $pythonSuite
if ($global:LASTEXITCODE -ne 0) {
    throw "Valtan split pattern master Python suite failed with exit code $global:LASTEXITCODE."
}

& $projector -Mode Validate -RepositoryRoot $repositoryRoot
Write-Host 'Valtan split pattern master harness: PASS'
