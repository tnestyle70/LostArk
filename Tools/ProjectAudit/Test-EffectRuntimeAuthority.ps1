[CmdletBinding()]
param(
    [string]$RepositoryRoot = '',
    [string]$HarnessPath = '',
    [string]$ReconstructedProgramPath = '',
    [string]$RuntimeCatalogPath = '',
	[string]$RenderResourceRuntimeCatalogPath = '',
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
        throw "Required Effect runtime authority file is missing: $RelativePath"
    }
    return [IO.File]::ReadAllText($path)
}

$header = Read-RequiredSource 'Client\Public\Effect_RuntimeAuthority.h'
$source = Read-RequiredSource 'Client\Private\Effect_RuntimeAuthority.cpp'
$catalogHeader = Read-RequiredSource 'Client\Public\Effect_Catalog.h'
$catalogSource = Read-RequiredSource 'Client\Private\Effect_Catalog.cpp'
$presentationHeader = Read-RequiredSource `
	'Client\Public\Effect_PresentationService.h'
$presentationSource = Read-RequiredSource `
	'Client\Private\Effect_PresentationService.cpp'
$objectHeader = Read-RequiredSource 'Client\Public\Effect_Object.h'
$objectSource = Read-RequiredSource 'Client\Private\Effect_Object.cpp'
$playbackHeader = Read-RequiredSource 'Client\Public\Effect_Playback.h'
$playbackSource = Read-RequiredSource 'Client\Private\Effect_Playback.cpp'
$rendererHeader = Read-RequiredSource 'Client\Public\Effect_DocumentRenderer.h'
$rendererSource = Read-RequiredSource 'Client\Private\Effect_DocumentRenderer.cpp'
$jsonHeader = Read-RequiredSource 'Client\Public\DataJson.h'
$jsonSource = Read-RequiredSource 'Client\Private\DataJson.cpp'
$harness = Read-RequiredSource `
    'Tools\ClientFrontendHarness\Private\ClientFrontendHarness.cpp'
$clientProject = Read-RequiredSource 'Client\Default\Client.vcxproj'
$clientFilters = Read-RequiredSource 'Client\Default\Client.vcxproj.filters'
$harnessProject = Read-RequiredSource `
    'Tools\ClientFrontendHarness\Default\ClientFrontendHarness.vcxproj'
$harnessFilters = Read-RequiredSource `
    'Tools\ClientFrontendHarness\Default\ClientFrontendHarness.vcxproj.filters'

foreach ($type in @(
    'EFFECT_RUNTIME_DERIVED_IDENTITY',
    'EFFECT_RUNTIME_EXECUTION_CONTRACT',
    'EFFECT_RUNTIME_HANDLER_RECEIPT',
    'EFFECT_RUNTIME_AUTHORITY_IDENTITY',
    'EFFECT_COMPILED_RUNTIME_DOCUMENT',
    'EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM_IDENTITY',
    'EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM',
    'CEffectRuntimeAuthorityCodec')) {
    if ($header -notmatch [regex]::Escape($type)) {
        throw "Effect runtime authority public type is missing: $type"
    }
}
foreach ($identity in @(
    'strSourceContractHash',
    'strSourceSemanticClosureHash',
    'strGeometryContractHash',
    'strMaterialContractHash',
    'strResourceBindingHash',
    'strCompilerInputHash',
    'strCompiledIrSha256',
    'strCompilerReceiptTokenSha256')) {
    if ($header -notmatch [regex]::Escape($identity)) {
        throw "Effect runtime authority identity is missing: $identity"
    }
}
foreach ($boundary in @(
    'IMMUTABLE_COMPILED_IR',
    'COMPILED_AUTHORITY_EXTERNAL_AUTHENTICATION_PENDING',
    'TYPED_RUNTIME_PROGRAM_ADAPTER_PENDING',
    'bTypedProgramMaterialized = false',
    'bExternalIdentityAuthenticated = false',
    'bRuntimeExecutionAdmission = false',
    'bProductAdmission = false',
    'Compiled Effect IR canonical SHA mismatch',
    'Compiled Effect handler execution contract mismatch',
	'Compiled Effect receipt identity mismatch',
	'Get_FrozenArtist31470FProgramIdentity',
	'Parse_ReconstructedRuntimeProgram',
	'LOSTARK_EFFECT_RUNTIME_AUTHORITY_SEMANTIC_TESTS',
	'Validate_ReconstructedRuntimeProgramForHarness',
	'MAXIMUM_PROGRAM_BYTES',
    'MAXIMUM_PROGRAM_DEPTH',
    'MAXIMUM_PROGRAM_VALUES',
    'blockerOwnership',
    'a85b8b41afb2f2a51bceafa55d06bf0937b1a245',
	'72e417747dee14dd0a3be5ffd64f69f904bd696ef1acc049037fc81f38779849',
	'618d5684c94fffa2c21ec0ee911e564fd0f6a1d35fc92843d8efcaeeadd55b4b',
	'94b0bc3704e4c9aaca5216cfafbfcb397c924390395597442c6831d096f2bff5',
	'sourceDeploymentRowId',
    'sourceDeploymentRowSha256',
    'InOutProgram = std::move(Staged)')) {
    if ($source -notmatch [regex]::Escape($boundary)) {
        throw "Effect runtime authority fail-closed boundary is missing: $boundary"
	}
}
if ($header -match 'SemanticTest' -or
	$header -match 'Validate_ReconstructedRuntimeProgramForHarness' -or
	$header -match 'LOSTARK_EFFECT_RUNTIME_AUTHORITY_SEMANTIC_TESTS') {
	throw 'The semantic-test validator must not be part of the production public header.'
}
if ($clientProject -match 'LOSTARK_EFFECT_RUNTIME_AUTHORITY_SEMANTIC_TESTS') {
	throw 'The production Client project must not compile the semantic-test validator.'
}
if ($harnessProject -notmatch 'LOSTARK_EFFECT_RUNTIME_AUTHORITY_SEMANTIC_TESTS') {
	throw 'The ClientFrontendHarness project must explicitly own the semantic-test validator.'
}
foreach ($catalogBoundary in @(
    'g_RuntimeAuthorities',
	'g_RuntimeProgramEntries',
    'Find_RuntimeAuthority',
	'Find_RuntimeProgramEntry',
	'Find_ReconstructedRuntimeProgram',
    'Contains_RuntimeAuthority',
	'Contains_ReconstructedRuntimeProgram',
    'Get_RuntimeAuthorityAssetIds',
	'Get_ReconstructedRuntimeProgramAssetIds',
	'Is_ReconstructedRuntimeProgramAssetId',
    'legacy formatVersion 2 or typed formatVersion 3',
	'Parse_DerivedEntry',
	'IMMUTABLE_RECONSTRUCTED_RUNTIME_PROGRAM',
	'reconstructedRuntimeProgram',
	'publishReceiptSha256',
	'PUBLICATION_PROVENANCE_ONLY_NOT_EXECUTION_AUTHORITY',
	'CANONICAL_JSON_EXCLUDING_RECEIPT_SHA256',
	'74175fe1e41b22ae593a9d1ff92027606bc0b31d62d17927ef6ac5673dd4a7a2',
	'5c91709f2f0ec855c54c94e6dad5bcd7ed048c6133ca9a9af7d4873f20da1bd3',
	'92c883f78d88018a50d8dec09eb6fb155974bec4b3756a796b3499fc2f839d94',
	'5c207e04952971adb553249540e336ba3ad065719e438a9892c6850d2c989c4e',
	'5407c3d0983c3aaf4bf085904ef8d7b5f3e9119ae448703ff7e8f612a1c144fb',
	'ee4a12cf5cbd63bc9af6b0af18ca37da7631a4b0b6ed1465c95bf99fb9be8825',
	'candidateUtf8Json',
	'UTF8_JSON_EXACT',
	'df15009e41b6c1fe9161af873b96dfc428771944786c14f9435f7c0ffa4d869c')) {
    if ($catalogHeader -notmatch [regex]::Escape($catalogBoundary) -and
        $catalogSource -notmatch [regex]::Escape($catalogBoundary)) {
        throw "Effect Catalog format3 boundary is missing: $catalogBoundary"
    }
}
foreach ($renderResourceBoundary in @(
	'EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY',
	'EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY_IDENTITY',
	'Get_RenderResourceAuthority',
	'TextureResourcesById',
	'TextureBindingsById',
	'RecipeTextureBindingsById',
	'RendererSlotBindingsById',
	'RenderStateDescriptorsById',
	'renderResourcePublishReceiptSha256',
	'reconstructedRenderResourceAuthority',
	'lostark.effect-reconstructed-render-resource-authority-link',
	'lostark.effect-reconstructed-render-resource-publication-receipt',
	'PUBLICATION_PROVENANCE_ONLY_NOT_EXECUTION_SUBMIT_RENDER_AUTHORITY',
	'bc5cd1accbbe3c628993a47093dc829eec6f050ab8467fca82f6b7bcf2dfe0ff',
	'bd05c7dca6bdef205b27c208644be19bb94bdbef2e05712bfc49b9b946d8f28a',
	'4efa9ea724df336a5f3af719e24211b7206fe21dfd97becc630f88c5dbd9b412',
	'8a856dd473d49ee255f613c2e25395668c7209e434f7e3a869525a10f4a34c4e',
	'815418a98fbf84a00ae172098a57470b25fac6b5ddfb9836d38dea25db3dafbd',
	'37e1abb8309ac7fbd4244ce0a119db5e8cefc8213d98864e6dc7a4e0f2fa1740',
	'74473d8be1e5930a0809740f1d8240216d4a5478acb9a8ff75001ce0335ceaef',
	'508187b5b905ed714af7c1d18c572f07770a0011b07527e7e16fd61217797e6a',
	'2a2e4bc7fd79164ebb578c7f8f531e7afae7c1f30e5f0d57d3e316bea2fd7922',
	'RESOLVED_EXACT_RUNTIME_COOK_RECEIPT',
	'RESOLVED_RECONSTRUCTED_EXACT_DDS_DEPLOYMENT_RECEIPT')) {
	if ($catalogHeader -notmatch [regex]::Escape($renderResourceBoundary) -and
		$catalogSource -notmatch [regex]::Escape($renderResourceBoundary)) {
		throw "Effect Catalog render-resource boundary is missing: $renderResourceBoundary"
	}
}
if ($catalogSource -notmatch
	'(?s)SourceReceiptStatus != "RESOLVED_EXACT_RUNTIME_COOK_RECEIPT".*SourceReceiptStatus !=\s*"RESOLVED_RECONSTRUCTED_EXACT_DDS_DEPLOYMENT_RECEIPT"') {
	throw 'Render-resource source receipt status must use the exact closed two-value allowlist.'
}
if ($catalogSource -match 'ResourceDocument.*RuntimeAuthorit' -or
    $catalogSource -match 'Compile_Assembly\(.*RuntimeAuthorit') {
    throw 'Compiled runtime authority must not become a raw drawable document.'
}
foreach ($transportBoundary in @(
	'EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY',
	'EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION',
	'pReconstructedRuntimePreparation',
	'Get_ReconstructedRuntimeEntry',
	'Get_ReconstructedRuntimePreparation',
	'CEffectReconstructedRuntimeBoundary',
	'EFFECT_RECONSTRUCTED_ANCHOR_BINDING',
	'strOwnerEmitterId')) {
	if ($catalogHeader -notmatch [regex]::Escape($transportBoundary) -and
		$presentationHeader -notmatch [regex]::Escape($transportBoundary) -and
		$presentationSource -notmatch [regex]::Escape($transportBoundary) -and
		$objectHeader -notmatch [regex]::Escape($transportBoundary) -and
		$playbackHeader -notmatch [regex]::Escape($transportBoundary) -and
		$rendererHeader -notmatch [regex]::Escape($transportBoundary)) {
		throw "Reconstructed Catalog entry transport boundary is missing: $transportBoundary"
	}
}
foreach ($sharedGate in @(
	'Prepare_Presentation',
	'Admit_ProductSpawn',
	'Admit_Execution',
	'Admit_Submit',
	'Admit_Render')) {
	if ($catalogHeader -notmatch [regex]::Escape($sharedGate) -or
		$catalogSource -notmatch [regex]::Escape($sharedGate) -or
		$harness -notmatch [regex]::Escape($sharedGate)) {
		throw "Reconstructed shared production gate is not executable in harness: $sharedGate"
	}
}
foreach ($callsite in @(
	@{ Source = $presentationSource; Token = 'CEffectReconstructedRuntimeBoundary::Prepare_Presentation' },
	@{ Source = $presentationSource; Token = 'CEffectReconstructedRuntimeBoundary::Admit_ProductSpawn' },
	@{ Source = $objectSource; Token = 'EFFECT_RECONSTRUCTED_RUNTIME_SEAM::OBJECT' },
	@{ Source = $objectSource; Token = 'm_ReconstructedRuntimeBoundary.Admit_Execution' },
	@{ Source = $objectSource; Token = 'm_ReconstructedRuntimeBoundary.Admit_Submit' },
	@{ Source = $objectSource; Token = 'm_ReconstructedRuntimeBoundary.Admit_Render' },
	@{ Source = $playbackSource; Token = 'EFFECT_RECONSTRUCTED_RUNTIME_SEAM::PLAYBACK' },
	@{ Source = $playbackSource; Token = 'm_ReconstructedRuntimeBoundary.Admit_Execution' },
	@{ Source = $rendererSource; Token = 'EFFECT_RECONSTRUCTED_RUNTIME_SEAM::RENDERER' },
	@{ Source = $rendererSource; Token = 'm_ReconstructedRuntimeBoundary.Admit_Render' })) {
	if ($callsite.Source -notmatch [regex]::Escape($callsite.Token)) {
		throw "Real Effect callsite does not delegate to shared reconstructed gate: $($callsite.Token)"
	}
}
if ($catalogSource -notmatch 'EXPECTED_OWNER_EMITTER_IDS' -or
	$catalogSource -notmatch 'AnchorRequest count is not frozen at 5') {
	throw 'Reconstructed AnchorRequests must preserve exact five owner emitter tuples.'
}
if (($objectHeader + $playbackHeader + $rendererHeader) -match
	'Stage_ReconstructedRuntimeProgram\s*\([^\)]*EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM') {
	throw 'Raw reconstructed program staging must not bypass the Catalog-owned entry handle.'
}
if ($catalogSource -match 'Data/Effects/Imported/Artist/Candidates' -or
	$catalogSource -match 'reconstructed-runtime-program\.candidate\.json' -or
	$catalogSource -match 'Replace\([^\)]*\\r\\n[^\)]*\\n') {
	throw 'Production Catalog must consume exact embedded candidate bytes without path I/O or EOL normalization.'
}
foreach ($actualCatalogBoundary in @(
	'PublishedCatalogPath',
	'PublishedCatalogStructure',
	'b5086d14940ecb35d3c577024902a080e57f571112f0e79a4f8c8f0aa875509f',
	'PublishedComponents->Get_Array().size() == 555u',
	'PublishedEffects->Get_Array().size() == 102u',
	'WriteCatalog(PublishedCatalogText)',
	'argc <= 3')) {
	if ($harness -notmatch [regex]::Escape($actualCatalogBoundary)) {
		throw "Actual tracked reconstructed Catalog harness boundary is missing: $actualCatalogBoundary"
	}
}
foreach ($renderResourceCatalogBoundary in @(
	'--effect-reconstructed-render-resource-authority',
	'CatalogText.size() == 27''065''827u',
	'ea3afd4e6f2fb2b2a627a8ba565daf9db931e3306c10da7ee5523611bf481ab3',
	'nullptr == EntryA->Get_RenderResourceAuthority()')) {
	if ($harness -notmatch [regex]::Escape($renderResourceCatalogBoundary)) {
		throw "Exact old10/new13 Catalog harness identity is missing: $renderResourceCatalogBoundary"
	}
}
$publishedLoadIndex = $harness.IndexOf(
	'const bool_t LoadedA = PublishedCatalogStructure')
$externalCandidateReadIndex = $harness.IndexOf(
	'const bool_t ExactFixture = Read_FrozenReconstructedProgramFixture')
if ($publishedLoadIndex -lt 0 -or $externalCandidateReadIndex -lt 0 -or
	$publishedLoadIndex -ge $externalCandidateReadIndex) {
	throw 'The tracked runtime Catalog must load before the external candidate is read for synthetic attacks.'
}
if ($jsonHeader -notmatch 'Was_FloatingPointToken' -or
    $jsonSource -notmatch 'bFloatingPointToken') {
    throw 'Canonical runtime JSON must preserve integer versus floating-point token domains.'
}
foreach ($registration in @(
    $clientProject, $clientFilters, $harnessProject, $harnessFilters)) {
    if ($registration -notmatch 'Effect_RuntimeAuthority') {
        throw 'Effect runtime authority project/filter registration is missing.'
    }
}
foreach ($test in @(
    'Runtime Authority Canonical JSON Preserves Integer And Float Token Domains',
    'Format3 Compiled Authority Parses As Immutable Non-Executable Runtime Input',
	'Format3 Generic17 Rejects Reconstructed Extension Field Transactionally',
    'Format3 Product Promotion Rejects Before Replacing Parsed Authority',
    'Format3 Cross-Layer Compiled IR Hash Mutation Rejects Transactionally',
    'Format3 Catalog Commits Compiled Authority Without Raw Drawable Document',
    'Format3 Floating Point Version Rejects And Preserves Prior Catalog',
    'Format3 Catalog Failed Reload Preserves Prior Revision And Pointer',
	'Artist 31470 Synthetic Attack Fixture Uses Exact Frozen LF Candidate Bytes',
	'Artist 31470 Actual Tracked Catalog Commits Exact Immutable Program Identity And Six Renderer Families',
	'Artist 31470 Reconstructed Catalog Has Zero Raw Document Fallback',
	'Artist 31470 Catalog Presentation Object Playback Renderer Preserve One Opaque Handle',
	'Artist 31470 Shared Object Stage Rejects Invalid Restage Transactionally',
	'Artist 31470 Reconstructed Product Spawn Remains Fail Closed',
	'Artist 31470 Catalog Reload Keeps Generation A Alive And Queries Generation B',
	'Artist 31470 Catalog Snapshot Restore And Clear Preserve External Program Lifetimes',
	'Artist 31470 Catalog Malformed Reload Preserves Revision And Program Pointer',
	'Artist 31470 Catalog Candidate Identity Mutation Preserves Revision And Program Pointer',
	'Artist 31470 Catalog Section SHA Mutation Preserves Revision And Program Pointer',
	'Artist 31470 Catalog Program SHA Mutation Preserves Revision And Program Pointer',
	'Artist 31470 Catalog Rejects Embedded CRLF Candidate Without Production Normalization',
	'Artist 31470 Catalog Rejects Missing Outer10 Field',
	'Artist 31470 Catalog Rejects Extra Outer10 Field',
	'Artist 31470 Catalog Rejects Wrong Outer10 Field Type',
	'Artist 31470 Catalog Rejects Missing Link16 Field',
	'Artist 31470 Catalog Rejects Extra Link16 Field',
	'Artist 31470 Catalog Rejects Wrong Link16 Field Type',
	'Artist 31470 Catalog Rejects Reordered Link16 Fields',
	'Artist 31470 Catalog Rejects Missing Receipt25 Field',
	'Artist 31470 Catalog Rejects Extra Receipt25 Field',
	'Artist 31470 Catalog Rejects Wrong Receipt25 Field Type',
	'Artist 31470 Catalog Rejects Reordered Receipt25 Fields',
	'Artist 31470 Catalog Rejects Coordinated Tool Identity And Receipt Reseal',
	'Artist 31470 Catalog Rejects Unsealed Tool Identity Mutation',
	'Artist 31470 Catalog Rejects Generic Payload Authority Laundering',
	'Artist 31470 Catalog Rejects Reordered Strict Payload Fields',
	'Artist 31470 Reserved ID Rejects Legacy Fallback After Failed Reload',
	'Artist 31470 Shared Gates Deny Execution Submit And Render',
	'Artist 31470 Exact13 Harness Input Has Frozen Catalog Identity',
	'Artist 31470 Exact13 Catalog Commits One Immutable Render Resource Authority',
	'Artist 31470 Exact13 Typed Maps Retrieve Frozen Mesh And Sprite M0 Rows',
	'Artist 31470 Exact13 Preparation And Renderer Boundary Preserve Authority Pointer',
	'Artist 31470 Exact13 Sidecar Mutation Rolls Back Entry And Authority Pointers',
	'Artist 31470 Exact13 Link21 Mutation Rolls Back Entry And Authority Pointers',
	'Artist 31470 Exact13 Receipt26 Mutation Rolls Back Entry And Authority Pointers',
	'Artist 31470 Exact13 Outer13 Mutation Rolls Back Entry And Authority Pointers',
	'Artist 31470 Exact Historical Old10 Loads With Null Render Resource Authority',
    'Artist 31470 Reconstructed Runtime Program Parses To Immutable Typed Rows',
	'Artist 31470 Reconstructed Program Rejects Malformed And Raw Identity Attacks Transactionally',
	'Artist 31470 Reconstructed Program Reaches Semantic Validation And Rejects Canonically Resealed Nested Ownership Identity And Admission Attacks Transactionally',
    'Artist 31470 Reconstructed Program Rejects A-B Builder Identity Swap Transactionally')) {
    if ($harness -notmatch [regex]::Escape($test)) {
        throw "Effect runtime authority harness evidence is missing: $test"
	}
}
foreach ($semanticAttack in @(
	'schedule-id',
	'schedule-order',
	'schedule-source-cue',
	'schedule-source-occurrence',
	'schedule-source-system',
	'schedule-source-event-index',
	'schedule-global-time',
	'schedule-duration',
	'schedule-source-cue-row-sha',
	'schedule-event-index-float-token',
	'schedule-global-time-int-token',
	'random-seed-authority',
	'renderer-config-color',
	'distribution-sample-input-digest',
	'material-d3d-descriptor',
	'geometry-carrier-byte-size',
	'Expected semantic rejection stage')) {
	if ($harness -notmatch [regex]::Escape($semanticAttack)) {
		throw "Effect runtime authority resealed semantic attack is missing: $semanticAttack"
	}
}

if (-not [string]::IsNullOrWhiteSpace($HarnessPath)) {
    $resolvedHarness = [IO.Path]::GetFullPath($HarnessPath)
    if (-not (Test-Path -LiteralPath $resolvedHarness -PathType Leaf)) {
        throw "Effect runtime authority harness executable is missing: $resolvedHarness"
    }
	if ([string]::IsNullOrWhiteSpace($ReconstructedProgramPath)) {
		$ReconstructedProgramPath = Join-Path $RepositoryRoot `
			'Data\Effects\Imported\Artist\Candidates\skill.31470.reconstructed-runtime-program.candidate.json'
	}
	$resolvedProgram = [IO.Path]::GetFullPath($ReconstructedProgramPath)
	if (-not (Test-Path -LiteralPath $resolvedProgram -PathType Leaf)) {
		throw "Reconstructed runtime program candidate is missing: $resolvedProgram"
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
	$requiredCatalogTexture = Join-Path $resolvedResourceRoot `
		'Effect\DimensionMaster\Textures\FX_TEX_05\fx_m_noise_001.dds'
	if (-not (Test-Path -LiteralPath $requiredCatalogTexture -PathType Leaf)) {
		throw "Actual tracked Catalog Resource resolution is missing: $requiredCatalogTexture"
	}
	$runtimeAuthorityArguments = @(
		'--effect-runtime-authority', $resolvedProgram, $resolvedCatalog)
	$hadResourceRoot = Test-Path Env:LOSTARK_RESOURCE_ROOT
	$priorResourceRoot = $env:LOSTARK_RESOURCE_ROOT
	try {
		$env:LOSTARK_RESOURCE_ROOT = $resolvedResourceRoot
		$output = & $resolvedHarness $runtimeAuthorityArguments 2>&1
		if ($LASTEXITCODE -ne 0 -or
			($output -join "`n") -notmatch 'failures : 0') {
			throw "Effect runtime authority harness failed:`n$($output -join "`n")"
		}

		$programOutput = & $resolvedHarness `
			'--effect-reconstructed-runtime-program' $resolvedProgram 2>&1
		if ($LASTEXITCODE -ne 0 -or
			($programOutput -join "`n") -notmatch 'failures : 0') {
			throw "Reconstructed runtime program harness failed:`n$($programOutput -join "`n")"
		}

		if (-not [string]::IsNullOrWhiteSpace(
			$RenderResourceRuntimeCatalogPath)) {
			$resolvedRenderResourceCatalog = [IO.Path]::GetFullPath(
				$RenderResourceRuntimeCatalogPath)
			if (-not (Test-Path -LiteralPath $resolvedRenderResourceCatalog -PathType Leaf)) {
				throw "Exact13 Effect runtime Catalog is missing: $resolvedRenderResourceCatalog"
			}
			$renderResourceOutput = & $resolvedHarness `
				'--effect-reconstructed-render-resource-authority' `
				$resolvedRenderResourceCatalog $resolvedCatalog 2>&1
			if ($LASTEXITCODE -ne 0 -or
				($renderResourceOutput -join "`n") -notmatch 'failures : 0') {
				throw "Exact13 render-resource Catalog harness failed:`n$($renderResourceOutput -join "`n")"
			}
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

Write-Output `
	'PASS: format3 immutable compiled authority and distinct reconstructed program are identity-bound, transactionally staged, and runtime/Product blocked'
