[CmdletBinding()]
param(
    [string]$BundleRoot = '',
    [string]$ResourceRoot = '',
    [string]$SourceRoot = ''
)

$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrWhiteSpace($BundleRoot)) {
    $BundleRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    $BundleRoot = [IO.Path]::GetFullPath($BundleRoot)
}

$repoRoot = if ([IO.Directory]::Exists((Join-Path $BundleRoot 'Client')) -and
    [IO.Directory]::Exists((Join-Path $BundleRoot 'Data'))) {
    $BundleRoot
}
else {
    [IO.Path]::GetFullPath((Join-Path $BundleRoot '..\..\..\..'))
}
if ([string]::IsNullOrWhiteSpace($ResourceRoot)) {
    $ResourceRoot = Join-Path $repoRoot 'Client\Bin\Resources'
}
$ResourceRoot = [IO.Path]::GetFullPath($ResourceRoot)
if (-not [string]::IsNullOrWhiteSpace($SourceRoot)) {
    $SourceRoot = [IO.Path]::GetFullPath($SourceRoot)
}

function Require-File([string]$relativePath) {
    $path = Join-Path $BundleRoot $relativePath
    if (-not [IO.File]::Exists($path)) {
        throw "Missing final bundle file: $relativePath"
    }
    return $path
}

function Read-JsonFile([string]$relativePath) {
    $path = Require-File $relativePath
    try {
        return Get-Content -LiteralPath $path -Raw -Encoding UTF8 |
            ConvertFrom-Json
    }
    catch {
        throw "Invalid JSON in ${relativePath}: $($_.Exception.Message)"
    }
}

function Get-NormalizedTextHash([string]$path) {
    $text = [IO.File]::ReadAllText($path, [Text.UTF8Encoding]::new($false))
    $normalized = $text.Replace("`r`n", "`n").Replace("`r", "`n")
    $bytes = [Text.UTF8Encoding]::new($false).GetBytes($normalized)
    $sha = [Security.Cryptography.SHA256]::Create()
    try {
        return [BitConverter]::ToString($sha.ComputeHash($bytes)).Replace('-', '')
    }
    finally {
        $sha.Dispose()
    }
}

$requiredCode = @(
    'Engine\Public\VIBuffer_ParticleRect.h',
    'Engine\Private\VIBuffer_ParticleRect.cpp',
    'Engine\Public\VIBuffer_DynamicTrail.h',
    'Engine\Private\VIBuffer_DynamicTrail.cpp',
    'Client\Public\Effect_AuthoringDocument.h',
    'Client\Public\Effect_MaterialTemplate.h',
    'Client\Public\EffectAuthoringTransfer.h',
    'Client\Public\Effect_DocumentCodec.h',
    'Client\Private\Effect_DocumentCodec.cpp',
    'Client\Public\Effect_Playback.h',
    'Client\Private\Effect_Playback.cpp',
    'Client\Public\Effect_DocumentRenderer.h',
    'Client\Private\Effect_DocumentRenderer.cpp',
    'Client\Public\Effect_Object.h',
    'Client\Private\Effect_Object.cpp',
    'Client\Public\Effect_ThumbnailCache.h',
    'Client\Private\Effect_ThumbnailCache.cpp',
    'Client\Public\Effect_Tool.h',
    'Client\Private\Effect_Tool.cpp',
    'Client\Public\Effect_Catalog.h',
    'Client\Private\Effect_Catalog.cpp',
    'Client\Public\AnimationEffectCueDocument.h',
    'Client\Private\AnimationEffectCueDocument.cpp',
    'Client\Public\Effect_PresentationService.h',
    'Client\Private\Effect_PresentationService.cpp',
    'Client\Public\Animation_Tool.h',
    'Client\Private\Animation_Tool.cpp',
    'Client\Public\AnimationPreviewAssets.h',
    'Client\Public\AnimationTargetService.h',
    'Client\Private\AnimationTargetService.cpp',
    'Client\Public\CharacterPreviewPanel.h',
    'Client\Private\CharacterPreviewPanel.cpp',
    'Client\Public\Character.h',
    'Client\Private\Character.cpp',
    'Client\Public\PlayerSkillCatalog.h',
    'Client\Private\PlayerSkillCatalog.cpp',
    'Client\Public\MainApp.h',
    'Client\Private\MainApp.cpp',
    'Client\Public\Loader.h',
    'Client\Private\Loader.cpp',
    'Client\Bin\ShaderFiles\Shader_EffectCommon.hlsli',
    'Client\Bin\ShaderFiles\Shader_VtxEffectMeshPreview.hlsl',
    'Client\Bin\ShaderFiles\Shader_VtxEffectRectPreview.hlsl',
    'Client\Bin\ShaderFiles\Shader_VtxEffectParticle.hlsl',
    'Client\Bin\ShaderFiles\Shader_VtxEffectTrail.hlsl',
    'Client\Bin\ShaderFiles\Shader_VtxEffectDecal.hlsl',
    'Tools\EffectPipeline\Publish-Effects.ps1',
    'Tools\EffectPipeline\Test-EffectPipeline.ps1',
    'Tools\ClientFrontendHarness\Default\ClientFrontendHarness.vcxproj',
    'Tools\ClientFrontendHarness\Private\ClientFrontendHarness.cpp'
)
foreach ($relativePath in $requiredCode) {
    $bundlePath = Require-File $relativePath
    if (-not [string]::IsNullOrWhiteSpace($SourceRoot)) {
        $sourcePath = Join-Path $SourceRoot $relativePath
        if (-not [IO.File]::Exists($sourcePath)) {
            throw "Missing source file for final-code mirror: $relativePath"
        }
        # Git checkout와 Markdown 박제본의 CRLF/LF 차이는 코드 차이가 아니다.
        # UTF-8 text를 LF로 정규화한 뒤 SHA-256을 비교해 내용 동일성만 검사한다.
        $bundleHash = Get-NormalizedTextHash $bundlePath
        $sourceHash = Get-NormalizedTextHash $sourcePath
        if ($bundleHash -cne $sourceHash) {
            throw "Final-code mirror hash mismatch: $relativePath"
        }
    }
}

$loaderSource = Get-Content -LiteralPath (
    Require-File 'Client\Private\Loader.cpp') -Raw -Encoding UTF8
foreach ($prototypeTag in @(
    'Prototype_Component_Model_LanceMaster',
    'Prototype_Component_Model_GunSlinger',
    'Prototype_Component_Model_Slayer',
    'Prototype_Component_Model_Artist',
    'Prototype_Component_Model_DimensionMaster')) {
    if ($loaderSource -notmatch [regex]::Escape($prototypeTag)) {
        throw "Animation preview loader is missing playable prototype skip: $prototypeTag"
    }
}
if ($loaderSource -notmatch 'playablePrototypeTags' -or
    $loaderSource -notmatch 'CPlayableCharacterAssetService::Ensure_Prototypes' -or
    $loaderSource -notmatch 'if \(bPlayablePrototype\)') {
    throw 'Animation preview loader must reuse playable services and directly load preview-only models.'
}

$characterSource = Get-Content -LiteralPath (
    Require-File 'Client\Private\Character.cpp') -Raw -Encoding UTF8
if ($characterSource -notmatch 'cue\.strAnchorSlotId' -or
    $characterSource -notmatch 'm_pBodyModel->Has_Bone\(' -or
    $characterSource.IndexOf('m_pBodyModel->Has_Bone(') -gt
        $characterSource.IndexOf('m_EffectCueDocument = std::move(staged)')) {
    throw 'Character must validate every staged Effect cue anchor before commit.'
}

foreach ($relativePath in @(
    'Client\Private\Effect_Playback.cpp',
    'Client\Private\Effect_Tool.cpp')) {
    $durationSource = Get-Content -LiteralPath (
        Require-File $relativePath) -Raw -Encoding UTF8
    if ($durationSource -notmatch 'Particle\.vLifeTimeSeconds\.y' -or
        $durationSource -notmatch 'Trail\.fPointLifeTimeSeconds') {
        throw "Effect preview duration must include Particle and Trail tails: $relativePath"
    }
}

$commonShader = Get-Content -LiteralPath (
    Require-File 'Client\Bin\ShaderFiles\Shader_EffectCommon.hlsli') `
    -Raw -Encoding UTF8
if ($commonShader -notmatch 'SrcBlend\[1\] = One' -or
    $commonShader -notmatch 'DestBlend\[1\] = One' -or
    $commonShader -notmatch 'float4\(distortion, 0\.f, 0\.f\)') {
    throw 'Effect distortion target must use signed RG additive independent blending.'
}

$mainAppSource = Get-Content -LiteralPath (
    Require-File 'Client\Private\MainApp.cpp') -Raw -Encoding UTF8
$effectToolHeader = Get-Content -LiteralPath (
    Require-File 'Client\Public\Effect_Tool.h') -Raw -Encoding UTF8
$animationToolHeader = Get-Content -LiteralPath (
    Require-File 'Client\Public\Animation_Tool.h') -Raw -Encoding UTF8
$animationToolSource = Get-Content -LiteralPath (
    Require-File 'Client\Private\Animation_Tool.cpp') -Raw -Encoding UTF8
$animationPreviewAssets = Get-Content -LiteralPath (
    Require-File 'Client\Public\AnimationPreviewAssets.h') -Raw -Encoding UTF8
if ($mainAppSource -notmatch 'make_shared<CCharacterPreviewPanel>' -or
    $effectToolHeader -notmatch 'shared_ptr<CCharacterPreviewPanel>' -or
    $animationToolHeader -notmatch 'shared_ptr<CCharacterPreviewPanel>') {
    throw 'Effect and Animation tools must share one MainApp-owned preview session.'
}
if ($effectToolHeader -notmatch 'Reset_DetailDraft\(\)' -or
    (Get-Content -LiteralPath (
        Require-File 'Client\Private\Effect_Tool.cpp') -Raw -Encoding UTF8) `
        -notmatch 'm_ActiveDocument = std::move\(Staged\);[\s\S]{0,300}?Reset_DetailDraft\(\);') {
    throw 'Active Effect document replacement must invalidate the local Detail draft.'
}
$effectToolSource = Get-Content -LiteralPath (
    Require-File 'Client\Private\Effect_Tool.cpp') -Raw -Encoding UTF8
if ($effectToolSource -notmatch 'Resolve_CurrentTargetSpec' -or
    $effectToolSource -notmatch 'Confirm Clear All Elements' -or
    $effectToolSource -notmatch 'World preview hidden:') {
    throw 'Effect target spec, destructive clear confirmation, and anchor failure status are required.'
}
if ($effectToolSource -match 'ImVec2\(0\.f, 112\.f\)' -or
    $effectToolSource -notmatch 'SetNextWindowSizeConstraints' -or
    $effectToolSource -notmatch 'AllEffectsTree[\s\S]{0,160}?-fStatusReserve') {
    throw 'All Effects must use a resizable, remaining-height tree instead of the fixed 112 px child.'
}
if ($effectToolSource -match '"CreateEffect"' -or
    $effectToolSource -notmatch 'Load existing data in All Effects or Data Files' -or
    $effectToolSource -notmatch 'Start_WorldPreviewFromBeginning\(\)' -or
    $effectToolSource -notmatch 'm_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;[\s\S]{0,900}?Start_WorldPreviewFromBeginning\(\);') {
    throw 'Loading an existing Effect must be unambiguous and automatically restart complete world playback.'
}
if ($mainAppSource -notmatch 'FPS: %\.1f\s+\|\s+Frame: %\.2f ms' -or
    $mainAppSource -notmatch 'io\.Framerate' -or
    $mainAppSource -notmatch 'io\.DeltaTime \* 1000\.f') {
    throw 'F1 Developer Tools must always show smoothed FPS and latest frame time without enabling the profiler.'
}
if ($effectToolHeader -notmatch 'Try_SelectSkill' -or
    $effectToolSource -notmatch 'ImGuiTreeNodeFlags_Selected' -or
    $effectToolSource -notmatch 'Try_SelectSkill\(Entry\.Skill\.strEffectId\)' -or
    $effectToolSource -notmatch 'CEffect_Tool::Try_SelectSkill[\s\S]{0,1200}?EFFECT_PREVIEW_FILTER::COMPLETE[\s\S]{0,1200}?Start_WorldPreviewFromBeginning\(\)') {
    throw 'All Effects skill rows must visibly select and load or restart the complete authored Effect.'
}
if ($effectToolHeader -notmatch 'Stage_DetailDraftPreview' -or
    $effectToolSource -notmatch 'Drag numeric values for live world preview' -or
    $effectToolSource -notmatch 'DragFloat3\("Velocity"' -or
    $effectToolSource -notmatch 'CEffect_Tool::Stage_DetailDraftPreview[\s\S]{0,1800}?Stage_WorldPreview\(Staged\)' -or
    $effectToolSource -notmatch 'if \(bChanged\)[\s\S]{0,180}?Stage_DetailDraftPreview\(\)' -or
    $effectToolSource -notmatch 'Detail draft reverted to the active Document preview') {
    throw 'Effect Detail numeric drags must live-stage a draft while Apply/Revert retain the active Document boundary.'
}
if ($effectToolHeader -notmatch 'IMPORTED_REFERENCE' -or
    $effectToolSource -notmatch '\.imported-effect-draft\.json' -or
    $effectToolSource -notmatch '\.unbound-effect-draft-index\.json' -or
    $effectToolSource -notmatch 'Imported Draft rows are extraction reference only' -or
    $effectToolSource -notmatch 'matching Authored row for playback') {
    throw 'Extraction drafts must be visible but explicitly non-loadable until converted to an Effect Document.'
}
$previewPanelHeader = Get-Content -LiteralPath (
    Require-File 'Client\Public\CharacterPreviewPanel.h') -Raw -Encoding UTF8
$previewPanelSource = Get-Content -LiteralPath (
    Require-File 'Client\Private\CharacterPreviewPanel.cpp') -Raw -Encoding UTF8
$animationTargetSource = Get-Content -LiteralPath (
    Require-File 'Client\Private\AnimationTargetService.cpp') -Raw -Encoding UTF8
$effectObjectSource = Get-Content -LiteralPath (
    Require-File 'Client\Private\Effect_Object.cpp') -Raw -Encoding UTF8
if ($previewPanelHeader -notmatch 'CHARACTER_PREVIEW_LOCK_OWNER' -or
    $previewPanelHeader -notmatch 'm_SessionLocks' -or
    $effectToolSource -notmatch 'CHARACTER_PREVIEW_LOCK_OWNER::EFFECT_TOOL' -or
    $animationToolSource -notmatch 'CHARACTER_PREVIEW_LOCK_OWNER::ANIMATION_TOOL' -or
    $effectToolSource -notmatch 'CEffect_Tool::Update[\s\S]*?Set_SessionLock\([\s\S]*?CHARACTER_PREVIEW_LOCK_OWNER::EFFECT_TOOL' -or
    $animationToolSource -notmatch 'CAnimation_Tool::Render\(\)[\s\S]*?Set_SessionLock\([\s\S]*?CHARACTER_PREVIEW_LOCK_OWNER::ANIMATION_TOOL[\s\S]*?if \(!ImGui::Begin') {
    throw 'Animation and Effect dirty state must contribute independent shared-preview locks.'
}
if ($animationTargetSource -notmatch 'Clear_Preview\(\)' -or
    $previewPanelSource -notmatch 'CAnimationTargetService::Clear_Preview\(\)') {
    throw 'Expired preview targets must clear their identity and advance target generation.'
}
if ($effectToolHeader -notmatch 'EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT' -or
    $effectToolSource -notmatch 'Synchronize_LoadedSkillPreview' -or
    $effectToolSource -notmatch 'CAnimationSkillBindingDocument::Load' -or
    $effectToolSource -notmatch 'Skill animation synced:' -or
    $previewPanelHeader -notmatch 'Select_TargetAsset' -or
    $previewPanelSource -notmatch 'Select_TargetAsset') {
    throw 'Loaded skill Effects must target the playable class root and replace a retained prop/T-animation preview.'
}
if ($animationPreviewAssets -notmatch 'bPlayableClassBody' -or
    $effectToolSource -notmatch 'Render_Selector\(false, \{\}, false\)' -or
    $previewPanelSource -notmatch '!includePreviewOnlyTargets && !asset\.bPlayableClassBody' -or
    $animationToolSource -notmatch 'Render_Selector\(false, \{\}\)') {
    throw 'Effect Tool must list playable class bodies only while Animation Tool retains auxiliary preview targets.'
}
$presentationSource = Get-Content -LiteralPath (
    Require-File 'Client\Private\Effect_PresentationService.cpp') -Raw -Encoding UTF8
if ($mainAppSource -notmatch 'Update_Engine\(fTimeDelta\);\s+CEffectPresentationService::Synchronize_FollowAnchors\(\);' -or
    $presentationSource -notmatch 'Synchronize_FollowAnchors\(\)' -or
    $presentationSource -notmatch 'bFollowAnchorMissing' -or
    $presentationSource -notmatch 'Set_Visible\(false\)') {
    throw 'Runtime FOLLOW anchors must resample after Character animation update and defer unsafe removal.'
}
if ($effectToolSource -notmatch 'Discard unsaved Effect changes\?' -or
    $effectToolSource -notmatch 'if \(Has_UnsavedWork\(\)\)' -or
    $effectToolSource -notmatch 'Has_UnappliedDetailDraft' -or
    $effectToolSource -notmatch 'Apply or Revert the open Detail draft before saving') {
    throw 'Effect Create/Load/Discard/Save must preserve unsaved Document and Detail work.'
}
if ($effectToolSource -notmatch 'Loaded editable draft; preview is hidden' -or
    $effectToolSource -notmatch 'Load rejected; active Document and preview were preserved' -or
    $effectToolSource -notmatch 'open Detail draft before selecting another Element' -or
    $effectToolSource -notmatch 'open Detail draft before adding an Element' -or
    $effectToolSource -notmatch 'open Detail draft before deleting an Element' -or
    $effectToolSource -notmatch 'open Detail draft before clearing Elements') {
    throw 'Valid partial drafts must reload, and dirty Detail drafts must survive selection/structure commands.'
}
$codecSource = Get-Content -LiteralPath (
    Require-File 'Client\Private\Effect_DocumentCodec.cpp') -Raw -Encoding UTF8
if ($codecSource -notmatch 'Authoring save preserves valid partial drafts' -or
    $codecSource -notmatch 'Save_Atomic[\s\S]*?if \(!Validate\(Document, strOutError\)\)') {
    throw 'Effect authoring save must preserve valid non-drawable drafts; publisher keeps the drawable gate.'
}
if ($effectObjectSource -notmatch 'Advance_Preview' -or
    $effectToolSource -notmatch 'fSequentialAdvance' -or
    $effectToolSource -notmatch 'pObject->Advance_Preview\(fSequentialAdvance\)') {
    throw 'Playing Effect previews must advance sequentially; Seek is reserved for scrub and loop jumps.'
}
if ($effectToolSource -notmatch 'm_bPreviewPlaying = false;\s+bSeekAfterLoop = true;' -or
    $effectToolSource -notmatch 'Refresh_AllEffects' -or
    $effectToolSource -notmatch 'CPlayerSkillCatalog::Get_Skills' -or
    $effectToolSource -notmatch 'Material Inputs' -or
    $effectToolSource -notmatch 'Mesh Shape' -or
    $effectToolSource -notmatch 'Complete Effect' -or
    $effectToolSource -notmatch 'Solo Element' -or
    $effectToolSource -notmatch 'Mute Element' -or
    $effectToolSource -notmatch 'Save As' -or
    $effectToolSource -notmatch 'Try_ReloadActiveDocument' -or
    $effectToolSource -match 'Reload Published Effects' -or
    $effectToolSource -match 'Published Effects') {
    throw 'All Effects authoring tree, stable resource cards, preview filters, and Data Files commands are required without Published authoring UI.'
}
if ($effectToolSource -notmatch 'm_bActiveDocumentMatchesRuntime' -or
    $effectToolSource -notmatch 'Refresh_RuntimeEquivalence' -or
    $effectToolSource -notmatch 'CEffectDocumentCodec::Serialize\(\*pRuntimeDocument\)' -or
    $effectToolSource -notmatch '!Has_UnsavedWork\(\)') {
    throw 'Animation cue transfer must use the cached equivalence of the exact clean document admitted in the runtime Effect catalog.'
}
$thumbnailSource = Get-Content -LiteralPath (
    Require-File 'Client\Private\Effect_ThumbnailCache.cpp') -Raw -Encoding UTF8
$materialTemplateSource = Get-Content -LiteralPath (
    Require-File 'Client\Public\Effect_MaterialTemplate.h') -Raw -Encoding UTF8
if ($thumbnailSource -notmatch 'Create_ModelThumbnail' -or
    $thumbnailSource -notmatch 'Engine::CModel::Create' -or
    $thumbnailSource -notmatch 'Shader_VtxMeshPreview\.hlsl' -or
    $effectToolSource -cmatch 'WMODEL' -or
    $materialTemplateSource -notmatch 'effect\.standard' -or
    $materialTemplateSource -notmatch 'g_BaseTexture' -or
    $materialTemplateSource -notmatch 'g_DissolveTexture') {
    throw 'DDS/CModel thumbnails and the explicit standard Material Template boundary are required.'
}
if ($effectToolHeader -notmatch 'Rebuild_ResourceBrowserView' -or
    $effectToolHeader -notmatch 'm_iResourceCatalogRevision' -or
    $effectToolSource -notmatch 'm_iResourceViewRevision == m_iResourceCatalogRevision' -or
    $effectToolSource -notmatch 'm_VisibleResourceIndices' -or
    $effectToolSource -notmatch 'EffectTool\.ResourceGrid' -or
    $effectToolSource -notmatch 'FPS %\.1f \| Frame %\.2f ms' -or
    $thumbnailSource -notmatch 'MAX_LOADS_PER_FRAME = 1u' -or
    $thumbnailSource -notmatch 'LOAD_INTERVAL_FRAMES = 2u' -or
    $thumbnailSource -notmatch 'MAX_TEXTURE_DIMENSION = 256u') {
    throw 'Resource Browser filtering must be revision-cached and thumbnail work must be frame-budgeted with visible Effect Tool diagnostics.'
}
if ($effectToolHeader -notmatch 'EFFECT_RESOURCE_DOMAIN_CATALOG' -or
    $effectToolHeader -notmatch 'strDomainId' -or
    $effectToolHeader -notmatch 'm_ResourceDomains' -or
    $effectToolHeader -notmatch 'm_DataFileDomains' -or
    $effectToolHeader -notmatch 'm_strSelectedAuthoringDomainId' -or
    $effectToolSource -notmatch 'Authoring Category##ResourceDomain' -or
    $effectToolSource -notmatch 'Authoring Category##DataFilesDomain' -or
    $effectToolSource -notmatch 'Entry\.strDomainId != strDomainId' -or
    $effectToolSource -notmatch 'Select_AuthoringDomainForClass' -or
    $effectToolSource -notmatch 'EffectAsset_DomainId' -or
    $effectToolSource -notmatch 'Uncategorized') {
    throw 'Effect resources and Data Files must share a class/boss authoring-domain boundary.'
}
if ($effectToolHeader -notmatch 'm_eResourceLibraryFileKind' -or
    $effectToolSource -notmatch 'SeparatorText\("Resource Library"\)' -or
    $effectToolSource -notmatch 'RadioButton\("Meshes"' -or
    $effectToolSource -notmatch 'RadioButton\("Textures"' -or
    $effectToolSource -notmatch 'PARTICLE == pElement->eKind' -or
    $effectToolSource -notmatch 'ImVec2\(540\.f, 500\.f\)' -or
    $effectToolSource -notmatch 'Promote to Authored Skill' -or
    $effectToolSource -notmatch 'Try_PromoteImportedDocument') {
    throw 'Resource Library, mesh-backed Particle cards, larger All Effects, and Imported promotion are required.'
}
if ($effectToolHeader -notmatch 'PENDING_DOCUMENT_LOAD' -or
    $effectToolHeader -notmatch 'm_SynchronizedAnimationClips' -or
    $effectToolSource -notmatch 'Unsaved Effect Changes' -or
    $effectToolSource -notmatch 'Save & Load' -or
    $effectToolSource -notmatch 'Discard & Load' -or
    $effectToolSource -notmatch 'Try_LoadDocumentPathStaged' -or
    $effectToolSource -notmatch 'bBypassUnsavedGuard' -or
    $effectToolSource -notmatch 'Update_SynchronizedAnimationSequence' -or
    $effectToolSource -notmatch 'm_iSynchronizedAnimationClipIndex \+ 1u' -or
    $effectToolSource -notmatch 'Start_Animation\(\s*NextClip\.strClipName\.c_str\(\), false\)' -or
    $effectToolSource -notmatch 'Audition Selected' -or
    $effectToolSource -notmatch 'EFFECT_PREVIEW_FILTER::SOLO_SELECTED' -or
    $effectToolSource -notmatch 'Selected->Detail\.Timing\.fStartDelaySeconds' -or
    $effectToolSource -notmatch 'Applied to active Document memory; Save required to persist\.' -or
    $effectToolSource -notmatch 'Particle System \| Source Systems ' -or
    $effectToolSource -notmatch 'Mesh-backed ' -or
    $effectToolSource -notmatch 'iParticleBudget') {
    throw 'Effect document switching, animation sequence, Element audition, Apply feedback, and Particle layer summary must remain explicit.'
}
foreach ($scopeName in @(
    'EffectTool.AuthoringWindow',
    'EffectTool.ModelViewWindow',
    'EffectTool.DetailWindow',
    'EffectTool.AllEffectsWindow',
    'EffectTool.DataFilesWindow',
    'EffectTool.ThumbnailTrim')) {
    if ($effectToolSource -notmatch [regex]::Escape($scopeName)) {
        throw "Effect Tool profiler capture is missing detail scope: $scopeName"
    }
}
$authoringHeader = Get-Content -LiteralPath (
    Require-File 'Client\Public\Effect_AuthoringDocument.h') -Raw -Encoding UTF8
$rendererSource = Get-Content -LiteralPath (
    Require-File 'Client\Private\Effect_DocumentRenderer.cpp') -Raw -Encoding UTF8
$playbackSource = Get-Content -LiteralPath (
    Require-File 'Client\Private\Effect_Playback.cpp') -Raw -Encoding UTF8
if ($authoringHeader -notmatch 'EFFECT_AUTHORING_FORMAT_VERSION = 11u' -or
	$authoringHeader -notmatch 'struct EFFECT_SOURCE_MATERIAL_DESC' -or
	$authoringHeader -notmatch 'DynamicParameterSemantics' -or
	$codecSource -notmatch 'Read_SourceMaterialProfile' -or
	$rendererSource -notmatch 'SourceMaterialProfileIndex' -or
	$rendererSource -notmatch 'Resolve_SubUVFrames' -or
	$authoringHeader -notmatch 'struct EFFECT_PARTICLE_SYSTEM_DESC' -or
    $authoringHeader -notmatch 'ParticleSystem' -or
    $authoringHeader -notmatch 'struct EFFECT_MODEL_CUE_DESC' -or
    $authoringHeader -notmatch 'ModelCues' -or
    $codecSource -notmatch 'Read_ModelCueTransform' -or
    $codecSource -notmatch 'Is_SafeModelCueAssetId' -or
    $rendererSource -notmatch 'Shader_VtxAnimMeshBinary\.hlsl' -or
    $rendererSource -notmatch 'Stage_ModelCueResource' -or
    $rendererSource -notmatch 'Render_ModelCues' -or
    $playbackSource -notmatch 'Cue\.fStartDelaySeconds \+ Cue\.fDurationSeconds') {
    throw 'Current Effect Particle System and animated Model Cue codec/runtime paths are required.'
}
if ($effectToolHeader -notmatch 'EFFECT_DETAIL_SELECTION' -or
    $effectToolHeader -notmatch 'SOLO_PARTICLE_SYSTEM' -or
    $effectToolSource -notmatch 'Try_SelectParticleSystem' -or
    $effectToolSource -notmatch 'Apply Particle System' -or
    $effectToolSource -notmatch 'Audition Particle System' -or
    $effectToolSource -notmatch 'Source Systems %zu \| Emitters %zu \| Layers %zu' -or
    $playbackSource -notmatch 'fUniformScaleMultiplier' -or
    $playbackSource -notmatch 'fDirectionYawDegrees' -or
    $playbackSource -notmatch 'fInitialSpeedMultiplier') {
    throw 'Effect Tool must expose one non-destructive Particle System parent over source layers.'
}
$effectAuthoringDomains = @(
    'Artist',
    'DimensionMaster',
    'LanceMaster',
    'Valtan',
    'Warlord'
)
$effectPaletteResourceCount = 0
foreach ($domainId in $effectAuthoringDomains) {
    $domainRoot = Join-Path $ResourceRoot (Join-Path 'Effect' $domainId)
    if (-not [IO.Directory]::Exists($domainRoot)) {
        throw "Missing Effect authoring resource domain: $domainId"
    }
    $models = @(Get-ChildItem -LiteralPath $domainRoot -Recurse -File `
        -Filter '*.wmodel' -ErrorAction Stop)
    $textures = @(Get-ChildItem -LiteralPath $domainRoot -Recurse -File `
        -Filter '*.dds' -ErrorAction Stop)
    if ($models.Count -eq 0 -or $textures.Count -eq 0) {
        throw "Effect authoring domain must expose WModel and DDS resources: $domainId"
    }
    $effectPaletteResourceCount += $models.Count + $textures.Count
}
$rendererSource = Get-Content -LiteralPath (
    Require-File 'Client\Private\Effect_DocumentRenderer.cpp') `
    -Raw -Encoding UTF8
if ($rendererSource -notmatch 'fSequenceTerm\) \+\s+Element\.Detail\.UV\.iTileIndex' -or
    $effectToolSource -notmatch 'Active\.Detail\.UV\.iTileIndex = iTile') {
    throw 'UV tile clicks must set the authored sequence offset and refresh preview.'
}
if ($rendererSource -match 'unordered_map<const EFFECT_ELEMENT_DESC\*' -or
    $rendererSource -notmatch 'for \(const EFFECT_ELEMENT_DESC& DocumentElement : m_Document\.Elements\)' -or
    $rendererSource -notmatch 'Render_AfterImages\(Frame, strElementId\)' -or
    $rendererSource -notmatch 'Render_Particles\(Frame, strElementId\)' -or
    $rendererSource -notmatch 'Render_Trails\(Frame, strElementId\)') {
    throw 'Effect draw submission must preserve authored Element stack order across all five kinds.'
}
$decalRenderSource = [regex]::Match($rendererSource,
    'HRESULT Client::CEffectDocumentRenderer::Render_Decal\([\s\S]*?\r?\n\}').Value
if ([string]::IsNullOrWhiteSpace($decalRenderSource) -or
    $decalRenderSource -notmatch 'Bind_MaterialInputs\(m_pDecalShader' -or
    $decalRenderSource -match 'Bind_Matrix\("g_WorldMatrix"' -or
    $decalRenderSource -match 'Bind_Common\(m_pDecalShader') {
    throw 'Decal must bind material inputs without absent forward World/View/Projection shader variables.'
}

$registrationFiles = if ([IO.Directory]::Exists(
    (Join-Path $BundleRoot 'ProjectRegistration'))) {
    @(
        'ProjectRegistration\Client.vcxproj.items.xml',
        'ProjectRegistration\Client.vcxproj.filters.items.xml',
        'ProjectRegistration\Engine.vcxproj.items.xml',
        'ProjectRegistration\Engine.vcxproj.filters.items.xml'
    )
}
else {
    @(
        'Client\Default\Client.vcxproj',
        'Client\Default\Client.vcxproj.filters',
        'Engine\Default\Engine.vcxproj',
        'Engine\Default\Engine.vcxproj.filters'
    )
}
foreach ($relativePath in $registrationFiles) {
    $path = Require-File $relativePath
    try {
        $null = [xml](Get-Content -LiteralPath $path -Raw -Encoding UTF8)
    }
    catch {
        throw "Invalid project registration XML in ${relativePath}: $($_.Exception.Message)"
    }
}

$skills = Read-JsonFile 'Data\Balance\PlayerSkills.json'
$boundSkillRows = @($skills.skills | Where-Object {
    [string]$_.characterClass -eq 'DIMENSIONMASTER' -and
    [string]$_.inputSlot -ne 'SPACE'
})
if ($boundSkillRows.Count -eq 0) {
    throw 'PlayerSkills.json has no DimensionMaster trial roster.'
}
$expectedEffectIds = [Collections.Generic.List[string]]::new()
foreach ($skill in $boundSkillRows) {
    $skillId = [int64]$skill.skillId
    $expectedEffectId = "effect.dimensionmaster.skill.$skillId"
    if ([string]$skill.effectId -cne $expectedEffectId) {
        throw "PlayerSkills.effectId mismatch for current DimensionMaster $($skill.inputSlot) skill $skillId"
    }
    $expectedEffectIds.Add($expectedEffectId)
    if ([string]$skill.skillKind -eq 'COMBO') {
        for ($stage = 1; $stage -le @($skill.comboStages).Count; ++$stage) {
            $expectedEffectIds.Add("$expectedEffectId.ba$stage")
        }
    }
}
$expectedEffectIds = @($expectedEffectIds)
$expectedEffectCount = $expectedEffectIds.Count

$catalog = Read-JsonFile 'Data\Effects\EffectCatalog.json'
if ($catalog.formatVersion -ne 1 -or
    $catalog.effects.Count -ne $expectedEffectCount) {
    throw "EffectCatalog.json must contain exactly the current DimensionMaster roster and its combo stages: expected=$expectedEffectCount actual=$($catalog.effects.Count)"
}
$catalogIds = @($catalog.effects | ForEach-Object { [string]$_.effectAssetId })
if (@($catalogIds | Sort-Object -Unique).Count -ne $expectedEffectCount) {
    throw 'EffectCatalog.json contains a duplicate EffectAssetId.'
}
foreach ($effectId in $expectedEffectIds) {
    if ($catalogIds -cnotcontains $effectId) {
        throw "EffectCatalog.json is missing $effectId"
    }
}

$documentResourceIds = [Collections.Generic.HashSet[string]]::new(
    [StringComparer]::Ordinal)
foreach ($entry in $catalog.effects) {
    $relativeDocument = [string]$entry.authoringPath
    if ($relativeDocument -notmatch '^Effects/Authored/[a-z0-9._-]+\.effect\.json$') {
        throw "Unsafe catalog document path: $relativeDocument"
    }
    $document = Read-JsonFile (Join-Path 'Data' $relativeDocument)
    if ($document.version -notin @(5, 6, 7, 8, 9, 10, 11) -or
        $document.effectAssetId -cne $entry.effectAssetId -or
        @($document.elements).Count -eq 0) {
        throw "Effect document header/elements are invalid: $relativeDocument"
    }
    if ($document.version -ge 8) {
        $particleSystem = $document.particleSystem
        if ($null -eq $particleSystem -or
            [double]$particleSystem.uniformScaleMultiplier -le 0 -or
            [double]$particleSystem.uniformScaleMultiplier -gt 100 -or
            [Math]::Abs([double]$particleSystem.yawOffsetDegrees) -gt 3600 -or
            [Math]::Abs([double]$particleSystem.directionYawDegrees) -gt 3600 -or
            [double]$particleSystem.initialSpeedMultiplier -lt 0 -or
            [double]$particleSystem.initialSpeedMultiplier -gt 100) {
            throw "Effect v8 Particle System is invalid: $relativeDocument"
        }
    }
    $elementIds = @($document.elements | ForEach-Object { [string]$_.id })
    if (@($elementIds | Sort-Object -Unique).Count -ne $elementIds.Count) {
        throw "Duplicate Element ID in $relativeDocument"
    }
    foreach ($element in $document.elements) {
        if ($document.version -ge 6 -and
            ($element.displayName -isnot [string] -or
             $element.groupId -isnot [string] -or
             $element.sourceNode -isnot [string] -or
             $element.visible -isnot [bool] -or
             @('effect.standard','effect.source_material') -cnotcontains
                [string]$element.material.templateId)) {
            throw "Effect v6 Element metadata/template is invalid: $relativeDocument"
        }
        if (@('mesh','sprite','particle','decal','trail','light','screenPost') -cnotcontains
            [string]$element.kind) {
            throw "Unknown Element kind in ${relativeDocument}: $($element.kind)"
        }
        foreach ($resource in @($element.resources)) {
            $assetId = [string]$resource.assetId
            if ($assetId -notmatch '^Effect/[A-Za-z0-9_./-]+\.(dds|wmodel)$' -or
                $assetId.Contains('..') -or [IO.Path]::IsPathRooted($assetId)) {
                throw "Unsafe Effect resource asset ID: $assetId"
            }
            $null = $documentResourceIds.Add($assetId)
            $resourcePath = Join-Path $ResourceRoot ($assetId -replace '/', '\')
            if (-not [IO.File]::Exists($resourcePath)) {
                throw "Missing Effect resource payload: $assetId"
            }
        }
    }
    if ($document.version -ge 7) {
        $modelCues = @($document.modelCues)
        if ($modelCues.Count -gt 8) {
            throw "Effect Model Cue budget is invalid: $relativeDocument"
        }
        $cueIds = @($modelCues | ForEach-Object { [string]$_.cueId })
        if (@($cueIds | Sort-Object -Unique).Count -ne $cueIds.Count) {
            throw "Duplicate Effect Model Cue ID: $relativeDocument"
        }
        foreach ($cue in $modelCues) {
            $modelAssetId = [string]$cue.modelAssetId
            if ([string]$cue.cueId -notmatch '^[A-Za-z0-9_.-]+$' -or
                [string]::IsNullOrWhiteSpace([string]$cue.clipName) -or
                $modelAssetId -notmatch '^Character/[A-Za-z0-9_./-]+\.wmodel$' -or
                $modelAssetId.Contains('..') -or
                [double]$cue.startDelaySeconds -lt 0 -or
                [double]$cue.durationSeconds -le 0) {
                throw "Effect Model Cue is invalid: $relativeDocument"
            }
            $modelPath = Join-Path $ResourceRoot ($modelAssetId -replace '/', '\')
            if (-not [IO.File]::Exists($modelPath)) {
                throw "Missing Effect Model Cue payload: $modelAssetId"
            }
        }
    }
}

$completeT = Read-JsonFile `
    'Data\Effects\Authored\effect.dimensionmaster.skill.2050500.effect.json'
$completeTReceipt = Read-JsonFile `
    'Data\Effects\Imported\DimensionMaster\Converted\skill.2050500.element-conversion-receipt.json'
if ($completeT.version -notin @(10, 11) -or
    @($completeT.elements).Count -ne
        [int]$completeTReceipt.summary.emittedElementCount -or
    [double]$completeT.particleSystem.uniformScaleMultiplier -ne 1.0 -or
    [double]$completeT.particleSystem.initialSpeedMultiplier -ne 1.0 -or
    @($completeT.modelCues).Count -ne 1 -or
    [string]$completeT.modelCues[0].modelAssetId -cne
        'Character/DimensionMaster/DimensionMaster_DimensionSummon.wmodel' -or
    [string]$completeT.modelCues[0].clipName -cne
        'sk_swp_dms_00_sk_sk_dimensionprison') {
    throw 'DimensionMaster T 2050500 must match its source-derived Element receipt and Summon cue.'
}

$eventsPath = Require-File `
    'Data\Animation\Authored\DimensionMaster\DimensionMaster.animevents'
$eventLines = @(Get-Content -LiteralPath $eventsPath -Encoding UTF8)
if ($eventLines.Count -lt 2 -or
    $eventLines[0] -notmatch '^LOSTARK_ANIM_EVENTS 5 "DimensionMaster" ([0-9]+)$') {
    throw 'DimensionMaster.animevents must have a v5 owner/count header.'
}
$declaredRows = [int]$Matches[1]
$actualRows = @($eventLines | Select-Object -Skip 1 |
    Where-Object { -not [string]::IsNullOrWhiteSpace($_) }).Count
if ($declaredRows -ne $actualRows) {
    throw "Animation event row count mismatch: declared=$declaredRows actual=$actualRows"
}
$admittedRows = @($eventLines | Where-Object {
    $_ -match ' EFFECT ' -and $_ -match ' effectref=asset '
})
$skillBindings = Read-JsonFile `
    'Data\Animation\Authored\DimensionMaster\DimensionMaster.skillbindings.json'
$clipOwners = @{}
foreach ($binding in @($skillBindings.bindings)) {
    foreach ($clipValue in @($binding.clips)) {
        $clipName = if ($clipValue -is [string]) {
            [string]$clipValue
        }
        else {
            [string]$clipValue.clip
        }
        if ([string]::IsNullOrWhiteSpace($clipName) -or
            $clipOwners.ContainsKey($clipName)) {
            throw "DimensionMaster skill binding has an empty or duplicate clip: $clipName"
        }
        $clipOwners[$clipName] = [int64]$binding.skillId
    }
}
foreach ($row in $admittedRows) {
    $cueMatch = [regex]::Match($row,
        '^"(?<Clip>[^"]+)" EFFECT .* payload="(?<Effect>effect\.dimensionmaster\.skill\.(?<Skill>[0-9]+)(?:\.ba(?<Stage>[0-9]+))?)" effectref=asset ')
    if (-not $cueMatch.Success) {
        throw "Malformed admitted DimensionMaster Effect cue: $row"
    }
    $clipName = $cueMatch.Groups['Clip'].Value
    $effectId = $cueMatch.Groups['Effect'].Value
    $skillId = [int64]$cueMatch.Groups['Skill'].Value
    if (-not $clipOwners.ContainsKey($clipName) -or
        [int64]$clipOwners[$clipName] -ne $skillId -or
        $catalogIds -cnotcontains $effectId) {
        throw "Animation Effect cue is not owned by the current skill binding/catalog: $row"
    }
}
$tCue = @($admittedRows | Where-Object {
    $_ -match '^"pc_sp_m_00_sk_sk_dimensionprison" EFFECT startms=0 ' -and
    $_ -match ' payload="effect\.dimensionmaster\.skill\.2050500" '
})
if ($tCue.Count -ne 1) {
    throw 'DimensionMaster T 2050500 must start its Summon Effect once with dimensionprison.'
}
$comboSkill = @($boundSkillRows | Where-Object skillKind -eq 'COMBO')
if ($comboSkill.Count -ne 1) {
    throw 'DimensionMaster trial roster must have exactly one LMB combo Effect owner.'
}
$comboBinding = @($skillBindings.bindings | Where-Object {
    [int64]$_.skillId -eq [int64]$comboSkill[0].skillId
})
if ($comboBinding.Count -ne 1 -or
    @($comboBinding[0].clips).Count -ne @($comboSkill[0].comboStages).Count) {
    throw 'DimensionMaster combo clips and Server stages must have the same count.'
}
for ($stage = 1; $stage -le @($comboBinding[0].clips).Count; ++$stage) {
    $clipValue = @($comboBinding[0].clips)[$stage - 1]
    $comboClip = if ($clipValue -is [string]) {
        [string]$clipValue
    }
    else {
        [string]$clipValue.clip
    }
    $comboEffectId = "effect.dimensionmaster.skill.$($comboSkill[0].skillId).ba$stage"
    if (@($admittedRows | Where-Object {
        $_ -match ('^"' + [regex]::Escape($comboClip) + '" ') -and
        $_ -match ' EFFECT startms=0 ' -and
        $_ -match ('payload="' + [regex]::Escape($comboEffectId) + '"')
    }).Count -ne 1) {
        throw "Missing DimensionMaster combo Effect cue: $comboClip"
    }
}

Write-Host ('PASS: final Effect Tool bundle; code={0}, documents={1}, resources={2}, palette={3}, cues={4}.' -f
    $requiredCode.Count, $expectedEffectCount, $documentResourceIds.Count,
    $effectPaletteResourceCount, $admittedRows.Count)
