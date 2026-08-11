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
    'Data\Effects\Imported\Artist\Candidates\skill.31470.source-oracle-acquisition.receipt.json'

Push-Location $RepositoryRoot
try {
    & python -B `
        'Tools/LevelPlacementExtractor/test_build_artist_31470_source_oracle_acquisition.py'
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F Source oracle acquisition tests failed: $LASTEXITCODE"
    }

    & python -B `
        'Tools/LevelPlacementExtractor/build_artist_31470_source_oracle_acquisition.py' `
        '--check'
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F Source oracle acquisition generator check failed: $LASTEXITCODE"
    }

    if ($Deep) {
        & '.\Tools\ProjectAudit\Test-Artist31470CustomHandlerOracle.ps1' `
            -RepositoryRoot $RepositoryRoot `
            -ReleaseRoot $ReleaseRoot `
            -BinaryRoot $BinaryRoot `
            -Deep
    }
    else {
        & '.\Tools\ProjectAudit\Test-Artist31470CustomHandlerOracle.ps1' `
            -RepositoryRoot $RepositoryRoot `
            -ReleaseRoot $ReleaseRoot `
            -BinaryRoot $BinaryRoot
    }

    $receipt = Get-Content -LiteralPath $receiptPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $summary = $receipt.summary
    $delta = $receipt.blockerDelta
    $vss = @($receipt.auditedSearchRoots | Where-Object {
        $_.auditId -ceq 'local.vss.v1'
    })
    if ($receipt.schema -cne 'lostark.effect-source-oracle-acquisition' -or
        [int]$receipt.formatVersion -ne 1 -or
        $receipt.characterClass -cne 'ARTIST' -or
        [int]$receipt.skillId -ne 31470 -or
        $receipt.inputSlot -cne 'F' -or
        [int]$summary.sourceBlockerModuleOccurrenceCount -ne 29 -or
        [int]$summary.sourceBlockerExactClassCount -ne 15 -or
        [int]$summary.nativeFamilyClusterCount -ne 7 -or
        [int]$summary.sourceEraActualOutputProviderCount -ne 0 -or
        [int]$summary.actualMutatedOutputPilotCount -ne 0 -or
        [int]$summary.remainingBlockedModuleOccurrenceCount -ne 29 -or
        [int]$summary.permissionUncheckedRootCount -ne 1 -or
        $summary.evidenceAcquisitionDecision -cne 'PASS_ACCESSIBLE_SCOPE_EXHAUSTED' -or
        $summary.executionReadinessDecision -cne 'BLOCKED' -or
        $summary.nextStageDecision -cne 'NO_GO' -or
        [int]$delta.resolvedModuleOccurrenceCount -ne 0 -or
        [int]$delta.afterBlockedModuleOccurrenceCount -ne 29 -or
        $vss.Count -ne 1 -or
        $vss[0].status -cne 'PERMISSION_UNCHECKED' -or
        [bool]$receipt.nextStageAdmission.allowed -or
        [bool]$receipt.productAdmission.allowed) {
        throw 'Artist F Source oracle acquisition summary contract changed.'
    }

    $mode = if ($Deep) { 'deep' } else { 'shallow' }
    Write-Output (
        "PASS: Artist F 31470 Source oracle acquisition mode=$mode " +
        'classes=15 families=7 blocked=29 providers=0 pilots=0 ' +
        'vss=permission-unchecked nextStage=NO-GO product=false'
    )
}
finally {
    Pop-Location
}
