[CmdletBinding()]
param(
    [string]$RepositoryRoot = '',
    [switch]$Deep,
    [string]$ReleaseRoot = 'C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC',
    [string]$BinaryRoot = 'C:\ProgramData\Smilegate\Games\LOSTARK\Binaries\Win64'
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    $RepositoryRoot = [IO.Path]::GetFullPath($RepositoryRoot)
}

$receiptPath = Join-Path $RepositoryRoot `
    'Data\Effects\Imported\Artist\Candidates\skill.31470.custom-handler-oracle.receipt.json'

Push-Location $RepositoryRoot
try {
    & python -B `
        'Tools/LevelPlacementExtractor/test_build_artist_31470_custom_handler_oracle.py'
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F custom handler oracle tests failed: $LASTEXITCODE"
    }

    $verifyArguments = @(
        '-B',
        'Tools/LevelPlacementExtractor/verify_artist_31470_custom_handler_oracle.py'
    )
    if ($Deep) {
        if (-not (Test-Path -LiteralPath $ReleaseRoot -PathType Container)) {
            throw "Artist F custom handler Release root is missing: $ReleaseRoot"
        }
        if (-not (Test-Path -LiteralPath $BinaryRoot -PathType Container)) {
            throw "Artist F custom handler binary root is missing: $BinaryRoot"
        }
        $verifyArguments += @(
            '--release-root', $ReleaseRoot,
            '--binary-root', $BinaryRoot
        )
    }
    else {
        $verifyArguments += '--shallow'
    }
    & python @verifyArguments
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F custom handler independent oracle failed: $LASTEXITCODE"
    }

    if ($Deep) {
        & python -B `
            'Tools/LevelPlacementExtractor/build_artist_31470_custom_handler_oracle.py' `
            '--release-root' $ReleaseRoot `
            '--binary-root' $BinaryRoot `
            '--check'
        if ($LASTEXITCODE -ne 0) {
            throw "Artist F custom handler generator check failed: $LASTEXITCODE"
        }
    }

    $receipt = Get-Content -LiteralPath $receiptPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $summary = $receipt.summary
    $matrixSummary = $receipt.feasibilityMatrix.summary
    if ($receipt.schema -cne 'lostark.effect-custom-handler-oracle' -or
        [int]$receipt.formatVersion -ne 2 -or
        $receipt.characterClass -cne 'ARTIST' -or
        [int]$receipt.skillId -ne 31470 -or
        $receipt.inputSlot -cne 'F' -or
        [int]$summary.sourceBlockedModuleCount -ne 29 -or
        [int]$summary.standardSeededOccurrenceCount -ne 11 -or
        [int]$summary.currentCrossRevisionAliasEvidenceCount -ne 7 -or
        [int]$summary.actualNativeParticleOutputOracleCount -ne 0 -or
        [int]$summary.nativeExactAliasAdmissionCount -ne 0 -or
        [int]$summary.capabilityGrantCount -ne 0 -or
        [int]$summary.blockedCustomModuleOccurrenceCount -ne 15 -or
        [int]$summary.blockedCustomDistributionOccurrenceCount -ne 3 -or
        [int]$summary.moduleBlockerOwnerCount -ne 29 -or
        [int]$summary.resolvedModuleBlockerCount -ne 0 -or
        [int]$summary.remainingBlockedModuleCount -ne 29 -or
        [int]$summary.distributionBlockerOwnerCount -ne 3 -or
        [int]$summary.ownerlessBlockerCount -ne 0 -or
        [int]$summary.projectedModuleDecisionCountsAfterJoin.READY_FOR_HANDLER -ne 370 -or
        [int]$summary.projectedModuleDecisionCountsAfterJoin.BLOCKED -ne 29 -or
        [int]$summary.projectedDistributionDecisionCountsAfterJoin.READY_FOR_HANDLER -ne 626 -or
        [int]$summary.projectedDistributionDecisionCountsAfterJoin.BLOCKED -ne 3 -or
        [int]$summary.productAdmissionCount -ne 0 -or
        [int]$summary.silentFallbackCount -ne 0 -or
        [int]$receipt.capabilityGrants.Count -ne 0 -or
        [int]$receipt.feasibilityMatrix.moduleRows.Count -ne 29 -or
        [int]$receipt.feasibilityMatrix.distributionRows.Count -ne 3 -or
        [int]$matrixSummary.actualNativeParticleOutputOracleCount -ne 0 -or
        [int]$matrixSummary.actualNativeDistributionOutputOracleCount -ne 0 -or
        [int]$matrixSummary.blockedModuleRowCount -ne 29 -or
        [int]$matrixSummary.unresolvedExecutionRowCount -ne 29 -or
        $matrixSummary.executionReadinessDecision -cne 'BLOCKED' -or
        [bool]$receipt.productAdmission.allowed) {
        throw 'Artist F custom handler oracle summary contract changed.'
    }

    $mode = if ($Deep) { 'deep' } else { 'shallow' }
    Write-Host (
        "PASS: Artist F 31470 custom handler oracle mode=$mode " +
        'ready=370 blocked=29 distributionReady=626 distributionBlocked=3 ' +
        'outputOracles=0 ownerless=0 product=false'
    )
}
finally {
    Pop-Location
}
