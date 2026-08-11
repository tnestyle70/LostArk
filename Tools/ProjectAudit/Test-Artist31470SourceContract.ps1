[CmdletBinding()]
param(
    [string]$RepositoryRoot = '',
    [switch]$DeepSourceAudit,
    [string]$SourceGraphRoot = '',
    [string]$SourceMeshRoot = '',
    [string]$RuntimeMeshRoot = '',
    [string]$CurrentPackageRoot = '',
    [string]$CurrentEFGameScriptPackage = '',
    [string]$CurrentEngineScriptPackage = '',
    [string]$PointLightPackage = '',
    [string]$MeshRotationPackage = '',
    [string]$ColorScalePackage = ''
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    $RepositoryRoot = [IO.Path]::GetFullPath($RepositoryRoot)
}

$candidatePath = Join-Path $RepositoryRoot `
    'Data\Effects\Imported\Artist\Candidates\effect.artist.skill.31470.native-v14.source-contract-candidate.effect.json'
$receiptPath = Join-Path $RepositoryRoot `
    'Data\Effects\Imported\Artist\Candidates\skill.31470.native-v14.source-contract-candidate.receipt.json'
$registryPath = Join-Path $RepositoryRoot `
    'Data\Effects\Contracts\ue3-cascade-source-v1.registry.json'
$sourceReceiptPath = Join-Path $RepositoryRoot `
    'Data\Effects\Imported\Artist\skill.31470.source-receipt.json'
$sourceEvidencePath = Join-Path $RepositoryRoot `
    'Data\Effects\Imported\Artist\skill.31470.source-evidence-envelope.json'
$localReferencePath = Join-Path $RepositoryRoot `
    'Data\Effects\Imported\Artist\Graphs\skill.31470.local-reference-closure.json'
$geometryParityPath = Join-Path $RepositoryRoot `
    'Data\Effects\Imported\Artist\Geometry\skill.31470.wmodel-geometry-parity.receipt.json'
$sourceSemanticClosurePath = Join-Path $RepositoryRoot `
    'Data\Effects\Imported\Artist\Candidates\skill.31470.source-semantic-closure.json'

Push-Location $RepositoryRoot
try {
    if ($DeepSourceAudit) {
        $deepInputs = @(
            $SourceGraphRoot,
            $CurrentPackageRoot,
            $CurrentEFGameScriptPackage,
            $CurrentEngineScriptPackage,
            $PointLightPackage,
            $MeshRotationPackage,
            $ColorScalePackage
        )
        if (@($deepInputs | Where-Object {
            [string]::IsNullOrWhiteSpace($_) -or -not (Test-Path -LiteralPath $_)
        }).Count -ne 0) {
            throw 'Deep Artist F source audit requires seven explicit existing source paths.'
        }

        & python 'Tools/LevelPlacementExtractor/build_artist_31470_local_reference_closure.py' `
            '--source-receipt' 'Data/Effects/Imported/Artist/skill.31470.source-receipt.json' `
            '--action-cue-recipe' 'Data/Effects/Imported/Artist/skill.31470.action-cue-recipe.json' `
            '--active-inventory' 'Data/Effects/Imported/Artist/skill.31470.source-active-effect-inventory.receipt.json' `
            '--normalized-graph' 'Data/Effects/Imported/Artist/Graphs/skill.31470.normalized-effect-graph.json' `
            '--external-module-closure' 'Data/Effects/Imported/Artist/Modules/skill.31470.external-module-closure.json' `
            '--current-package-root' $CurrentPackageRoot `
            '--current-efgame-script-package' $CurrentEFGameScriptPackage `
            '--current-engine-script-package' $CurrentEngineScriptPackage `
            '--point-light-package' $PointLightPackage `
            '--mesh-rotation-recovery-package' $MeshRotationPackage `
            '--color-scale-package' $ColorScalePackage `
            '--output' 'Data/Effects/Imported/Artist/Graphs/skill.31470.local-reference-closure.json' `
            '--check'
        if ($LASTEXITCODE -ne 0) {
            throw "Artist F local-reference closure check failed: $LASTEXITCODE"
        }

        & python 'Tools/LevelPlacementExtractor/build_artist_31470_source_evidence.py' `
            '--source-receipt' 'Data/Effects/Imported/Artist/skill.31470.source-receipt.json' `
            '--action-cue-recipe' 'Data/Effects/Imported/Artist/skill.31470.action-cue-recipe.json' `
            '--active-inventory' 'Data/Effects/Imported/Artist/skill.31470.source-active-effect-inventory.receipt.json' `
            '--normalized-graph' 'Data/Effects/Imported/Artist/Graphs/skill.31470.normalized-effect-graph.json' `
            '--external-module-closure' 'Data/Effects/Imported/Artist/Modules/skill.31470.external-module-closure.json' `
            '--local-reference-closure' 'Data/Effects/Imported/Artist/Graphs/skill.31470.local-reference-closure.json' `
            '--geometry-parity' 'Data/Effects/Imported/Artist/Geometry/skill.31470.wmodel-geometry-parity.receipt.json' `
            '--source-graph-root' $SourceGraphRoot `
            '--output' 'Data/Effects/Imported/Artist/skill.31470.source-evidence-envelope.json' `
            '--check'
        if ($LASTEXITCODE -ne 0) {
            throw "Artist F source-evidence envelope check failed: $LASTEXITCODE"
        }
    }

    & python 'Tools/LevelPlacementExtractor/artist_31470_source_semantic_closure.py' `
        '--active-inventory' 'Data/Effects/Imported/Artist/skill.31470.source-active-effect-inventory.receipt.json' `
        '--normalized-graph' 'Data/Effects/Imported/Artist/Graphs/skill.31470.normalized-effect-graph.json' `
        '--external-module-closure' 'Data/Effects/Imported/Artist/Modules/skill.31470.external-module-closure.json' `
        '--source-evidence' 'Data/Effects/Imported/Artist/skill.31470.source-evidence-envelope.json' `
        '--local-reference-closure' 'Data/Effects/Imported/Artist/Graphs/skill.31470.local-reference-closure.json' `
        '--output' 'Data/Effects/Imported/Artist/Candidates/skill.31470.source-semantic-closure.json' `
        '--check'
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F source-semantic closure check failed: $LASTEXITCODE"
    }

    & python 'Tools/LevelPlacementExtractor/verify_artist_31470_source_semantic_closure.py' `
        '--semantic-closure' 'Data/Effects/Imported/Artist/Candidates/skill.31470.source-semantic-closure.json' `
        '--active-inventory' 'Data/Effects/Imported/Artist/skill.31470.source-active-effect-inventory.receipt.json' `
        '--normalized-graph' 'Data/Effects/Imported/Artist/Graphs/skill.31470.normalized-effect-graph.json' `
        '--external-module-closure' 'Data/Effects/Imported/Artist/Modules/skill.31470.external-module-closure.json' `
        '--source-evidence' 'Data/Effects/Imported/Artist/skill.31470.source-evidence-envelope.json' `
        '--local-reference-closure' 'Data/Effects/Imported/Artist/Graphs/skill.31470.local-reference-closure.json'
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F source-semantic closure oracle failed: $LASTEXITCODE"
    }

    & python 'Tools/LevelPlacementExtractor/build_artist_31470_source_contract.py' `
        '--source-receipt' 'Data/Effects/Imported/Artist/skill.31470.source-receipt.json' `
        '--action-cue-recipe' 'Data/Effects/Imported/Artist/skill.31470.action-cue-recipe.json' `
        '--active-inventory' 'Data/Effects/Imported/Artist/skill.31470.source-active-effect-inventory.receipt.json' `
        '--normalized-graph' 'Data/Effects/Imported/Artist/Graphs/skill.31470.normalized-effect-graph.json' `
        '--module-closure' 'Data/Effects/Imported/Artist/Modules/skill.31470.external-module-closure.json' `
        '--material-closure' 'Data/Effects/Imported/Artist/Materials/skill.31470.active-material-closure.json' `
        '--source-evidence' 'Data/Effects/Imported/Artist/skill.31470.source-evidence-envelope.json' `
        '--local-reference-closure' 'Data/Effects/Imported/Artist/Graphs/skill.31470.local-reference-closure.json' `
        '--geometry-parity' 'Data/Effects/Imported/Artist/Geometry/skill.31470.wmodel-geometry-parity.receipt.json' `
        '--source-semantic-closure' 'Data/Effects/Imported/Artist/Candidates/skill.31470.source-semantic-closure.json' `
        '--output-candidate' 'Data/Effects/Imported/Artist/Candidates/effect.artist.skill.31470.native-v14.source-contract-candidate.effect.json' `
        '--output-receipt' 'Data/Effects/Imported/Artist/Candidates/skill.31470.native-v14.source-contract-candidate.receipt.json' `
        '--output-registry' 'Data/Effects/Contracts/ue3-cascade-source-v1.registry.json' `
        '--output-header' 'Client/Public/Generated/Effect_SourceContractRegistry.generated.h' `
        '--check'
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F source-contract generator check failed: $LASTEXITCODE"
    }

    Push-Location 'Tools/LevelPlacementExtractor'
    try {
        $unitTestRunner = @'
import sys
import unittest

names = [
    "test_build_artist_31470_source_contract",
    "test_build_artist_31470_local_reference_closure",
    "test_artist_31470_source_semantic_closure",
    "test_build_imported_effect_documents",
]
suite = unittest.defaultTestLoader.loadTestsFromNames(names)
result = unittest.TextTestRunner(stream=sys.stdout, verbosity=1).run(suite)
raise SystemExit(0 if result.wasSuccessful() else 1)
'@
        $unitTestOutput = ($unitTestRunner | & python - | Out-String).Trim()
        $unitTestExitCode = $LASTEXITCODE
        if (-not [string]::IsNullOrWhiteSpace($unitTestOutput)) {
            Write-Host $unitTestOutput
        }
        if ($unitTestExitCode -ne 0) {
            throw "Artist F source-contract unit tests failed: $unitTestExitCode"
        }
    }
    finally {
        Pop-Location
    }

    $candidate = Get-Content -LiteralPath $candidatePath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $receipt = Get-Content -LiteralPath $receiptPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $registry = Get-Content -LiteralPath $registryPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $sourceReceipt = Get-Content -LiteralPath $sourceReceiptPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $sourceEvidence = Get-Content -LiteralPath $sourceEvidencePath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $localReference = Get-Content -LiteralPath $localReferencePath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $geometryParity = Get-Content -LiteralPath $geometryParityPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $sourceSemanticClosure = Get-Content -LiteralPath $sourceSemanticClosurePath -Raw -Encoding UTF8 |
        ConvertFrom-Json

    $rendererCounts = @{}
    $moduleReferenceCount = 0
    $localDistributionBindingCount = 0
    $localComponentBindingCount = 0
    $evidenceIds = @{}
    $cueIds = @{}
    foreach ($element in @($candidate.elements)) {
        $name = [string]$element.renderer.type
        $rendererCounts[$name] = 1 + [int]($rendererCounts[$name])
        if ([string]$element.sourceRecipe.sourceContractSha256 -cne
            [string]$registry.contractSha256) {
            throw "Element source-contract hash mismatch: $($element.id)"
        }
        if ([bool]$element.material.sourceProfile.enabled) {
            throw "Source Contract must not enable a runtime material profile: $($element.id)"
        }
        $compiler = $element.sourceRecipe.compilerEvidence
        $moduleReferenceCount += @($compiler.moduleReferenceOrder).Count
        $evidenceIds[[string]$compiler.evidenceId] = $true
        $cueIds[[string]$compiler.sourceCueId] = $true
        foreach ($binding in @($element.sourceRecipe.localReferenceBindings)) {
            if ([string]$binding.referenceKind -ceq 'DISTRIBUTION_TARGET') {
                ++$localDistributionBindingCount
            }
            elseif ([string]$binding.referenceKind -ceq 'TYPEDATA_COMPONENT') {
                ++$localComponentBindingCount
            }
            else {
                throw "Unknown active local-reference kind: $($binding.referenceKind)"
            }
            if ([bool]$binding.executionAdmission.allowed -or
                @($binding.executionAdmission.blockers).Count -eq 0) {
                throw "Local reference is not fail-closed: $($binding.occurrenceId)"
            }
        }
        if ([string]$compiler.sourceEvidenceStatus -cne 'SOURCE_EVIDENCE_PARTIAL' -or
            [string]$compiler.lodSelectionPolicy -cne 'FIRST_LOD_ONLY' -or
            [int]$compiler.selectedLodArrayIndex -ne 0 -or
            [bool]$element.sourceRecipe.compiledExecutionAdmission.allowed) {
            throw "Source compiler evidence is not fail-closed: $($element.id)"
        }
        $geometry = $element.sourceRecipe.geometryBinding
        if ([string]$element.renderer.type -ceq 'meshParticle' -and
            ((-not [bool]$geometry.enabled) -or
             [double]$geometry.carrierGeometryPreScale -ne 0.01 -or
             [string]$geometry.particleScaleSemantics -cne
                'DIMENSIONLESS_AXIS_REORDER_ONLY')) {
            throw "Mesh carrier geometry scale contract changed: $($element.id)"
        }
    }

    $expected = @{
        MeshParticle = 13
        SpriteParticle = 16
        DecalParticle = 3
        CascadeRibbon = 1
        LightParticle = 1
        ScreenPost = 1
    }
    foreach ($entry in $expected.GetEnumerator()) {
        if ([int]$rendererCounts[$entry.Key] -ne [int]$entry.Value) {
            throw "Renderer count mismatch: $($entry.Key)"
        }
    }
    if ($candidate.schema -cne 'lostark.effect-authoring' -or
        [int]$candidate.version -ne 14 -or
        $candidate.purpose -cne 'source_contract' -or
        @($candidate.elements).Count -ne 35 -or
        $sourceReceipt.inputSlot -cne 'R' -or
        $receipt.importedReceiptInputSlot -cne 'R' -or
        $receipt.derivedInputSlot -cne 'F' -or
        $receipt.aggregateSourceEvidenceStatus -cne 'SOURCE_EVIDENCE_PARTIAL' -or
        [bool]$receipt.productAdmission.allowed -or
        [bool]$receipt.summary.sourceContractRuntimeAdmission -or
        [bool]$receipt.summary.visualApprovalComplete -or
        [int]$receipt.summary.excludedExecutionDisabledElementCount -ne 93 -or
        $moduleReferenceCount -ne 399 -or
        $evidenceIds.Count -ne 35 -or
        $cueIds.Count -ne 7 -or
        [int]$sourceEvidence.summary.activeSelectedModuleReferenceOrderCount -ne 399 -or
        [int]$localReference.formatVersion -ne 5 -or
        [int]$localReference.summary.distributionTargetUniqueCount -ne 15 -or
        [int]$localReference.summary.distributionTargetOccurrenceCount -ne 17 -or
        [int]$localReference.summary.receiptPackageIdentityPinnedUniqueCount -ne 8 -or
        [int]$localReference.summary.pinnedPayloadDecodedUniqueCount -ne 7 -or
        [int]$localReference.summary.exactPhysicalSourcePackagePresentUniqueCount -ne 3 -or
        [int]$localReference.summary.distributionTargetPayloadDecodedUniqueCount -ne 14 -or
        [int]$localReference.summary.distributionTargetPayloadUnresolvedUniqueCount -ne 1 -or
        [int]$localReference.summary.distributionTargetPayloadUnresolvedOccurrenceCount -ne 2 -or
        [int]$localReference.summary.distributionTargetSemanticReadyUniqueCount -ne 9 -or
        [int]$localReference.summary.distributionTargetSemanticReadyOccurrenceCount -ne 9 -or
        [int]$localReference.summary.distributionTargetSemanticBlockedUniqueCount -ne 6 -or
        [int]$localReference.summary.distributionTargetSemanticBlockedOccurrenceCount -ne 8 -or
        [int]$localReference.summary.compiledExecutionAllowedOccurrenceCount -ne 0 -or
        $localDistributionBindingCount -ne 17 -or
        $localComponentBindingCount -ne 1 -or
        [int]$receipt.summary.consumedLocalReferenceOccurrenceCount -ne 18 -or
        [int]$receipt.productAdmission.blockerCount -ne
            @($receipt.productAdmission.blockers).Count -or
        ([bool]$receipt.productAdmission.allowed -ne
            ([int]$receipt.productAdmission.blockerCount -eq 0)) -or
        [int]$geometryParity.summary.sourceDuplicateFullPayloadVertexCount -ne 78) {
        throw 'Artist F Source Contract boundary is invalid.'
    }

    $semanticDenominators = $sourceSemanticClosure.summary.denominators
    if ($sourceSemanticClosure.schema -cne 'lostark.effect-source-semantic-closure' -or
        [int]$sourceSemanticClosure.formatVersion -ne 1 -or
        [int]$semanticDenominators.activeOccurrenceCount -ne 35 -or
        [int]$semanticDenominators.selectedLodFieldCount -ne 70 -or
        [int]$semanticDenominators.orderedModuleReferenceCount -ne 399 -or
        [int]$semanticDenominators.topLevelTaggedPropertyCount -ne 1434 -or
        [int]$semanticDenominators.primitiveLeafCount -ne 1572 -or
        [int]$semanticDenominators.distributionCount -ne 629 -or
        [int]$semanticDenominators.unapprovedClassAliasOccurrenceCount -ne 26 -or
        [int]$semanticDenominators.externalNativeTailCount -ne 248 -or
        [int]$semanticDenominators.seededModuleCount -ne 14 -or
        [int]$sourceSemanticClosure.summary.unknownDecisionCount -ne 0 -or
        [int]$sourceSemanticClosure.summary.unconsumedRowCount -ne 0 -or
        [int]$sourceSemanticClosure.summary.silentIgnoredRowCount -ne 0 -or
        -not [bool]$sourceSemanticClosure.summary.allRowsClassified -or
        [bool]$sourceSemanticClosure.summary.semanticExecutionAdmission -or
        [bool]$sourceSemanticClosure.productAdmission.allowed) {
        throw 'Artist F source-semantic closure boundary is invalid.'
    }

    $catalogText = [IO.File]::ReadAllText(
        (Join-Path $RepositoryRoot 'Data\Effects\EffectCatalog.json'))
    if ($catalogText.Contains(
        'effect.artist.skill.31470.native-v14.source-contract-candidate')) {
        throw 'Source Contract candidate entered EffectCatalog.'
    }

    Write-Output `
        'PASS: Artist F 31470 Source Contract cues=7 elements=35 moduleOrder=399 properties=1434/1572 distributions=629 renderers=13/16/3/1/1/1 evidence=partial runtime=false product=false visual=false'
}
finally {
    Pop-Location
}
