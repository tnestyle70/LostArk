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

function Assert-RequiredFileIdentity(
	[string]$RelativePath,
	[long]$ExpectedByteCount,
	[string]$ExpectedSha256) {
	$path = Join-Path $RepositoryRoot $RelativePath
	if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
		throw "Required Effect runtime authority artifact is missing: $RelativePath"
	}
	$bytes = [IO.File]::ReadAllBytes($path)
	$sha256 = [Security.Cryptography.SHA256]::Create()
	try {
		$actualSha256 = ([BitConverter]::ToString(
			$sha256.ComputeHash($bytes))).Replace('-', '').ToLowerInvariant()
	}
	finally {
		$sha256.Dispose()
	}
	if ($bytes.LongLength -ne $ExpectedByteCount -or
		$actualSha256 -cne $ExpectedSha256) {
		throw "Effect runtime authority artifact identity is stale: $RelativePath"
	}
}

Assert-RequiredFileIdentity `
	'Data\Effects\Imported\Artist\Candidates\skill.31470.reconstructed-runtime-program.candidate.json' `
	15117436 `
	'bdeccba5b204ffae0bc88469b90158ff3479da0a113c437c2842f1f91f5f04f6'
Assert-RequiredFileIdentity `
	'Data\Effects\Imported\Artist\Materials\skill.31470.reconstructed-render-resource-authority.receipt.json' `
	774127 `
	'1567c622876f74018ac9a21a4ba9e04dd8a3fd08f0bfe934698a65b8185d2660'
Assert-RequiredFileIdentity `
	'Client\Bin\DataFiles\Effect\EffectCatalog.runtime.json' `
	27144447 `
	'e8412973025b01510e3a81ac7c119ac58994ab126aa8cf624afa8c126d342f8d'

$header = Read-RequiredSource 'Client\Public\Effect_RuntimeAuthority.h'
$source = Read-RequiredSource 'Client\Private\Effect_RuntimeAuthority.cpp'
$catalogHeader = Read-RequiredSource 'Client\Public\Effect_Catalog.h'
$catalogSource = Read-RequiredSource 'Client\Private\Effect_Catalog.cpp'
$executionHeader = Read-RequiredSource `
	'Client\Public\Effect_ReconstructedExecution.h'
$executionSource = Read-RequiredSource `
	'Client\Private\Effect_ReconstructedExecution.cpp'
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
    '31ecc2edc328347ac6e3bf6fe444c270d463ef40',
	'bdeccba5b204ffae0bc88469b90158ff3479da0a113c437c2842f1f91f5f04f6',
	'8e618a53242fb2fee9b13528d9696182038ded977454d98ff49ff500570ebeb8',
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
	'a60613cdbef3db5ee2bf660f947bf809309e551193c5b8e53e6908729916c9c2',
	'ebdd01e2815990308eb703abc4b73e4c99a051a2cb4ff36d553f9bc6e61ac240',
	'5812cbc6794705e2644853db596e3c55ebace8faf4079a4a0b3f374b1c203836',
	'5c207e04952971adb553249540e336ba3ad065719e438a9892c6850d2c989c4e',
	'5407c3d0983c3aaf4bf085904ef8d7b5f3e9119ae448703ff7e8f612a1c144fb',
	'ee4a12cf5cbd63bc9af6b0af18ca37da7631a4b0b6ed1465c95bf99fb9be8825',
	'candidateUtf8Json',
	'UTF8_JSON_EXACT',
	'df15009e41b6c1fe9161af873b96dfc428771944786c14f9435f7c0ffa4d869c',
	'da83b7d05b8d97357fa379b3a5c48bdb4296883647e455babc55adaff09b8ef6',
	'15''117''436u')) {
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
	'1567c622876f74018ac9a21a4ba9e04dd8a3fd08f0bfe934698a65b8185d2660',
	'6f4ed12c7c5b6499ece7cf520436f747e4877a4a89a1584ba57de7324adf8ac4',
	'fcef9bb95c5412f1d25f206e207b6eccd8198a26a8994a6ee5ac179498b001de',
	'1706bb76180d3650c043db59b4eb09291fde3b7c3a5884675270a4f887bbb16b',
	'63d7107714b6eb812f709f6d97876665b4929ef51785dd50cc59eb3015d3601c',
	'01cd26412754ffd8e7acb4e7c1fe4280ec2dfab65077a664c86eac6a415b8541',
	'1536d7b780f6787c624bbad1150889fe7f63e78ede91067029856b07b9718d02',
	'e7a630b9d94dfb177b3f678561865bd9e7bad2dd3f1eb082656b79e5c3af3190',
	'774''127u',
	'RESOLVED_EXACT_RUNTIME_COOK_RECEIPT',
	'RESOLVED_RECONSTRUCTED_EXACT_DDS_DEPLOYMENT_RECEIPT')) {
	if ($catalogHeader -notmatch [regex]::Escape($renderResourceBoundary) -and
		$catalogSource -notmatch [regex]::Escape($renderResourceBoundary)) {
		throw "Effect Catalog render-resource boundary is missing: $renderResourceBoundary"
	}
}
if ($catalogSource -notmatch
	'(?s)Parse_RenderTextureResources\(.*?Value\.Get_Array\(\)\.size\(\) != 52u' -or
	$catalogSource -notmatch
	'(?s)Parse_RenderTextureBindings\(.*?Value\.Get_Array\(\)\.size\(\) != 77u' -or
	$catalogSource -notmatch
	'(?s)iTextureResourceCount != 52u.*?iTextureBindingCount != 77u') {
	throw 'Render-resource parser and sidecar summary must share the current exact 52-resource/77-binding denominators.'
}
if ($catalogSource -notmatch
	'(?s)SourceReceiptStatus != "RESOLVED_EXACT_RUNTIME_COOK_RECEIPT".*SourceReceiptStatus !=\s*"RESOLVED_RECONSTRUCTED_EXACT_DDS_DEPLOYMENT_RECEIPT"') {
	throw 'Render-resource source receipt status must use the exact closed two-value allowlist.'
}
foreach ($selectedEvaluatorBoundary in @(
	'EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_VERSION',
	'artist-f.selected-occurrence-xorshift32.v1',
	'EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_REQUEST',
	'EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARATION',
	'EFFECT_RECONSTRUCTED_SELECTED_PACKET',
	'EFFECT_RECONSTRUCTED_SELECTED_FRAME',
	'CEffectReconstructedSelectedEvaluator',
	'Get_RandomDrawCount',
	'vMeshDimensionlessScaleXzy',
	'vSpriteSignedWorldSizeXzy',
	'CAMERA_BILLBOARD_WITH_VELOCITY_ALIGNMENT',
	'RendererTextureResource',
	'SidecarRendererSlotDecision',
	'strPayloadSha256',
	'strMetadataIdentitySha256')) {
	if ($executionHeader -notmatch [regex]::Escape($selectedEvaluatorBoundary)) {
		throw "Selected production evaluator public boundary is missing: $selectedEvaluatorBoundary"
	}
}
function Test-SelectedPublicTokensOutsideTestGuard(
	[string]$Text,
	[string[]]$Tokens) {
	$found = @{}
	foreach ($token in $Tokens) {
		$found[$token] = $false
	}
	$testGuardDepth = 0
	foreach ($line in [regex]::Split($Text, "`r?`n")) {
		$trimmed = $line.TrimStart()
		if ($trimmed -match '^#\s*(if|ifdef|ifndef)\b') {
			if ($testGuardDepth -gt 0) {
				$testGuardDepth++
			}
			elseif ($trimmed -match
				'LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS') {
				$testGuardDepth = 1
			}
		}
		foreach ($token in $Tokens) {
			if ($line.IndexOf($token, [StringComparison]::Ordinal) -lt 0) {
				continue
			}
			$found[$token] = $true
			if ($testGuardDepth -gt 0) {
				return $false
			}
		}
		if ($trimmed -match '^#\s*endif\b' -and $testGuardDepth -gt 0) {
			$testGuardDepth--
		}
	}
	foreach ($token in $Tokens) {
		if (-not $found[$token]) {
			return $false
		}
	}
	return $true
}
$selectedPublicTypeTokens = @(
	'enum class EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND',
	'enum class EFFECT_RECONSTRUCTED_SPRITE_ALIGNMENT',
	'enum class EFFECT_RECONSTRUCTED_SPRITE_ORIENTATION',
	'struct EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY',
	'struct EFFECT_RECONSTRUCTED_SELECTED_HANDLER_IDENTITY',
	'struct EFFECT_RECONSTRUCTED_SELECTED_CYLINDER_POLICY',
	'struct EFFECT_RECONSTRUCTED_SELECTED_TEXTURE_LANE',
	'struct EFFECT_RECONSTRUCTED_SELECTED_STATE_BINDING',
	'struct EFFECT_RECONSTRUCTED_SELECTED_GEOMETRY_BINDING',
	'struct EFFECT_RECONSTRUCTED_SELECTED_SPRITE_SINK',
	'struct EFFECT_RECONSTRUCTED_SELECTED_MATERIAL_CONSTANTS',
	'struct EFFECT_RECONSTRUCTED_SELECTED_SHADER_BINDING',
	'struct EFFECT_RECONSTRUCTED_SELECTED_MATERIAL_BINDING',
	'struct EFFECT_RECONSTRUCTED_SELECTED_EMITTER_SELECTION',
	'struct EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_REQUEST',
	'struct EFFECT_RECONSTRUCTED_SELECTED_HANDLER_CONSUMPTION',
	'struct EFFECT_RECONSTRUCTED_SELECTED_PARTICLE_VALUES',
	'struct EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA;',
	'class EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARATION final',
	'class EFFECT_RECONSTRUCTED_SELECTED_PACKET final',
	'class EFFECT_RECONSTRUCTED_SELECTED_FRAME final',
	'class CEffectReconstructedSelectedEvaluator final')
if (-not (Test-SelectedPublicTokensOutsideTestGuard `
	$executionHeader $selectedPublicTypeTokens)) {
	throw 'Selected evaluator API and immutable packet types must remain outside the inspector test macro.'
}
$selectedPublicStartIndex = $executionHeader.IndexOf(
	'enum class EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND')
$selectedPublicEndIndex = $executionHeader.IndexOf(
	'class CEffectReconstructedCpuInspector;', $selectedPublicStartIndex)
if ($selectedPublicStartIndex -lt 0 -or $selectedPublicEndIndex -lt 0) {
	throw 'Selected evaluator public block bounds are missing.'
}
$guardedHeaderScratch = $executionHeader.Insert(
	$selectedPublicEndIndex, "#endif`r`n").Insert(
	$selectedPublicStartIndex,
	"#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)`r`n")
if (Test-SelectedPublicTokensOutsideTestGuard `
	$guardedHeaderScratch $selectedPublicTypeTokens) {
	throw 'Selected evaluator public macro-guard scratch mutation escaped the guard-depth audit.'
}
foreach ($selectedEvaluatorImplementation in @(
	'EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA',
	'SELECTED_MODULE_HANDLER_ROUTES',
	'Find_ModuleHandlerRoute',
	'Resolve_ModuleOpcode',
	'Has_ModuleHandlerRoute',
	'Spawn_TimingParticles',
	'Simulate_FixedSteps',
	'Bind_ModuleDistributions',
	'Evaluate_PreparedDistribution',
	'Selected evaluator handler dispatch is unsupported.',
	'CEffectReconstructedSelectedEvaluator::Prepare',
	'CEffectReconstructedSelectedEvaluator::Evaluate',
	'Selected evaluator requires one shared exact schedule row.',
	'Selected evaluator expected an absent renderer texture row.',
	'Selected evaluator expected an absent sidecar state row.',
	'Selected evaluator visual result failed its closed contract.',
	'Shader_VtxEffectMeshPreview.hlsl',
	'OpaqueBackDepthWrite',
	'Shader_VtxEffectParticle.hlsl',
	'AlphaTwoSidedDepthRead',
	'g_SourceTexture0',
	'g_SourceTexture1')) {
	if ($executionSource -notmatch [regex]::Escape(
		$selectedEvaluatorImplementation)) {
		throw "Selected production evaluator implementation is missing: $selectedEvaluatorImplementation"
	}
}
if ($executionSource -match 'Spawn_InspectionParticles') {
	throw 'The production fixed-step core must not retain an inspector-named execution seam.'
}
$selectedHandlerRouteStartIndex = $executionSource.IndexOf(
	'const SELECTED_MODULE_HANDLER_ROUTE* Find_ModuleHandlerRoute(')
$selectedHandlerResolveIndex = $executionSource.IndexOf(
	'Resolve_ModuleOpcode(', $selectedHandlerRouteStartIndex)
$selectedHandlerRouteEndIndex = $executionSource.IndexOf(
	'bool_t Same4(', $selectedHandlerResolveIndex)
if ($selectedHandlerRouteStartIndex -lt 0 -or
	$selectedHandlerResolveIndex -lt 0 -or
	$selectedHandlerRouteEndIndex -lt 0 -or
	$selectedHandlerRouteStartIndex -ge $selectedHandlerResolveIndex -or
	$selectedHandlerResolveIndex -ge $selectedHandlerRouteEndIndex) {
	throw 'Selected evaluator closed handler-route bounds are missing.'
}
$selectedHandlerLookupBody = $executionSource.Substring(
	$selectedHandlerRouteStartIndex,
	$selectedHandlerResolveIndex - $selectedHandlerRouteStartIndex)
$selectedHandlerRouteBody = $executionSource.Substring(
	$selectedHandlerRouteStartIndex,
	$selectedHandlerRouteEndIndex - $selectedHandlerRouteStartIndex)
foreach ($handlerRouteToken in @(
	'Selection.Handler.strId',
	'Selection.strImplementationId',
	'Selection.iImplementationVersion',
	'Selection.strImplementationSha256',
	'Handler.eKind == Route->eExpectedHandlerKind')) {
	if ($selectedHandlerRouteBody -notmatch [regex]::Escape($handlerRouteToken)) {
		throw "Selected evaluator handler dispatch key is incomplete: $handlerRouteToken"
	}
}
if ($selectedHandlerLookupBody -match 'strExpectedSourceClass|strExactSourceClass') {
	throw 'Selected evaluator handler dispatch must not use source class as lookup authority.'
}
$selectedPrepareIndex = $executionSource.IndexOf(
	'bool_t Client::CEffectReconstructedSelectedEvaluator::Prepare(')
$selectedEvaluateIndex = $executionSource.IndexOf(
	'bool_t Client::CEffectReconstructedSelectedEvaluator::Evaluate(')
$selectedEvaluateEndIndex = $executionSource.IndexOf(
	'#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)',
	$selectedEvaluateIndex)
$inspectorImplementationIndex = $executionSource.IndexOf(
	'bool_t Client::CEffectReconstructedCpuInspector::Simulate(')
if ($selectedPrepareIndex -lt 0 -or $selectedEvaluateIndex -lt 0 -or
	$selectedEvaluateEndIndex -lt 0 -or
	$inspectorImplementationIndex -lt 0 -or
	$selectedPrepareIndex -ge $selectedEvaluateIndex -or
	$selectedEvaluateIndex -ge $selectedEvaluateEndIndex -or
	$selectedEvaluateIndex -ge $inspectorImplementationIndex) {
	throw 'Selected evaluator production methods must precede and remain independent of the inspector wrapper.'
}
$selectedPrepareBody = $executionSource.Substring(
	$selectedPrepareIndex, $selectedEvaluateIndex - $selectedPrepareIndex)
$selectedEvaluateBody = $executionSource.Substring(
	$selectedEvaluateIndex, $selectedEvaluateEndIndex - $selectedEvaluateIndex)
if ($selectedPrepareBody -match 'RendererRuntimeConfig') {
	throw 'Selected evaluator Prepare must not consume rendererRuntimeConfig fallback data.'
}
$selectedDistributionCoreStartIndex = $executionSource.IndexOf(
	'std::array<double, 4u> Select_Range(')
$selectedDistributionEvaluateIndex = $executionSource.IndexOf(
	'std::array<double, 4u> Evaluate_Distribution(',
	$selectedDistributionCoreStartIndex)
$selectedDistributionCoreEndIndex = $executionSource.IndexOf(
	'bool_t Numeric_Close(', $selectedDistributionEvaluateIndex)
if ($selectedDistributionCoreStartIndex -lt 0 -or
	$selectedDistributionEvaluateIndex -lt 0 -or
	$selectedDistributionCoreEndIndex -lt 0 -or
	$selectedDistributionCoreStartIndex -ge $selectedDistributionEvaluateIndex -or
	$selectedDistributionEvaluateIndex -ge $selectedDistributionCoreEndIndex) {
	throw 'Selected evaluator production distribution-core bounds are missing.'
}
$selectedDistributionCoreRegion = $executionSource.Substring(
	$selectedDistributionCoreStartIndex,
	$selectedDistributionCoreEndIndex - $selectedDistributionCoreStartIndex)
$selectedHelperStartIndex = $executionSource.IndexOf(
	'uint32_t Next_Random(uint32_t& iInOutState)')
$selectedVisualHelperStartIndex = $executionSource.IndexOf(
	'bool_t Evaluate_VisualDistribution(', $selectedHelperStartIndex)
$selectedHelperEndIndex = $executionSource.IndexOf(
	'#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)',
	$selectedVisualHelperStartIndex)
if ($selectedHelperStartIndex -lt 0 -or
	$selectedVisualHelperStartIndex -lt 0 -or
	$selectedHelperEndIndex -lt 0 -or
	$selectedHelperStartIndex -ge $selectedVisualHelperStartIndex -or
	$selectedVisualHelperStartIndex -ge $selectedHelperEndIndex) {
	throw 'Selected evaluator reachable production helper bounds are missing.'
}
$selectedProductionHelperRegion = $executionSource.Substring(
	$selectedHelperStartIndex, $selectedHelperEndIndex - $selectedHelperStartIndex)
$selectedVisualHelperRegion = $executionSource.Substring(
	$selectedVisualHelperStartIndex,
	$selectedHelperEndIndex - $selectedVisualHelperStartIndex)
$selectedActionVisualEndIndex = $executionSource.IndexOf(
	'template <typename T>', $selectedVisualHelperStartIndex)
if ($selectedActionVisualEndIndex -lt 0 -or
	$selectedActionVisualEndIndex -le $selectedVisualHelperStartIndex) {
	throw 'Selected evaluator action-time visual helper bounds are missing.'
}
$selectedActionVisualRegion = $executionSource.Substring(
	$selectedVisualHelperStartIndex,
	$selectedActionVisualEndIndex - $selectedVisualHelperStartIndex)
if ($selectedProductionHelperRegion -notmatch
	'(?s)case OPCODE::CYLINDER_Z:.*?Bind\(\{ "startradius", "startheight", "velocityscale",\s*"startlocation" \}\)' -or
	$selectedProductionHelperRegion -notmatch
	'(?s)case OPCODE::GROUND:.*?Bind\(\{ "adjustlocation", "skiplocation" \}\)') {
	throw 'Selected evaluator prepared cylinder/ground distribution slot contracts are missing.'
}
$selectedCylinderEvaluateStart = $selectedVisualHelperRegion.IndexOf(
	'case OPCODE::CYLINDER_Z:')
$selectedCylinderEvaluateEnd = $selectedVisualHelperRegion.IndexOf(
	'case OPCODE::LOCATION:', $selectedCylinderEvaluateStart)
$selectedGroundEvaluateStart = $selectedVisualHelperRegion.IndexOf(
	'case OPCODE::GROUND:', $selectedCylinderEvaluateEnd)
$selectedGroundEvaluateEnd = $selectedVisualHelperRegion.IndexOf(
	'case OPCODE::DYNAMIC_PARAMETER:', $selectedGroundEvaluateStart)
if ($selectedCylinderEvaluateStart -lt 0 -or
	$selectedCylinderEvaluateEnd -lt 0 -or
	$selectedGroundEvaluateStart -lt 0 -or
	$selectedGroundEvaluateEnd -lt 0) {
	throw 'Selected evaluator cylinder/ground action-time blocks are missing.'
}
$selectedCylinderEvaluateBody = $selectedVisualHelperRegion.Substring(
	$selectedCylinderEvaluateStart,
	$selectedCylinderEvaluateEnd - $selectedCylinderEvaluateStart)
$selectedGroundEvaluateBody = $selectedVisualHelperRegion.Substring(
	$selectedGroundEvaluateStart,
	$selectedGroundEvaluateEnd - $selectedGroundEvaluateStart)
if ($selectedCylinderEvaluateBody -notmatch
	'(?s)ModuleOperation, 3u,.*?DrawCount, B, strOutError\).*?ModuleOperation, 2u,.*?DrawCount, A, strOutError\).*?SourcePosition\[Index\] \+= Offset\[Index\] \+ B\[Index\].*?SourceVelocity\[Index\] \+= Offset\[Index\] \* A\[0u\]' -or
	$selectedGroundEvaluateBody -notmatch
	'(?s)ModuleOperation, 0u,.*?DrawCount, A, strOutError\).*?ModuleOperation, 1u,.*?DrawCount, B, strOutError\).*?if \(B\[0u\] < 0\.5\).*?SourcePosition\[Index\] \+= A\[Index\]') {
	throw 'Selected evaluator cylinder/ground distribution consumption order is not the frozen production order.'
}
function Find-SelectedForbiddenToken(
	[string]$Text,
	[string[]]$Tokens) {
	foreach ($token in $Tokens) {
		if ($Text.IndexOf($token, [StringComparison]::Ordinal) -ge 0) {
			return $token
		}
	}
	return ''
}
$forbiddenProductionIoTokens = @(
	'CEffectReconstructedCpuInspector::',
	'Spawn_InspectionParticles',
	'CEffectCatalog',
	'CDataJson',
	'std::ifstream',
	'std::ofstream',
	'std::fstream',
	'std::cin',
	'std::cout',
	'std::cerr',
	'std::filesystem',
	'FILE*',
	'fopen(',
	'_wfopen(',
	'printf(',
	'scanf(',
	'CreateFile',
	'ReadFile',
	'WriteFile',
	'GetFileAttributes',
	'GetModuleFileName',
	'CreateProcess',
	'ShellExecute',
	'GetEnvironmentVariable',
	'SetCurrentDirectory',
	'LoadLibrary')
$selectedProductionReachable = $selectedDistributionCoreRegion +
	$selectedProductionHelperRegion +
	$selectedPrepareBody + $selectedEvaluateBody
$productionIoViolation = Find-SelectedForbiddenToken `
	$selectedProductionReachable $forbiddenProductionIoTokens
if (-not [string]::IsNullOrEmpty($productionIoViolation)) {
	throw "Selected evaluator reachable production path uses forbidden inspector/direct-I-O token: $productionIoViolation"
}
$forbiddenPreparedVisualTokens = @(
	'Find_ModuleDistribution',
	'Get_Distributions',
	'RendererRuntimeConfig',
	'strExactSourceClass',
	'SourceRecipe')
$preparedVisualViolation = Find-SelectedForbiddenToken `
	($selectedActionVisualRegion + $selectedEvaluateBody) `
	$forbiddenPreparedVisualTokens
if (-not [string]::IsNullOrEmpty($preparedVisualViolation)) {
	throw "Selected evaluator action-time visual path uses a forbidden raw lookup/dispatch token: $preparedVisualViolation"
}
$simulateHelperIndex = $selectedProductionHelperRegion.IndexOf(
	'bool_t Simulate_FixedSteps(')
if ($simulateHelperIndex -lt 0) {
	throw 'Selected evaluator fixed-step helper scratch mutation point is missing.'
}
$ioMutationScratch = $selectedProductionHelperRegion.Insert(
	$simulateHelperIndex, "std::ifstream forbiddenActionTimeRead;`r`n")
$ioMutationDetected = Find-SelectedForbiddenToken `
	$ioMutationScratch $forbiddenProductionIoTokens
if ($ioMutationDetected -ne 'std::ifstream') {
	throw 'Selected evaluator helper I-O scratch mutation escaped the reachable-path audit.'
}
$distributionMutationStart = $selectedDistributionCoreRegion.IndexOf(
	'std::array<double, 4u> Evaluate_Distribution(')
$distributionMutationBody = $selectedDistributionCoreRegion.IndexOf(
	'{', $distributionMutationStart)
if ($distributionMutationStart -lt 0 -or $distributionMutationBody -lt 0) {
	throw 'Selected evaluator distribution-core scratch mutation point is missing.'
}
$distributionIoMutationScratch = $selectedDistributionCoreRegion.Insert(
	$distributionMutationBody + 1,
	"`r`nstd::ifstream forbiddenDistributionRead;")
$distributionIoMutationDetected = Find-SelectedForbiddenToken `
	$distributionIoMutationScratch $forbiddenProductionIoTokens
if ($distributionIoMutationDetected -ne 'std::ifstream') {
	throw 'Selected evaluator distribution I-O scratch mutation escaped the reachable-path audit.'
}
$visualMutationStart = $selectedVisualHelperRegion.IndexOf(
	'bool_t Evaluate_SelectedVisual(')
$visualMutationBody = $selectedVisualHelperRegion.IndexOf(
	'{', $visualMutationStart)
if ($visualMutationStart -lt 0 -or $visualMutationBody -lt 0) {
	throw 'Selected evaluator visual scratch mutation point is missing.'
}
$rawVisualMutationScratch = $selectedVisualHelperRegion.Insert(
	$visualMutationBody + 1, "`r`nGet_Distributions();")
$rawVisualMutationDetected = Find-SelectedForbiddenToken `
	$rawVisualMutationScratch $forbiddenPreparedVisualTokens
if ($rawVisualMutationDetected -ne 'Get_Distributions') {
	throw 'Selected evaluator visual raw-lookup scratch mutation escaped the action-time audit.'
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
if ($catalogSource -match '(?s)(std::ifstream|ReadAllText|Read_Text)\s*\([^\)]*reconstructed-runtime-program\.candidate\.json' -or
	$catalogSource -match 'Replace\([^\)]*\\r\\n[^\)]*\\n') {
	throw 'Production Catalog must not read or normalize external reconstructed candidate bytes at runtime.'
}
foreach ($actualCatalogBoundary in @(
	'PublishedCatalogPath',
	'PublishedCatalogStructure',
	'PublishedCatalogText.size() == 27''144''447u',
	'e8412973025b01510e3a81ac7c119ac58994ab126aa8cf624afa8c126d342f8d',
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
	'CatalogText.size() == 27''144''447u',
	'e8412973025b01510e3a81ac7c119ac58994ab126aa8cf624afa8c126d342f8d',
	'Identity->iRenderResourceSidecarByteCount == 774''127u',
	'Authority->TextureResourcesById.size() == 52u',
	'Authority->TextureBindingsById.size() == 77u')) {
	if ($harness -notmatch [regex]::Escape($renderResourceCatalogBoundary)) {
		throw "Current Catalog render-resource harness identity is missing: $renderResourceCatalogBoundary"
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
	'Artist 31470 nonProduct Debug M0 Production Prepare Binds One Immutable Catalog Plan Sidecar And 24 Exact Handlers',
	'Artist 31470 nonProduct Debug M0 Production Evaluate Emits Exact Step88 Mesh Sprite 2/2 And Consumes 15 Plus 9 Handlers',
	'Artist 31470 nonProduct Debug M0 Production Packet Freezes Mesh14 Sprite7 Values Signed Size And Sink Ready Orientation',
	'Artist 31470 nonProduct Debug M0 Production Packet Projection Is Deterministic',
	'Artist 31470 nonProduct Debug M0 Prepare Rejects RNG Schedule Emitter Renderer Handler Default Empty And Nonfinite Mutations Transactionally',
	'Artist 31470 nonProduct Debug M0 Prepare Rejects Mesh Raster Depth And Sprite Blend Raster Role Swaps Transactionally',
	'Artist 31470 nonProduct Debug M0 Prepare Rejects Recognized Class Nonallowlisted Handler Triplet Transactionally',
	'Artist 31470 nonProduct Debug M0 Evaluate Rejects Step Serial And Empty Preparation While Preserving Prior Frame',
	'Artist 31470 nonProduct Debug M0 Evaluate Rejects Internal RNG Draw Occurrence And Lifetime Contract Mismatch While Preserving Prior Frame',
	'Artist 31470 nonProduct Debug M0 Evaluate Rejects Nonzero Empty Alpha Default Without Promoting It To Identity',
	'Artist 31470 nonProduct Debug M0 Prepare And Evaluate Ignore Cleared Or Poisoned RendererRuntimeConfig Fallback',
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
				throw "Current render-resource Effect runtime Catalog is missing: $resolvedRenderResourceCatalog"
			}
			$renderResourceOutput = & $resolvedHarness `
				'--effect-reconstructed-render-resource-authority' `
				$resolvedRenderResourceCatalog $resolvedCatalog 2>&1
			if ($LASTEXITCODE -ne 0 -or
				($renderResourceOutput -join "`n") -notmatch 'failures : 0') {
				throw "Current render-resource Catalog harness failed:`n$($renderResourceOutput -join "`n")"
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
