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
    'Data\Effects\Imported\Artist\Candidates\skill.31470.reconstructed-source-capability.receipt.json'

Push-Location $RepositoryRoot
try {
    & python -B `
        'Tools/LevelPlacementExtractor/test_build_artist_31470_reconstructed_source_capability.py'
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F reconstructed Source capability tests failed: $LASTEXITCODE"
    }

    & python -B `
        'Tools/LevelPlacementExtractor/build_artist_31470_reconstructed_source_capability.py' `
        '--check'
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F reconstructed Source capability check failed: $LASTEXITCODE"
    }

    if ($Deep) {
        & '.\Tools\ProjectAudit\Test-Artist31470SourceOracleAcquisition.ps1' `
            -RepositoryRoot $RepositoryRoot `
            -ReleaseRoot $ReleaseRoot `
            -BinaryRoot $BinaryRoot `
            -Deep
    }
    else {
        & '.\Tools\ProjectAudit\Test-Artist31470SourceOracleAcquisition.ps1' `
            -RepositoryRoot $RepositoryRoot `
            -ReleaseRoot $ReleaseRoot `
            -BinaryRoot $BinaryRoot
    }
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F Source oracle acquisition prerequisite failed: $LASTEXITCODE"
    }

    $receipt = Get-Content -LiteralPath $receiptPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $summary = $receipt.summary
    $expectedFamilyCounts = [ordered]@{
        'source.reconstructed.seeded.v1' = 11
        'source.reconstructed.cylinder-spin.v1' = 5
        'source.reconstructed.ground.v1' = 2
        'source.reconstructed.decal.v1' = 3
        'source.reconstructed.light.v1' = 1
        'source.reconstructed.velocity.v1' = 4
        'source.reconstructed.ef-vector-multiply.v1' = 3
    }
    foreach ($entry in $expectedFamilyCounts.GetEnumerator()) {
        $actual = @($receipt.occurrences | Where-Object {
            $_.policyFamilyId -ceq $entry.Key
        }).Count
        if ($actual -ne [int]$entry.Value) {
            throw "Artist F reconstructed family count changed: $($entry.Key)=$actual"
        }
    }

    $samples = @($receipt.occurrences.numericSamples)
    $badPolicies = @($receipt.familyPolicies | Where-Object {
        [string]::IsNullOrWhiteSpace($_.implementationId) -or
        [int]$_.implementationVersion -ne 1 -or
        $_.implementationSha256 -notmatch '^[0-9a-f]{64}$' -or
        $_.familySemanticImplementationSha256 -notmatch '^[0-9a-f]{64}$' -or
        [bool]$_.semanticContract.genericFallbackAllowed -or
        [bool]$_.sourceExact -or
        [bool]$_.runtimeExecutionAdmission -or
        [bool]$_.productAdmission
    })
    $badRows = @($receipt.occurrences | Where-Object {
        $_.capabilityDecision -cne 'READY_FOR_RECONSTRUCTED_REVIEW' -or
        $_.sourceEvidenceFidelity -cne 'CURRENT_REVISION_CROSS_REVISION_EVIDENCE' -or
        [bool]$_.sourceExact -or
        [bool]$_.currentEvidencePromotedToSourceExact -or
        [bool]$_.runtimeExecutionAdmission -or
        [bool]$_.productAdmission -or
        @($_.numericSamples).Count -ne 3 -or
        @($_.preservedEvidenceBlockers) -notcontains 'SOURCE_EXACT_NOT_CLAIMED' -or
        @($_.preservedEvidenceBlockers) -notcontains 'CURRENT_REVISION_CROSS_REVISION_EVIDENCE'
    })

    if ($receipt.schema -cne 'lostark.effect-reconstructed-source-capability' -or
        [int]$receipt.formatVersion -ne 1 -or
        $receipt.characterClass -cne 'ARTIST' -or
        [int]$receipt.skillId -ne 31470 -or
        $receipt.inputSlot -cne 'F' -or
        $receipt.policy.name -cne 'RECONSTRUCTED_APPROVED_V1' -or
        [int]$summary.policyFamilyCount -ne 7 -or
        [int]$summary.moduleOccurrenceCount -ne 29 -or
        [int]$summary.readyForReconstructedReviewCount -ne 29 -or
        [int]$summary.numericSampleCount -ne 87 -or
        [int]$summary.numericSampleMissingReadyCount -ne 0 -or
        [int]$summary.sourcePropertyRowCount -ne 148 -or
        [int]$summary.policyInputConsumedPropertyCount -ne 117 -or
        [int]$summary.preservedIrrelevantPropertyCount -ne 31 -or
        [int]$summary.unconsumedPropertyCount -ne 0 -or
        [int]$summary.distributionBindingCount -ne 65 -or
        [int]$summary.sourceEraIdentityPinnedDistributionCount -ne 0 -or
        [int]$summary.sourceEraIdentityUnpinnedDistributionCount -ne 65 -or
        [int]$summary.sourceEraIdentityFieldMissingDistributionCount -ne 60 -or
        [int]$summary.globalDuplicateIdCount -ne 0 -or
        [int]$summary.unknownRowCount -ne 0 -or
        [int]$summary.ownerlessRowCount -ne 0 -or
        [int]$summary.genericFallbackCount -ne 0 -or
        [int]$summary.sourceExactCount -ne 0 -or
        [int]$summary.currentEvidencePromotedCount -ne 0 -or
        [int]$summary.runtimeExecutionAdmissionCount -ne 0 -or
        [int]$summary.productAdmissionCount -ne 0 -or
        @($receipt.familyPolicies).Count -ne 7 -or
        @($receipt.executionDependencies).Count -ne 5 -or
        @($receipt.occurrences).Count -ne 29 -or
        $samples.Count -ne 87 -or
        $badPolicies.Count -ne 0 -or
        $badRows.Count -ne 0 -or
        [bool]$receipt.runtimeExecutionAdmission.allowed -or
        [bool]$receipt.productAdmission.allowed) {
        throw 'Artist F reconstructed Source capability summary or fail-closed contract changed.'
    }

    $mode = if ($Deep) { 'deep' } else { 'shallow' }
    Write-Output (
        "PASS: Artist F 31470 reconstructed Source capability mode=$mode " +
        'families=7 occurrences=29 properties=148 distributions=65 samples=87 unknown=0 ownerless=0 ' +
        'genericFallback=0 sourceExact=0 execution=false product=false'
    )
}
finally {
    Pop-Location
}
