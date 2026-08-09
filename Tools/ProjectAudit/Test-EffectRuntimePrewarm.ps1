param()

$ErrorActionPreference = 'Stop'
$root = Resolve-Path (Join-Path $PSScriptRoot '..\..')

function Read-Source([string]$relativePath) {
    $path = Join-Path $root $relativePath
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Missing source file: $relativePath"
    }
    return [IO.File]::ReadAllText($path)
}

function Require-Match(
    [string]$text,
    [string]$pattern,
    [string]$message) {
    if ($text -notmatch $pattern) {
        throw $message
    }
}

function Require-NoMatch(
    [string]$text,
    [string]$pattern,
    [string]$message) {
    if ($text -match $pattern) {
        throw $message
    }
}

function Function-Body(
    [string]$text,
    [string]$signaturePattern,
    [string]$nextSignaturePattern) {
    $match = [regex]::Match(
        $text,
        "(?s)$signaturePattern(?<body>.*?)$nextSignaturePattern")
    if (-not $match.Success) {
        throw "Could not isolate function body: $signaturePattern"
    }
    return $match.Groups['body'].Value
}

$rendererHeader = Read-Source 'Client\Public\Effect_DocumentRenderer.h'
$rendererSource = Read-Source 'Client\Private\Effect_DocumentRenderer.cpp'
$objectHeader = Read-Source 'Client\Public\Effect_Object.h'
$objectSource = Read-Source 'Client\Private\Effect_Object.cpp'
$presentationHeader = Read-Source 'Client\Public\Effect_PresentationService.h'
$presentationSource = Read-Source 'Client\Private\Effect_PresentationService.cpp'
$playbackHeader = Read-Source 'Client\Public\Effect_Playback.h'
$playbackSource = Read-Source 'Client\Private\Effect_Playback.cpp'
$characterSource = Read-Source 'Client\Private\Character.cpp'
$mainAppSource = Read-Source 'Client\Private\MainApp.cpp'
$catalogHeader = Read-Source 'Client\Public\Effect_Catalog.h'
$catalogSource = Read-Source 'Client\Private\Effect_Catalog.cpp'
$toolSource = Read-Source 'Client\Private\Effect_Tool.cpp'

foreach ($counter in @(
    'iCoreBuildCount',
    'iCatalogCommitCount',
    'iPreparedDocumentBuildCount',
    'iModelDiskLoadCount',
    'iTextureDiskLoadCount',
	'iVectorFieldDiskLoadCount',
    'iPreparedAttachCount',
    'iSynchronousDocumentStageCount',
    'iPreparedLookupMissCount')) {
    Require-Match $rendererHeader ([regex]::Escape($counter)) `
        "Effect prewarm probe is missing $counter."
}

Require-Match $rendererSource `
    'Build_RendererCore[\s\S]*?Acquire_RendererCore' `
    'Effect renderer core must have an explicit one-time build/acquire boundary.'
Require-Match $rendererSource `
    'g_EffectRendererCores\.find\(pDevice\.Get\(\)\)[\s\S]*?Build_RendererCore' `
    'Effect renderer core must be cached by the D3D device.'

$initializeBody = Function-Body $rendererSource `
    'HRESULT Client::CEffectDocumentRenderer::Initialize\(\)\s*\{' `
    'HRESULT Client::CEffectDocumentRenderer::Load_Texture'
Require-Match $initializeBody 'Acquire_RendererCore' `
    'EffectObject initialization must attach the cached renderer core.'
Require-NoMatch $initializeBody `
    'CShader::Create|CModel::Create|CreateDDSTextureFromFile' `
    'EffectObject initialization must not compile shaders or load document files.'
Require-NoMatch $initializeBody `
    'CVIBuffer_ParticleRect::Create|CVIBuffer_DynamicTrail::Create' `
    'Standalone product spawn must not allocate unused particle/trail buffers.'

$preparedBody = Function-Body $rendererSource `
    'bool_t Client::CEffectDocumentRenderer::Stage_Prepared\(' `
    'bool_t Client::CEffectDocumentRenderer::Stage_Document\('
Require-Match $preparedBody `
    'pCatalogDocumentIdentity\s*==\s*&Document' `
    'Product prepared attachment must use the exact staged catalog document identity.'
Require-NoMatch $preparedBody `
    'CShader::Create|CModel::Create|CreateDDSTextureFromFile|is_regular_file|ifstream' `
    'Prepared attachment must not compile or perform model/texture/file I/O.'

Require-Match $rendererSource `
    'PREPARED_KEY[\s\S]*?iCatalogRevision[\s\S]*?strEffectAssetId[\s\S]*?iResourceSignature' `
    'Prepared resources must be keyed by revision, EffectAssetId, and resource signature.'
Require-Match $rendererSource `
    'NonAnimatedModels[\s\S]*?AnimatedModelPrototypes[\s\S]*?Textures' `
    'Prewarm must deduplicate immutable nonanimated models and SRVs while separating animated prototypes.'
Require-Match $rendererSource `
    'Prototype\.pModel->Clone\(nullptr\)' `
    'Mutable animated Model Cues must clone their prepared prototype per instance.'
Require-Match $rendererSource `
    'Ensure_MutableInstanceBuffers[\s\S]*?EFFECT_ELEMENT_KIND::PARTICLE[\s\S]*?EFFECT_ELEMENT_KIND::TRAIL' `
    'Mutable particle/trail buffer requirements must be derived from the prepared Document.'
Require-Match $rendererSource `
    'CVIBuffer_ParticleRect::Create[\s\S]*?CVIBuffer_DynamicTrail::Create' `
    'Mutable particle/trail buffers must be created on the Effect instance, outside the shared core.'
Require-Match $rendererSource `
    'g_PreparedEffectDocuments\s*=\s*std::move\(Staged\)' `
    'Prepared cache publication must occur only after a complete staged map succeeds.'
Require-Match $rendererHeader `
    'std::shared_ptr<const PREPARED_DOCUMENT> m_pPreparedDocument' `
    'Active Effect instances must retain their prepared bundle across cache revision replacement.'

$spawnBody = Function-Body $presentationSource `
    'bool_t Client::CEffectPresentationService::Spawn\(' `
    'void Client::CEffectPresentationService::Update\('
Require-Match $spawnBody `
    'CEffectDocumentRenderer::Find_Prepared' `
    'Product Spawn must acquire a prepared bundle before cloning the EffectObject.'
Require-Match $spawnBody `
    'bRequirePreparedResources\s*=\s*true' `
    'Product Spawn must fail closed instead of falling back to synchronous staging.'
Require-Match $spawnBody `
    'bAutoPlay\s*=\s*false' `
    'Product Effect playback must be service-owned after Character update.'
Require-NoMatch $spawnBody `
    'Stage_Document|CShader::Create|CModel::Create|CreateDDSTextureFromFile|ifstream|is_regular_file' `
    'Product Spawn contains an action-time resource stage or file-I/O path.'
Require-Match $spawnBody `
    'ProbeBefore[\s\S]*?ProbeAfter[\s\S]*?iCoreBuildCount[\s\S]*?iModelDiskLoadCount[\s\S]*?iTextureDiskLoadCount[\s\S]*?iVectorFieldDiskLoadCount[\s\S]*?iSynchronousDocumentStageCount[\s\S]*?iPreparedAttachCount' `
    'Product Spawn must enforce unchanged compile/model/DDS/vector-field/stage counters and exactly one prepared attachment.'

Require-Match $presentationSource `
    'Prepare_ProductCues[\s\S]*?for \(const ANIMATION_EFFECT_CUE& Cue : Cues\)[\s\S]*?Cue\.strEffectAssetId' `
    'Prewarm targets must come from actual admitted animation asset cues.'
Require-NoMatch (Function-Body $presentationSource `
    'bool_t Client::CEffectPresentationService::Prepare_ProductCues\(' `
    'bool_t Client::CEffectPresentationService::Reprepare_ProductTargets\(') `
    'Get_EffectAssetIds' `
    'Cue readiness must not prewarm the whole diagnostic/source catalog.'
Require-Match $characterSource `
    'Prepare_ProductCues\([\s\S]*?staged\.Cues[\s\S]*?m_EffectCueDocument\s*=\s*std::move\(staged\)' `
    'Character cue targets must finish prewarm before the cue document becomes input-ready.'

Require-Match $playbackHeader 'Stage_PrevalidatedDocument' `
    'Prepared product playback needs a no-I/O state-clone entry point.'
Require-Match $playbackHeader 'Get_VectorFieldDiskLoadCount' `
    'Prepared playback must expose vector-field file-I/O instrumentation.'
Require-Match $rendererSource `
    'PREPARED_DOCUMENT[\s\S]*?pPlaybackResources[\s\S]*?Prepare_DocumentResources' `
    'Each revision-scoped prepared Document must own its immutable playback resources.'
Require-Match $objectSource `
    'Stage_PreparedDocument[\s\S]*?Stage_PrevalidatedDocument' `
    'Prepared EffectObject spawn must use the prevalidated playback path.'
$preparedObjectBody = Function-Body $objectSource `
    'bool_t Client::CEffectObject::Stage_PreparedDocument\(' `
    'void Client::CEffectObject::Set_RootWorld\('
Require-NoMatch $preparedObjectBody `
    'm_Playback\.Seek' `
    'Prepared product staging must defer its first Seek until post-Character anchor sync.'
Require-Match $playbackSource `
    'void Client::CEffectPlayback::Seek\([\s\S]*?Reset\(\);[\s\S]*?ActionRootWorld\s*=\s*RootWorld[\s\S]*?bActionRootCaptured\s*=\s*true' `
    'Pending Seek must capture zero-time snapshot attachments at the synchronized root.'
Require-Match $playbackSource `
    'Prepare_DocumentResources\([\s\S]*?Load_VectorFieldFromDisk' `
    'Vector fields must be loaded during readiness staging.'
Require-Match $playbackSource `
    'Apply_SourceUpdateModules\([\s\S]*?m_pPreparedResources->[\s\S]*?VectorFields\.find' `
    'Action-time particle updates must use only the instance retained prepared vector-field map.'
Require-NoMatch $playbackSource `
    'g_VectorFields' `
    'Vector fields must not use a process-global assetId-only cache across catalog revisions.'
Require-Match $playbackSource `
    'OutPrepared\s*=\s*std::move\(Staged\)' `
    'A vector-field resource bundle must publish only after every staged load succeeds.'

Require-Match $characterSource `
    'Resolve_CueWallOffset[\s\S]*?fCurrentStageWallSeconds\s*-\s*fOccurrenceWallSeconds[\s\S]*?fPlayRate[\s\S]*?fInitialSampleTimeSeconds\s*=\s*fInitialSampleSeconds' `
    'Late multi-clip cue crossing must seek to its authored absolute-timeline overshoot.'
Require-Match $characterSource `
    'strOccurrenceId\s*=\s*"stage:"[\s\S]*?"/clip:"[\s\S]*?"/cue:"[\s\S]*?"/loop:"' `
    'Runtime cue occurrence identity must include stage, clip, cue, and loop epoch.'
Require-Match $presentationSource `
    'Effect\.strOccurrenceId\s*==\s*Desc\.strOccurrenceId' `
    'Duplicate suppression must use the runtime occurrence identity.'
Require-Match $presentationSource `
    'fElapsedCueTimeSeconds\s*=\s*Desc\.fInitialSampleTimeSeconds' `
    'Cue-end clock must begin at the same initial playback sample.'
Require-Match $presentationSource `
    'bPendingInitialSeek[\s\S]*?Set_SampleTime\([\s\S]*?fPendingInitialSampleTimeSeconds[\s\S]*?else if \(nullptr != Effect\.pObject[\s\S]*?Advance_Preview\(fCueDelta\)' `
    'A newly crossed cue must apply one pending post-anchor Seek without adding the frame delta twice.'
Require-Match $presentationSource `
    'EFFECT_FOLLOW_POLICY::FOLLOW != Effect\.eFollowPolicy &&\s*!Effect\.bPendingInitialSeek' `
    'Snapshot cues must receive their final post-Character root before the pending initial Seek.'
Require-Match $presentationSource `
    'fCueDelta\s*=[\s\S]*?fTimeDelta\)\s*\*\s*Effect\.fPlaybackRate[\s\S]*?Advance_Preview\(fCueDelta\)' `
    'Service playback and cue-end clocks must advance on the same authored clip-rate delta.'
Require-Match $mainAppSource `
    'Update_Engine\(fTimeDelta\);\s*CEffectPresentationService::Synchronize_FollowAnchors\(\);\s*CEffectPresentationService::Update\(fTimeDelta\);' `
    'Current Character root/bones must be synchronized before service-owned Effect playback captures occurrences.'

Require-Match $catalogHeader 'RUNTIME_SNAPSHOT' `
    'Effect Catalog must expose an explicit rollback snapshot boundary.'
Require-Match $catalogSource `
    'Capture_Runtime\(\)[\s\S]*?Restore_Runtime\(' `
    'Effect Catalog snapshot/restore implementation is missing.'
Require-Match $toolSource `
    'Capture_Runtime\(\)[\s\S]*?CEffectCatalog::Load\(CatalogStatus\)[\s\S]*?Reprepare_ProductTargets[\s\S]*?Restore_Runtime' `
    'F1 Product reload must roll the catalog back when prepared resource staging fails.'

Write-Host (
    'Effect runtime prewarm audit PASS: cue-target-only prewarm, one core/device, ' +
    'shared immutable bundles, per-instance mutable state, no-I/O product Spawn, ' +
    'catalog rollback, and post-Character authored cue timing are wired.')
