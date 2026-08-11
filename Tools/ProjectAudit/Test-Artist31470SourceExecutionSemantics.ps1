[CmdletBinding()]
param(
    [string]$RepositoryRoot = '',
    [switch]$Deep,
    [string]$ReleaseRoot = 'C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC'
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    $RepositoryRoot = [IO.Path]::GetFullPath($RepositoryRoot)
}

$receiptPath = Join-Path $RepositoryRoot `
    'Data\Effects\Imported\Artist\Candidates\skill.31470.source-execution-semantics.receipt.json'

Push-Location $RepositoryRoot
try {
    & python -B `
        'Tools/LevelPlacementExtractor/test_build_artist_31470_source_execution_semantics.py'
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F source execution tests failed: $LASTEXITCODE"
    }

    $verifyArguments = @(
        '-B',
        'Tools/LevelPlacementExtractor/verify_artist_31470_source_execution_semantics.py'
    )
    if ($Deep) {
        if (-not (Test-Path -LiteralPath $ReleaseRoot -PathType Container)) {
            throw "Artist F source execution deep Release root is missing: $ReleaseRoot"
        }
        $verifyArguments += @('--release-root', $ReleaseRoot)
    }
    else {
        $verifyArguments += '--shallow'
    }
    & python @verifyArguments
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F source execution oracle failed: $LASTEXITCODE"
    }

    if ($Deep) {
        & python -B `
            'Tools/LevelPlacementExtractor/build_artist_31470_source_execution_semantics.py' `
            '--release-root' $ReleaseRoot `
            '--check'
        if ($LASTEXITCODE -ne 0) {
            throw "Artist F source execution generator check failed: $LASTEXITCODE"
        }
    }

    $receipt = Get-Content -LiteralPath $receiptPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $denominator = $receipt.summary.denominators
    $moduleCounts = $receipt.summary.moduleDecisionCounts
    $distributionCounts = $receipt.summary.distributionDecisionCounts
    if ($receipt.schema -cne 'lostark.effect-source-execution-semantics' -or
        [int]$receipt.formatVersion -ne 1 -or
        $receipt.characterClass -cne 'ARTIST' -or
        [int]$receipt.skillId -ne 31470 -or
        $receipt.inputSlot -cne 'F' -or
        [int]$denominator.occurrenceCount -ne 35 -or
        [int]$denominator.moduleCount -ne 399 -or
        [int]$denominator.propertyCount -ne 1434 -or
        [int]$denominator.primitiveLeafCount -ne 1572 -or
        [int]$denominator.distributionCount -ne 629 -or
        [int]$moduleCounts.READY_FOR_HANDLER -ne 370 -or
        [int]$moduleCounts.BLOCKED -ne 29 -or
        [int]$distributionCounts.READY_FOR_HANDLER -ne 626 -or
        [int]$distributionCounts.BLOCKED -ne 3 -or
        [bool]$receipt.productAdmission.allowed -or
        -not [bool]$receipt.summary.allRowsClassifiedAndBound -or
        [bool]$receipt.summary.allRowsConsumedOrIrrelevant -or
        [int]$receipt.summary.unclassifiedRowCount -ne 0 -or
        [int]$receipt.summary.silentFallbackCount -ne 0) {
        throw 'Artist F source execution summary contract changed.'
    }

    $mode = if ($Deep) { 'deep' } else { 'shallow' }
    Write-Host (
        "PASS: Artist F 31470 Source execution mode=$mode " +
        'modules=399 ready=370 blocked=29 distributions=629 blockedDistributions=3 product=false'
    )
}
finally {
    Pop-Location
}
