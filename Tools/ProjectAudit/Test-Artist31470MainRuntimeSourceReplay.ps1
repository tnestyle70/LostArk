[CmdletBinding()]
param([string]$RepositoryRoot = '', [switch]$Deep)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
}
$tool = Join-Path $RepositoryRoot `
    'Tools/LevelPlacementExtractor/verify_artist_31470_main_runtime_source_replay.py'

Push-Location $RepositoryRoot
try {
    if ($Deep) {
        powershell -ExecutionPolicy Bypass -File `
            Tools/ProjectAudit/Test-Artist31470MainOriginalDxbcReplay.ps1 -Deep
        if ($LASTEXITCODE -ne 0) {
            throw 'Artist 31470 original-DXBC prerequisite failed.'
        }
        python -B $tool --check
    }
    else {
        python -B $tool --validate-only
    }
    if ($LASTEXITCODE -ne 0) {
        throw 'Artist 31470 shipped-runtime source replay failed.'
    }
	Write-Output ("Artist 31470 main runtime source replay audit PASS: " +
		"mode=$(if ($Deep) { 'deep' } else { 'shallow' }) product=false visual=false")
}
finally {
    Pop-Location
}
