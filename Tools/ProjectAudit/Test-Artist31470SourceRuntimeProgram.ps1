[CmdletBinding()]
param(
    [string]$RepositoryRoot = '',
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [string]$HarnessPath = ''
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    $RepositoryRoot = [IO.Path]::GetFullPath($RepositoryRoot)
}

$candidatePath = Join-Path $RepositoryRoot `
    'Data\Effects\Imported\Artist\Candidates\skill.31470.source-runtime-program.candidate.json'
if ([string]::IsNullOrWhiteSpace($HarnessPath)) {
    $HarnessPath = Join-Path $RepositoryRoot `
        "Tools\ClientFrontendHarness\Bin\$Configuration\ClientFrontendHarness.exe"
}
else {
    $HarnessPath = [IO.Path]::GetFullPath($HarnessPath)
}

Push-Location $RepositoryRoot
try {
    & python -B `
        'Tools/EffectPipeline/test_build_artist_31470_source_runtime_program.py'
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F source runtime program tests failed: $LASTEXITCODE"
    }

    & python -B `
        'Tools/EffectPipeline/build_artist_31470_source_runtime_program.py' `
        '--check'
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F source runtime program generator check failed: $LASTEXITCODE"
    }

    $candidate = Get-Content -LiteralPath $candidatePath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $summary = $candidate.summary
    if ($candidate.schema -cne 'lostark.effect-source-runtime-program' -or
        [int]$candidate.formatVersion -ne 1 -or
        $candidate.effectAssetId -cne 'effect.artist.skill.31470.f' -or
        $candidate.runtimeSemanticAuthority -cne 'TYPED_SOURCE_PROGRAM_CANDIDATE' -or
        [int]$summary.emitterCount -ne 35 -or
        [int]$summary.opcodeCount -ne 399 -or
        [int]$summary.readyOpcodeCount -ne 370 -or
        [int]$summary.blockedOpcodeCount -ne 29 -or
        [int]$summary.distributionCount -ne 629 -or
        [int]$summary.silentFallbackCount -ne 0 -or
        [bool]$candidate.executionContract.executionAdmission -or
        [bool]$candidate.runtimeExecutionAdmission -or
        [bool]$candidate.productAdmission) {
        throw 'Artist F source runtime program summary contract changed.'
    }

    if (-not (Test-Path -LiteralPath $HarnessPath -PathType Leaf)) {
        throw "Artist F source runtime program harness is missing: $HarnessPath"
    }
    $harnessOutput = & $HarnessPath `
        '--effect-source-runtime-program' $candidatePath 2>&1
    if ($LASTEXITCODE -ne 0 -or
        ($harnessOutput -join "`n") -notmatch 'failures : 0') {
        throw "Artist F source runtime program C++ materializer failed:`n$($harnessOutput -join "`n")"
    }

    Write-Host (
        'PASS: Artist F 31470 Source runtime program ' +
        'emitters=35 opcodes=399 ready=370 blocked=29 distributions=629 ' +
        'silentFallback=0 runtime=false product=false'
    )
}
finally {
    Pop-Location
}
