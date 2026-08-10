[CmdletBinding()]
param(
    [string]$RepositoryRoot = '',
    [string]$HarnessPath = ''
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    $RepositoryRoot = [IO.Path]::GetFullPath($RepositoryRoot)
}

function Read-RequiredSource([string]$RelativePath) {
    $path = Join-Path $RepositoryRoot $RelativePath
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Required Cascade inspection compiler file is missing: $RelativePath"
    }
    return [IO.File]::ReadAllText($path)
}

$compilerHeader = Read-RequiredSource 'Client\Public\Effect_CascadeCompiler.h'
$compilerSource = Read-RequiredSource 'Client\Private\Effect_CascadeCompiler.cpp'
$playbackHeader = Read-RequiredSource 'Client\Public\Effect_Playback.h'
$playbackSource = Read-RequiredSource 'Client\Private\Effect_Playback.cpp'
$rendererHeader = Read-RequiredSource 'Client\Public\Effect_DocumentRenderer.h'
$rendererSource = Read-RequiredSource 'Client\Private\Effect_DocumentRenderer.cpp'
$presentationSource = Read-RequiredSource 'Client\Private\Effect_PresentationService.cpp'
$clientProject = Read-RequiredSource 'Client\Default\Client.vcxproj'
$clientFilters = Read-RequiredSource 'Client\Default\Client.vcxproj.filters'
$harnessProject = Read-RequiredSource `
    'Tools\ClientFrontendHarness\Default\ClientFrontendHarness.vcxproj'
$harnessFilters = Read-RequiredSource `
    'Tools\ClientFrontendHarness\Default\ClientFrontendHarness.vcxproj.filters'
$harnessSource = Read-RequiredSource `
    'Tools\ClientFrontendHarness\Private\ClientFrontendHarness.cpp'

foreach ($requiredType in @(
    'EFFECT_CASCADE_EMITTER_IDENTITY',
    'EFFECT_CASCADE_MODULE_ROLE',
    'EFFECT_CASCADE_MODULE_REFERENCE',
    'EFFECT_CASCADE_BLOCKER_REQUIREMENT',
    'EFFECT_CASCADE_PROPERTY_KEY',
    'EFFECT_CASCADE_CLASS_LINEAGE_STATUS',
    'EFFECT_CASCADE_PROPERTY_HANDLER_RESULT',
    'EFFECT_CASCADE_PROPERTY_HANDLER_RECEIPT',
    'EFFECT_CASCADE_HANDLER_RECEIPT',
    'EFFECT_CASCADE_INSPECTION_OPCODE',
    'EFFECT_CASCADE_DISTRIBUTION_EVIDENCE',
    'EFFECT_CASCADE_SELECTED_LOD_EVIDENCE',
    'EFFECT_CASCADE_RENDERER_EVIDENCE',
    'EFFECT_CASCADE_GEOMETRY_EVIDENCE',
    'EFFECT_CASCADE_INSPECTION_EMITTER',
    'EFFECT_CASCADE_INSPECTION_SYSTEM',
    'EFFECT_CASCADE_CONSUMPTION_RECEIPT',
    'EFFECT_CASCADE_INSPECTION_IR')) {
    if ($compilerHeader -notmatch [regex]::Escape($requiredType)) {
        throw "Cascade inspection public IR is missing: $requiredType"
    }
}
foreach ($identityField in @(
    'strSourceSystemId',
    'strSourceOccurrenceId',
    'strCanonicalPath',
    'strCanonicalReferenceId',
    'iStableReference',
    'strCanonicalLineageId',
    'strHandlerFieldId',
    'strCanonicalDocumentIdentity',
    'strInspectionHash')) {
    if ($compilerHeader -notmatch [regex]::Escape($identityField)) {
        throw "Cascade inspection identity field is missing: $identityField"
    }
}
if ($compilerHeader -match 'bArtist|LEGACY_PRODUCT_RAW_COMPATIBILITY' -or
    $compilerSource -match 'effect\.artist|31470|bArtist') {
    throw 'Production Cascade compiler contains fixture-specific or legacy execution policy.'
}
if ($compilerSource -match 'Effect_Distribution|Recipe\.Modules|SourceRecipe\.Modules|Module\.Literals|Module\.Distributions|CEffectDocumentCodec::Validate' -or
    $compilerSource -notmatch 'Recipe\.ModuleCoverage' -or
    $compilerSource -notmatch 'ModuleReferenceOrder' -or
    $compilerSource -notmatch 'CEffectDocumentCodec::Serialize\(Document\)') {
    throw 'Source inspection compiler must consume receipt coverage/reference evidence without raw module payload access.'
}
foreach ($requiredBoundary in @(
    'SOURCE_VALUE_PAYLOAD_NOT_MATERIALIZED',
    'SOURCE_EXECUTION_ADMISSION_REJECTED_BEFORE_PAYLOAD',
    'SOURCE_TYPED_DISTRIBUTION_ADAPTER_PENDING',
    'SOURCE_EXACT_CLASS_LINEAGE_ADAPTER_PENDING',
    'CANONICAL_DOCUMENT_CHECKSUM_NOT_AUTHENTICATION',
    'PRODUCT_ADMISSION_DISABLED',
    'RAW_OPCODE_EXECUTOR_UNCHANGED',
	'PROPERTY_BLOCKERS_REQUIRED',
	'MODULE_DEFAULT_OR_NATIVE_TAIL_UNRESOLVED',
	'PINNED_SOURCE_RECORD_PHYSICAL_ABSENT',
	'RECORD_DECODED_PACKAGE_IDENTITY_UNPINNED',
	'SOURCE_EXACT_PHYSICAL_PACKAGE',
    'Staged->bExecutable = false',
    'Staged->bProductAdmission = false')) {
    if ($compilerSource -notmatch [regex]::Escape($requiredBoundary)) {
        throw "Cascade inspection non-executable boundary is missing: $requiredBoundary"
    }
}
if ($compilerSource -notmatch 'Validate_ClassLineage' -or
    $compilerSource -notmatch 'RECEIPT_NORMALIZED_ONLY' -or
    $compilerSource -notmatch 'EXACT_SOURCE_CLASS' -or
    $compilerSource -notmatch 'never becomes an executable alias in G04') {
    throw 'Cascade class lineage must remain typed, explicit, and fail-closed.'
}
foreach ($mutationGate in @(
    'Validate_PropertyEvidenceMatrix',
    'Validate_ModuleEvidenceAggregate',
    'Validate_InspectionStructure',
	'Is_CanonicalBlockerSet',
    'Parse_ModuleRole',
    'Role_MatchesOpcode',
    'Is_EmitterNodeLodLineage',
	'Is_SourceRendererIdentity',
    'Is_SafeGeometryAssetId',
    'unknown storage or duplicate property path',
    'unconsumed or storage-mutated opcode property',
    'free-form LOD provenance promotion',
    'forged selected-LOD path',
    'module alias/reference lineage',
    'stable property reference collision')) {
    if ($compilerSource -notmatch [regex]::Escape($mutationGate)) {
        throw "Cascade inspection structural gate is missing: $mutationGate"
    }
}
foreach ($legacyGap in @(
    'particlemodulecollision',
    'particlemodulesizemultiplyvelocity',
    'particlemodulesubuvmovie',
    'distributionfloatsoundparameter',
    'distributionvectorconstant')) {
    if ($compilerSource -notmatch [regex]::Escape($legacyGap)) {
        throw "Cascade legacy migration report is missing: $legacyGap"
    }
}

foreach ($runtimeText in @(
    $playbackHeader,
    $playbackSource,
    $rendererHeader,
    $rendererSource,
    $presentationSource)) {
    if ($runtimeText -match 'Effect_CascadeCompiler|EFFECT_CASCADE_INSPECTION') {
        throw 'Incomplete inspection compiler must not alter Product playback, renderer, or presentation authority.'
    }
}

foreach ($projectText in @($clientProject, $clientFilters,
    $harnessProject, $harnessFilters)) {
    if ($projectText -notmatch 'Effect_CascadeCompiler') {
        throw 'Cascade inspection compiler project or filter registration is missing.'
    }
}
foreach ($harnessEvidence in @(
    'iSystemCount == 7u',
    'iEmitterCount == 35u',
    'iOrderedOpcodeCount == 399u',
    'iDistributionEvidenceCount == 629u',
    'Simultaneous Source And Coverage Unknown Property',
    'Rejects EF Class Mutation',
    'Self Identical Exact Source Class Without Alias',
    'Receipt Bound Classes Preserve 373 Exact And 26 Alias Required',
    'Free Form Alias As Non Executable Evidence',
    'Exact Class Mismatch As Alias Required Non Executable Evidence',
    'SOURCE_TAGGED To SOURCE_EXACT',
    'Execution Admission Before Payload And Handler Closure',
    'ColorScale Module Property Aggregate Promotion',
    'Rejects Forged Selected LOD Path',
    'Noncanonical Selected LOD Object Suffix',
    'Emitter Node Reused As Selected LOD Node',
    'Emitter Node Package Mismatch',
    'Alias ID And Source Reference Lineage Drift',
    'Opcode Alias Schema Mismatch',
    'REQUIRED Opcode With Valid Looking MODULE Role',
    'Rejects Duplicate Property Path And Reference',
    'Rejects Unknown Property Storage',
    'Rejects Duplicate Module Reference Index',
    'Rejects Nonfinite Typed Evidence',
	'Element And Renderer Identity Mismatch',
    'Geometry And Resource Binding Mismatch',
	'Mesh Renderer Geometry Binding Removal',
    'Raw B Reusing A Canonical Identity',
    'Fabricated Default Inspection',
    'Hash Binds Source Emitter Node And LOD Lineage',
    'Forged Opcode Handler Field Consumption Receipt',
	'Stripped Property Fidelity Blocker',
	'Required Property Blocker Loss',
	'Fabricated Property Fidelity Blocker',
    'Explicit Migration Gaps')) {
    if ($harnessSource -notmatch [regex]::Escape($harnessEvidence)) {
        throw "Cascade inspection harness evidence is missing: $harnessEvidence"
    }
}

if (-not [string]::IsNullOrWhiteSpace($HarnessPath)) {
    $resolvedHarness = [IO.Path]::GetFullPath($HarnessPath)
    if (-not (Test-Path -LiteralPath $resolvedHarness -PathType Leaf)) {
        throw "Cascade inspection harness executable is missing: $resolvedHarness"
    }
    $candidate = Join-Path $RepositoryRoot `
        'Data\Effects\Imported\Artist\Candidates\effect.artist.skill.31470.native-v14.source-contract-candidate.effect.json'
    $output = & $resolvedHarness '--effect-source-contract' $candidate 2>&1
    if ($LASTEXITCODE -ne 0 -or
        ($output -join "`n") -notmatch 'failures : 0' -or
        ($output -join "`n") -notmatch
            'Inspection IR Preserves Fixture 7 Systems 35 Emitters 399 Opcodes 629 Isolated Distributions') {
        throw "Cascade inspection compiler harness failed:`n$($output -join "`n")"
    }
}

Write-Output `
    'PASS: non-executable Cascade source-inspection IR preserves typed identities, handler receipts, and raw runtime isolation'
