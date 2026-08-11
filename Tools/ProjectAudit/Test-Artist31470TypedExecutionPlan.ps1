[CmdletBinding()]
param(
    [string]$RepositoryRoot = '',
    [string]$HarnessPath = '',
    [string]$ReconstructedProgramPath = '',
    [string]$RuntimeCatalogPath = '',
    [string]$ResourceRoot = ''
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
        throw "Required Artist 31470 typed execution file is missing: $RelativePath"
    }
    return [IO.File]::ReadAllText($path)
}

$header = Read-RequiredSource 'Client\Public\Effect_ReconstructedExecution.h'
$source = Read-RequiredSource 'Client\Private\Effect_ReconstructedExecution.cpp'
$playbackHeader = Read-RequiredSource 'Client\Public\Effect_Playback.h'
$playbackSource = Read-RequiredSource 'Client\Private\Effect_Playback.cpp'
$harness = Read-RequiredSource `
    'Tools\ClientFrontendHarness\Private\ClientFrontendHarness.cpp'
$clientProject = Read-RequiredSource 'Client\Default\Client.vcxproj'
$clientFilters = Read-RequiredSource 'Client\Default\Client.vcxproj.filters'
$harnessProject = Read-RequiredSource `
    'Tools\ClientFrontendHarness\Default\ClientFrontendHarness.vcxproj'
$harnessFilters = Read-RequiredSource `
    'Tools\ClientFrontendHarness\Default\ClientFrontendHarness.vcxproj.filters'
$executionContract = $header + $source

foreach ($type in @(
    'EFFECT_RECONSTRUCTED_EXECUTION_PLAN_IDENTITY',
    'EFFECT_RECONSTRUCTED_EXECUTION_PLAN_SUMMARY',
    'EFFECT_RECONSTRUCTED_EXECUTION_SEEDED_LIFETIME_AUTHORITY',
    'EFFECT_RECONSTRUCTED_EXECUTION_SCHEDULE',
    'EFFECT_RECONSTRUCTED_EXECUTION_EMITTER',
    'EFFECT_RECONSTRUCTED_EXECUTION_MODULE',
    'EFFECT_RECONSTRUCTED_EXECUTION_DISTRIBUTION',
    'EFFECT_RECONSTRUCTED_EXECUTION_SEED_POLICY',
    'EFFECT_RECONSTRUCTED_CPU_INSPECTION_STATE',
    'EFFECT_RECONSTRUCTED_CPU_INSPECTION_FRAME',
    'EFFECT_RECONSTRUCTED_CPU_OCCURRENCE_PACKET',
    'CEffectReconstructedExecutionPlanCompiler',
    'CEffectReconstructedCpuInspector')) {
    if ($header -notmatch [regex]::Escape($type)) {
        throw "Typed execution public contract is missing: $type"
    }
}

foreach ($boundary in @(
    'EFFECT_RECONSTRUCTED_EXECUTION_PLAN_VERSION = 1u',
    'EFFECT_RECONSTRUCTED_OCCURRENCE_RNG_VERSION = 1u',
    'EFFECT_RECONSTRUCTED_FIXED_STEP_HZ = 60u',
    'EXPECTED_SCHEDULE_EMITTER_COUNTS',
    '4u, 1u, 15u, 12u, 1u, 1u, 1u',
    '612u, 8u, 5u, 1u, 3u',
    '509u, 120u',
    'Data.Summary.iBurstCount != 31u',
    'Data.Summary.iLifetimeModuleCount != 34u',
    'Data.Summary.iSeededLifetimeModuleCount != 1u',
    'Data.Summary.iOperationalCapSum != 1291u',
    'Data.Summary.iOperationalCapMaximum != 594u',
    'Data.Summary.iRibbonOperationalMaxPoints != 500u',
    'Program.Handlers.size() != 385u',
    'Program.Emitters.size() != 35u',
    'Program.ActionSchedules.size() != 7u',
    'Program.Modules.size() != 399u',
    'Program.Distributions.size() != 629u',
    'DistributionSampleCount != 1852u',
    'EvaluatorSampleCount != 1839u',
    'ParameterSampleCount != 13u',
    'RegisterStableId',
    'ReferencedModuleIds',
    'strEvaluatorRegistryId',
    'bCpuTimingExecutable',
    'Reconstructed execution timing cannot use a deferred EF evaluator',
    'Reconstructed execution module handler identity or selection is invalid',
    'Reconstructed execution distribution enum is unknown',
    '9f99d7a65d6e2a74bc241dd4268751fc876522c9deac387eacfb40be7cc429b1',
    '35b0977b106c003b2542327959dd1ee11ea326dd17324c5dbc242153b086bd88',
    'Build_DistributionOwnerProjection',
    'FROZEN_SEEDED_MODULE_HANDLER_ID',
    'FROZEN_SEEDED_PROPERTY_HANDLER_ID',
    'FROZEN_SEEDED_CAPABILITY_IMPLEMENTATION_SHA256',
    'Reconstructed execution frozen seeded lifetime authority tuple is invalid',
    'Reconstructed execution frozen distribution target projection is invalid',
    'Reconstructed execution frozen distribution owner projection is invalid',
    'Reconstructed execution property distribution reverse membership or order is invalid',
    'Reconstructed CPU inspection refuses a non-positive lifetime fallback',
    'strSemanticProjectionSha256')) {
    if ($executionContract -notmatch [regex]::Escape($boundary)) {
        throw "Typed execution fail-closed boundary is missing: $boundary"
    }
}

foreach ($forbiddenRawInput in @(
    'SourceRecipe',
    'DetailTransform',
    'RendererRuntimeConfig',
    'EFFECT_DETAIL_DESC')) {
    if ($source -match [regex]::Escape($forbiddenRawInput)) {
        throw "Typed execution compiler consumes a forbidden raw input: $forbiddenRawInput"
    }
}

if ($clientProject -match 'LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS') {
    throw 'The production Client must not compile the CPU inspection simulator.'
}
if ($harnessProject -notmatch 'LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS') {
    throw 'ClientFrontendHarness must explicitly own the CPU inspection simulator.'
}
if ($playbackSource -match 'CEffectReconstructedCpuInspector::Simulate') {
    throw 'Production Playback must not execute the inspection-only CPU simulator.'
}
foreach ($registration in @(
    $clientProject, $clientFilters, $harnessProject, $harnessFilters)) {
    if ($registration -notmatch 'Effect_ReconstructedExecution') {
        throw 'Typed execution project/filter registration is missing.'
    }
}

foreach ($playbackBoundary in @(
    'Get_ReconstructedExecutionPlan',
    'CEffectReconstructedExecutionPlanCompiler::Compile_Preparation',
    'm_pReconstructedExecutionPlan = std::move(StagedPlan)',
    'm_pReconstructedExecutionPlan.reset()',
    'm_ReconstructedRuntimeBoundary.Admit_Execution')) {
    if (($playbackHeader + $playbackSource) -notmatch
        [regex]::Escape($playbackBoundary)) {
        throw "Playback typed execution boundary is missing: $playbackBoundary"
    }
}
$compileIndex = $playbackSource.IndexOf(
    'CEffectReconstructedExecutionPlanCompiler::Compile_Preparation')
$boundaryCommitIndex = $playbackSource.IndexOf(
    'm_ReconstructedRuntimeBoundary = std::move(StagedBoundary)')
$planCommitIndex = $playbackSource.IndexOf(
    'm_pReconstructedExecutionPlan = std::move(StagedPlan)')
if ($compileIndex -lt 0 -or $boundaryCommitIndex -le $compileIndex -or
    $planCommitIndex -le $boundaryCommitIndex) {
    throw 'Playback must compile the Plan before atomically committing boundary then Plan.'
}

foreach ($test in @(
    'Artist 31470 Typed Plan Freezes 7/35/399/629 Schedule Lifetime Burst Cap And Distribution Denominators',
    'Artist 31470 Typed Plan Uses Exact Stable IDs And Complete Reverse Joins Without Vector Index Identity',
    'Artist 31470 CPU Inspection Uses 60Hz Schedule And Emitter Delay Gates',
    'Artist 31470 CPU Inspection RNG Packet Order Lifetime And Caps Are Deterministic',
    'Artist 31470 CPU Inspection Completes Exact Loop And Lifetime Tails Without Fallback',
    'Artist 31470 CPU Inspection Invalid Sample Preserves Prior State And Frame',
    'Artist 31470 Typed Plan Rejects A B Identity Unknown Variant And Broken Reverse Join Transactionally',
    'Artist 31470 Typed Plan Rejects Coordinated Seeded Lifetime Full Object Authority Swap Transactionally',
    'Artist 31470 Typed Plan Rejects Bracket Qualified Dynamicparams Target Mutation Transactionally',
    'Artist 31470 Typed Plan Rejects Coordinated Dynamicparams Owner Classification Mutation Transactionally',
    'Artist 31470 Typed Plan Rejects Extra Property Distribution Reverse Membership Transactionally',
    'Artist 31470 Typed Plan Rejects Duplicate Property Distribution Reverse Membership Transactionally',
    'Artist 31470 Typed Plan Rejects Reordered Property Distribution Reverse Membership Transactionally',
    'Artist 31470 Typed Plan Is Invariant To Raw SourceRecipe Detail And Renderer Config Poison',
    'Artist 31470 Playback Invalid Plan Restage Preserves Plan And Preparation Pointers',
    'Artist 31470 Typed Plan Reload Produces Independent A B Identity And Preserves Generation A',
    '--effect-reconstructed-execution-plan')) {
    if ($harness -notmatch [regex]::Escape($test)) {
        throw "Typed execution harness evidence is missing: $test"
    }
}

if ([string]::IsNullOrWhiteSpace($ReconstructedProgramPath)) {
    $ReconstructedProgramPath = Join-Path $RepositoryRoot `
        'Data\Effects\Imported\Artist\Candidates\skill.31470.reconstructed-runtime-program.candidate.json'
}
$resolvedProgram = [IO.Path]::GetFullPath($ReconstructedProgramPath)
if (-not (Test-Path -LiteralPath $resolvedProgram -PathType Leaf)) {
    throw "Reconstructed runtime candidate is missing: $resolvedProgram"
}
$candidateBytes = [IO.File]::ReadAllBytes($resolvedProgram)
$candidateSha = (Get-FileHash -LiteralPath $resolvedProgram -Algorithm SHA256).Hash.ToLowerInvariant()
if ($candidateBytes.Length -ne 15072141 -or
    $candidateSha -ne '72e417747dee14dd0a3be5ffd64f69f904bd696ef1acc049037fc81f38779849' -or
    $candidateBytes -contains 13) {
    throw 'Typed execution input must remain the exact frozen LF candidate.'
}

if (-not [string]::IsNullOrWhiteSpace($HarnessPath)) {
    $resolvedHarness = [IO.Path]::GetFullPath($HarnessPath)
    if (-not (Test-Path -LiteralPath $resolvedHarness -PathType Leaf)) {
        throw "Typed execution harness executable is missing: $resolvedHarness"
    }
    if ([string]::IsNullOrWhiteSpace($RuntimeCatalogPath)) {
        $RuntimeCatalogPath = Join-Path $RepositoryRoot `
            'Client\Bin\DataFiles\Effect\EffectCatalog.runtime.json'
    }
    $resolvedCatalog = [IO.Path]::GetFullPath($RuntimeCatalogPath)
    if (-not (Test-Path -LiteralPath $resolvedCatalog -PathType Leaf)) {
        throw "Tracked Effect runtime catalog is missing: $resolvedCatalog"
    }
    if ([string]::IsNullOrWhiteSpace($ResourceRoot)) {
        if (-not [string]::IsNullOrWhiteSpace($env:LOSTARK_RESOURCE_ROOT)) {
            $ResourceRoot = $env:LOSTARK_RESOURCE_ROOT
        }
        else {
            $ResourceRoot = Join-Path $RepositoryRoot 'Client\Bin\Resources'
        }
    }
    $resolvedResourceRoot = [IO.Path]::GetFullPath($ResourceRoot)
    if (-not (Test-Path -LiteralPath $resolvedResourceRoot -PathType Container)) {
        throw "Effect runtime Resource root is missing: $resolvedResourceRoot"
    }
    $hadResourceRoot = Test-Path Env:LOSTARK_RESOURCE_ROOT
    $priorResourceRoot = $env:LOSTARK_RESOURCE_ROOT
    try {
        $env:LOSTARK_RESOURCE_ROOT = $resolvedResourceRoot
        $output = & $resolvedHarness `
            '--effect-reconstructed-execution-plan' `
            $resolvedProgram $resolvedCatalog 2>&1
        if ($LASTEXITCODE -ne 0 -or
            ($output -join "`n") -notmatch 'failures : 0') {
            throw "Artist 31470 typed execution harness failed:`n$($output -join "`n")"
        }
    }
    finally {
        if ($hadResourceRoot) {
            $env:LOSTARK_RESOURCE_ROOT = $priorResourceRoot
        }
        else {
            Remove-Item Env:LOSTARK_RESOURCE_ROOT -ErrorAction SilentlyContinue
        }
    }
}

Write-Output 'PASS: Artist 31470 typed execution plan schedules=7 emitters=35 modules=399 distributions=629 rng=v1 fixedHz=60 Product=false'
