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
    $effectToolSource -notmatch 'Load Document' -or
    $effectToolSource -notmatch 'Product Play' -or
    $effectToolSource -notmatch 'InputText\("Effect Name"' -or
    $effectToolSource -notmatch 'Start_WorldPreviewFromBeginning\(\)' -or
    $effectToolSource -notmatch 'Try_SetPreviewFilter\(EFFECT_PREVIEW_FILTER::COMPLETE\)[\s\S]{0,220}?Start_WorldPreviewFromBeginning\(\);') {
    throw 'Loading an existing Effect must be unambiguous and automatically restart complete world playback.'
}
if ($mainAppSource -notmatch 'FPS: %\.1f\s+\|\s+Frame: %\.2f ms' -or
    $mainAppSource -notmatch 'io\.Framerate' -or
    $mainAppSource -notmatch 'io\.DeltaTime \* 1000\.f') {
    throw 'F1 Developer Tools must always show smoothed FPS and latest frame time without enabling the profiler.'
}
$productRefreshMatch = [regex]::Match(
    $effectToolSource,
    'bool_t Client::CEffect_Tool::Refresh_AllEffects\([^)]*\)[\s\S]*?bool_t Client::CEffect_Tool::Refresh_DataFiles')
$productSelectMatch = [regex]::Match(
    $effectToolSource,
    'bool_t Client::CEffect_Tool::Try_SelectProductCue\([^)]*\)[\s\S]*?bool_t Client::CEffect_Tool::Try_SelectParticleSystem')
$productRootMatch = [regex]::Match(
    $effectToolSource,
    'bool_t Client::CEffect_Tool::Resolve_PreviewRoot\([^)]*\)[\s\S]*?void Client::CEffect_Tool::Clear_ProductCuePreview')
$startPreviewMatch = [regex]::Match(
    $effectToolSource,
    'void Client::CEffect_Tool::Start_WorldPreviewFromBeginning\(\)[\s\S]*?void Client::CEffect_Tool::Synchronize_LoadedSkillPreview')
if ($effectToolHeader -notmatch 'struct PRODUCT_CUE' -or
    $effectToolHeader -notmatch 'EFFECT_PRODUCT_PREVIEW' -or
    $effectToolHeader -notmatch 'Try_SelectProductCue' -or
    -not $productRefreshMatch.Success -or
    -not $productSelectMatch.Success -or
    -not $productRootMatch.Success -or
    -not $startPreviewMatch.Success -or
    $effectToolSource -notmatch 'ImGuiTreeNodeFlags_Selected' -or
    $effectToolSource -notmatch 'Active Product Cue missing' -or
    $effectToolSource -notmatch 'Source / Imported Diagnostics' -or
    $effectToolSource -notmatch 'BeginDisabled\(Entry\.ProductCues\.empty\(\)\)' -or
    $effectToolSource -notmatch 'Try_SelectProductCue\(Entry, iProductCueIndex\)' -or
    $productRefreshMatch.Value -notmatch 'CAnimationSkillBindingDocument::Validate' -or
    $productRefreshMatch.Value -notmatch 'CAnimationEffectCueDocument::Load' -or
    $productRefreshMatch.Value -notmatch 'ProductCues\.push_back' -or
    $productRefreshMatch.Value -match 'Skill\.strEffectId' -or
    $productSelectMatch.Value -notmatch 'CEffectCatalog::Contains\(ProductCue\.Cue\.strEffectAssetId\)' -or
    $productSelectMatch.Value -notmatch 'Try_LoadDocument\(ProductCue\.Cue\.strEffectAssetId\)' -or
    $productSelectMatch.Value -notmatch 'Start_WorldPreviewFromBeginning\(\)' -or
    $productRootMatch.Value -notmatch 'Compose_EffectLocal' -or
    $productRootMatch.Value -notmatch 'strCueAnchor' -or
    $productRootMatch.Value -notmatch 'Resolve_RootTransform' -or
    $productRootMatch.Value -notmatch 'Resolve_AnchorTransform' -or
    $productRootMatch.Value -notmatch 'eFollowPolicy' -or
    $productRootMatch.Value -notmatch 'iStartMs' -or
    $productRootMatch.Value -notmatch 'eStopPolicy' -or
    $productRootMatch.Value -notmatch 'iEndMs' -or
    $startPreviewMatch.Value -match 'm_ePreviewPivotKind\s*=') {
    throw 'All Effects Product Play must use only admitted asset cues with cue timing/anchor/follow placement, while missing Product cues fail closed and Source/Imported rows remain diagnostics.'
}
if ($effectToolHeader -notmatch 'Stage_DetailDraftPreview' -or
    $effectToolSource -notmatch 'Drag numeric values for live world preview' -or
    $effectToolSource -notmatch 'DragFloat3\("Velocity"' -or
    $effectToolSource -notmatch 'CEffect_Tool::Stage_DetailDraftPreview[\s\S]{0,1800}?Stage_WorldPreview\(Staged\)' -or
    $effectToolSource -notmatch 'if \(bChanged\)[\s\S]{0,180}?Stage_DetailDraftPreview\(\)' -or
    $effectToolSource -notmatch 'Detail draft reverted to the active Document preview') {
    throw 'Effect Detail numeric drags must live-stage a draft while Apply/Revert retain the active Document boundary.'
}
$effectAuthoringDocumentHeader = Get-Content -LiteralPath (
    Require-File 'Client\Public\Effect_AuthoringDocument.h') -Raw
$detailApplyPreservesSourceRecipe =
    $effectToolSource -match 'Element\.SourceRecipe\s*=\s*m_DetailDraft->SourceRecipe' -or
    ($effectToolSource -match 'Apply_EffectElementDetailDraft\(Element, \*m_DetailDraft\)' -and
     $effectAuthoringDocumentHeader -match 'Apply_EffectElementDetailDraft[\s\S]{0,800}?Target\.SourceRecipe\s*=\s*Draft\.SourceRecipe')
if (-not $detailApplyPreservesSourceRecipe -or
    $effectToolSource -notmatch 'Runtime Sample' -or
    $effectToolSource -notmatch 'Copy Runtime Probe' -or
    $effectToolSource -notmatch 'Query_ParticleRuntimeProbe') {
    throw 'Effect Detail Apply must preserve SourceRecipe and expose the read-only evaluated runtime probe.'
}
if ($effectToolHeader -notmatch 'SKILL' -or
    $effectToolHeader -notmatch 'Try_SelectFirstEmitter' -or
    $effectToolSource -notmatch 'Imported Cascade Diagnostics \| Source Systems' -or
    $effectToolSource -notmatch 'Open First Emitter' -or
    $effectToolSource -notmatch 'Bound Resources' -or
    $effectToolSource -notmatch 'Material Instance Parameters' -or
    $effectToolSource -notmatch 'Velocity Over Life' -or
    $effectToolSource -notmatch 'Sphere Surface' -or
    $effectToolSource -notmatch 'SubUV Animation' -or
    $effectToolSource -notmatch 'Vector Field') {
    throw 'Effect Tool must expose the stable Skill/Particle System/Component/Emitter/Module path, dynamic Resource Set, and source-class-aware Module labels.'
}
$applyDetailMatch = [regex]::Match(
    $effectToolSource,
    'CEffect_Tool::Apply_DetailDraft[\s\S]*?\n\}')
if (-not $applyDetailMatch.Success -or
    $applyDetailMatch.Value -match 'Element\.ResourceBindings\s*=' -or
    $applyDetailMatch.Value -match 'Element\.ActionCueAttachment\s*=') {
    throw 'Effect Detail Apply must not overwrite ResourceBindings or ActionCueAttachment outside an editing contract.'
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
if ($effectToolHeader -notmatch 'Refresh_AnimationClipLabels' -or
    $effectToolHeader -notmatch 'Restart_SynchronizedAnimationSequence' -or
    $effectToolSource -notmatch 'Reload Skill Labels' -or
    $effectToolSource -notmatch '\[Input\] Korean Skill Name \| Model Clip' -or
    $effectToolSource -notmatch 'Skill->strDisplayName' -or
    $effectToolSource -notmatch 'restoration-candidate' -or
    $effectToolSource -notmatch 'Start_WorldPreviewFromBeginning\(\)[\s\S]{0,180}?Restart_SynchronizedAnimationSequence\(\)') {
    throw 'Model View must label authored clips by input/Korean skill name and restart the bound skill animation with restoration preview playback.'
}
if ($effectToolHeader -notmatch 'Try_ResolveSynchronizedAnimationTime' -or
    $effectToolSource -notmatch 'Animation source time owns this Effect timeline' -or
    $effectToolSource -notmatch 'Try_ResolveSynchronizedAnimationTime\(fSynchronizedAnimationTime\)' -or
    $effectToolSource -notmatch 'Seek_SynchronizedAnimationSequence\(m_fPreviewTimeSeconds\)' -or
    $effectToolSource -notmatch 'Set_SynchronizedAnimationPaused\(true\)' -or
    $effectToolSource -notmatch 'fPosition / fTicksPerSecond') {
    throw 'Bound skill animation source time must own Effect play, pause, loop, and sample-time synchronization.'
}
if ($animationPreviewAssets -notmatch 'bPlayableClassBody' -or
    $effectToolSource -notmatch 'Render_Selector\(false, \{\}, false\)' -or
    $previewPanelSource -notmatch '!includePreviewOnlyTargets && !asset\.bPlayableClassBody' -or
    $animationToolSource -notmatch 'Render_Selector\(false, \{\}\)') {
    throw 'Effect Tool must list playable class bodies only while Animation Tool retains auxiliary preview targets.'
}
$presentationSource = Get-Content -LiteralPath (
    Require-File 'Client\Private\Effect_PresentationService.cpp') -Raw -Encoding UTF8
if ($mainAppSource -notmatch 'Update_Engine\(fTimeDelta\);\s+CEffectPresentationService::Commit_PendingSpawns\(\);\s+CEffectPresentationService::Synchronize_FollowAnchors\(\);' -or
    $presentationSource -notmatch 'Synchronize_FollowAnchors\(\)' -or
    $presentationSource -notmatch 'bFollowAnchorMissing' -or
    $presentationSource -notmatch 'Set_Visible\(false\)') {
    throw 'Runtime FOLLOW anchors must resample after Character animation update and defer unsafe removal.'
}
if ($effectToolSource -notmatch 'Unload Effect with unsaved changes\?' -or
    $effectToolSource -notmatch 'if \(Has_UnsavedWork\(\)\)' -or
    $effectToolSource -notmatch 'Has_UnappliedDetailDraft' -or
    $effectToolSource -notmatch 'Apply or Revert the open Detail draft before saving') {
    throw 'Effect Create/Load/Unload/Save must preserve unsaved Document and Detail work.'
}
if ($effectToolSource -notmatch 'Restoration Session' -or
    $effectToolSource -notmatch 'Apply \+ Save Authored' -or
    $effectToolSource -notmatch 'Try_ApplyDraftAndSave' -or
    $effectToolSource -notmatch 'Assembly/WFX/Runtime Catalog publish pending' -or
    $effectToolSource -notmatch 'Published Runtime Hierarchy \(diagnostic\)' -or
    $effectToolSource -notmatch 'bHasPublishedAssembly && !bActiveProductDocument') {
    throw 'Effect restoration authoring must expose one Apply/Save loop, explicit publish state, and the current Authored tree before stale runtime hierarchy.'
}
if ($effectToolSource -notmatch 'Loaded editable draft; preview is hidden' -or
    $effectToolSource -notmatch 'Loaded existing Effect for inspection without GPU staging' -or
    $effectToolSource -notmatch 'open Detail draft before selecting another Element' -or
    $effectToolSource -notmatch 'open Detail draft before adding an Element' -or
    $effectToolSource -notmatch 'open Detail draft before deleting an Element' -or
    $effectToolSource -notmatch 'open Detail draft before clearing Elements') {
    throw 'Valid partial drafts must reload, and dirty Detail drafts must survive selection/structure commands.'
}
$codecSource = Get-Content -LiteralPath (
    Require-File 'Client\Private\Effect_DocumentCodec.cpp') -Raw -Encoding UTF8
$codecHeader = Get-Content -LiteralPath (
    Require-File 'Client\Public\Effect_DocumentCodec.h') -Raw -Encoding UTF8
if ($codecSource -notmatch 'Authoring save preserves valid partial drafts' -or
    $codecSource -notmatch 'Save_EffectDocumentAtomic[\s\S]*?Validate\(Document, strOutError\)') {
    throw 'Effect authoring save must preserve valid non-drawable drafts; publisher keeps the drawable gate.'
}
if ($codecHeader -notmatch 'Save_AtomicIfUnchanged' -or
    $codecSource -notmatch 'Serialize\(RoundTrip\) != Json' -or
    $codecSource -notmatch 'changed on disk after it was loaded' -or
    $effectToolSource -notmatch 'm_strActiveDocumentBaselineCanonical' -or
    $effectToolSource -notmatch 'partial draft; world preview is hidden') {
    throw 'Effect authoring save must enforce canonical round-trip equality, reject stale writers, and distinguish partial-draft save from live preview.'
}
if ($effectToolSource -notmatch 'Finite Runtime Shader' -or
    $effectToolSource -notmatch 'Authored Named Parameters' -or
    $effectToolSource -notmatch 'EFFECT_SOURCE_RUNTIME_SHADER_PROFILE_IDS' -or
    $effectToolSource -notmatch 'RECONSTRUCTED_PROFILE') {
    throw 'Effect Detail must expose bounded runtime Material profile and named parameter tuning without claiming runtime exactness.'
}
if ($effectToolSource -notmatch 'Reference A/B Capture' -or
    $effectToolSource -notmatch 'Copy A/B Metadata' -or
    $effectToolSource -notmatch 'Screen Post: %s' -or
    $effectToolSource -notmatch '!m_bPreviewScreenPostEnabled' -or
    $effectToolSource -notmatch 'EFFECT_ELEMENT_KIND::SCREEN_POST') {
    throw 'Effect reference A/B must expose active ID, sample time, selected emitter, and isolated Screen Post state.'
}
if ($effectObjectSource -notmatch 'Advance_Preview' -or
    $effectToolSource -notmatch 'fSequentialAdvance' -or
    $effectToolSource -notmatch 'pObject->Advance_Preview\(fSequentialAdvance,\s*Root\)') {
    throw 'Playing Effect previews must advance sequentially; Seek is reserved for scrub and loop jumps.'
}
if ($effectToolSource -notmatch 'm_bPreviewPlaying = false;\s+bSeekAfterLoop = true;' -or
    $effectToolSource -notmatch 'Refresh_AllEffects' -or
    $effectToolSource -notmatch 'CPlayerSkillCatalog::Get_Skills' -or
    ($effectToolSource -notmatch 'Material Inputs' -and
     $effectToolSource -notmatch 'Bound Resources') -or
    $effectToolSource -notmatch 'Mesh Shape' -or
    $effectToolSource -notmatch 'Complete Effect' -or
    $effectToolSource -notmatch 'Solo Element' -or
    $effectToolSource -notmatch 'Mute Element' -or
    $effectToolSource -notmatch 'Element Groups' -or
    $effectToolSource -notmatch 'Solo Group' -or
    $effectToolSource -notmatch 'Mute Group' -or
    $effectToolHeader -notmatch 'SOLO_SELECTED_GROUP' -or
    $effectToolHeader -notmatch 'm_strSelectedElementGroupId' -or
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
$productReloadMatch = [regex]::Match(
    $effectToolSource,
    'CEffect_Tool::Try_PublishActiveProductAndReloadRuntime\(\)[\s\S]*?#endif\s*\n\}')
if (-not $productReloadMatch.Success -or
    $effectToolHeader -notmatch 'Try_PublishActiveProductAndReloadRuntime' -or
    $effectToolSource -notmatch 'Publish \+ Reload Product Test' -or
    $effectToolSource -match 'DIMENSIONMASTER_A_TRACK_B_EFFECT_ID' -or
    $productReloadMatch.Value -notmatch
        'm_ProductPreview->ProductCue\.Cue\.strEffectAssetId' -or
    $productReloadMatch.Value -notmatch
        'Animation_AssetName\(m_ProductPreview->eCharacterClass\)' -or
    $productReloadMatch.Value -notmatch '--character-class' -or
    $productReloadMatch.Value -notmatch 'Has_UnsavedWork\(\)' -or
    $productReloadMatch.Value -notmatch 'build_effect_components\.py' -or
    $productReloadMatch.Value -notmatch 'Publish-Effects\.ps1' -or
    $productReloadMatch.Value -notmatch 'CEffectCatalog::Load\(CatalogStatus\)') {
    throw 'Debug Product test must cross the exact Product cue Authored -> class builder -> publisher -> transactional Runtime Catalog boundary.'
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
$createMeshMatch = [regex]::Match(
    $effectToolSource,
    'bool_t Client::CEffect_Tool::Try_CreateMeshEffect\(\)[\s\S]*?bool_t Client::CEffect_Tool::Try_BindMeshAuthoringResource')
$createDocumentMatch = [regex]::Match(
    $effectToolSource,
    'bool_t Client::CEffect_Tool::Try_CreateDocument\(\)[\s\S]*?bool_t Client::CEffect_Tool::Try_CreateMeshEffect')
if (-not $createMeshMatch.Success -or -not $createDocumentMatch.Success -or
    $effectToolHeader -notmatch 'm_MeshAuthoringDraft' -or
    $effectToolHeader -notmatch 'Try_CreateMeshEffect' -or
    $effectToolSource -notmatch 'Mesh Effect Authoring' -or
    $effectToolSource -notmatch 'Manual Particle authoring is excluded from this workbench' -or
    $effectToolSource -notmatch 'InputText\("Effect Name"' -or
    $effectToolSource -notmatch 'bCanCreate = Is_EffectManualMeshCreateReady' -or
    $effectToolSource -notmatch 'Create Effect requires one WModel Mesh' -or
    $effectToolSource -notmatch 'Detail\.Mesh\.bUseModelMaterial = false' -or
    $effectToolSource -notmatch 'm_MeshAuthoringDraft\.Detail\.Transform\.vScale\s*=\s*\{[\s\S]{0,240}?EFFECT_MANUAL_MESH_DEFAULT_SCALE' -or
    $effectToolSource -notmatch 'EFFECT_STANDARD_MATERIAL_TEMPLATE_ID' -or
    $createMeshMatch.Value -notmatch 'Save_AtomicIfUnchanged' -or
    $createMeshMatch.Value -notmatch 'Refresh_DataFiles\(\)' -or
    $createMeshMatch.Value -notmatch 'Apply_DetailDraft\(Staged\)' -or
    $createMeshMatch.Value -match 'Reset_MeshAuthoringDraft\(\)' -or
    $createDocumentMatch.Value -match 'Reset_MeshAuthoringDraft\(\)' -or
    $materialTemplateSource -notmatch 'EFFECT_MANUAL_MESH_DEFAULT_SCALE\s*=\s*0\.01f' -or
    $materialTemplateSource -notmatch 'Is_EffectManualMeshCreateReady') {
    throw 'Reference-parity Mesh authoring must create and atomically save a named Data File without discarding builder selections.'
}
foreach ($slotLabel in @('Base', 'Noise', 'Mask', 'Emissive', 'Dissolve')) {
    if ($effectToolSource -notmatch ('"' + [regex]::Escape($slotLabel) + '"')) {
        throw "Mesh authoring slot is missing: $slotLabel"
    }
}
if ($commonShader -notmatch 'surfaceWarp' -or
    $commonShader -notmatch 'noiseSample\.rg \* 2\.f - 1\.f' -or
    $commonShader -notmatch 'Sample\(LinearSampler, surfaceUV\)' -or
    $commonShader -notmatch 'g_MaskTexture\.Sample\(LinearSampler, surfaceUV\)\.r' -or
    $commonShader -notmatch 'g_DissolveTexture\.Sample\(LinearSampler, surfaceUV\)\.r') {
    throw 'Standard Effect Material must execute Noise RG surface warp and R-channel Mask/Dissolve sampling.'
}
$standardShade = [regex]::Match(
    $commonShader,
    'EFFECT_PS_OUT Shade_Effect\([\s\S]*?\r?\n\}')
if (-not $standardShade.Success -or
    $standardShade.Value -match 'color\.rgb \*= g_EmissiveIntensity' -or
    $standardShade.Value -notmatch 'g_EmissiveTexture\.Sample\([\s\S]*?\)\.rgb \* g_EmissiveIntensity') {
    throw 'Bloom Intensity must affect the selected Emissive texture, not Base-only color.'
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
if ($effectToolSource -match 'bRefreshFilesAfterDomainSelection' -or
    $effectToolSource -notmatch 'Category changes filter the cached index; Refresh Index rescans disk\.' -or
    $effectToolSource -notmatch 'RecordRejectedDocument' -or
    $effectToolSource -notmatch 'invalid/duplicate entries') {
    throw 'Data Files category selection must use the cached index while explicit refresh isolates invalid documents per file.'
}
if ($effectToolHeader -notmatch 'Render_LoadedEffectContents' -or
    $effectToolHeader -notmatch 'Try_SetPreviewFilter' -or
    $effectToolHeader -notmatch 'Hide_WorldPreview' -or
    $effectToolSource -notmatch 'Loaded Effect Contents' -or
    $effectToolSource -notmatch 'Filter Effect Asset ID' -or
    $effectToolSource -notmatch 'Unload Document' -or
    $effectToolSource -notmatch 'saved Data File was preserved' -or
    $effectToolSource -notmatch 'Play Complete Effect' -or
    $effectToolSource -notmatch 'Play Group' -or
    $effectToolSource -notmatch 'Solo##' -or
    $effectToolSource -notmatch 'Hide Preview') {
    throw 'Data Files must expose explicit load/unload/hide semantics and complete/group/Element playback.'
}
if ($effectToolHeader -notmatch 'm_eResourceLibraryFileKind' -or
    $effectToolSource -notmatch 'SeparatorText\("Resource Library"\)' -or
    $effectToolSource -notmatch 'RadioButton\("Meshes"' -or
    $effectToolSource -notmatch 'RadioButton\("Textures"' -or
    $effectToolSource -notmatch 'PARTICLE == pElement->eKind' -or
    $effectToolSource -notmatch 'ImVec2\(540\.f, 500\.f\)' -or
    $effectToolSource -notmatch 'Promote Imported to Authored Skill' -or
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
    $effectToolSource -notmatch 'Cascade System \| Source Systems ' -or
    $effectToolSource -notmatch 'Standalone Mesh %zu \| Mesh Particle %zu' -or
    $effectToolSource -notmatch 'Standalone Sprite %zu \| Sprite Particle %zu' -or
    $effectToolSource -notmatch 'iParticleBudget') {
    throw 'Effect document switching, animation sequence, Element audition, Apply feedback, and Particle layer summary must remain explicit.'
}
$loadDocumentMatch = [regex]::Match(
    $effectToolSource,
    'bool_t Client::CEffect_Tool::Try_LoadDocumentPathStaged\([\s\S]*?bool_t Client::CEffect_Tool::Execute_PendingDocumentLoad')
if (-not $loadDocumentMatch.Success -or
    $loadDocumentMatch.Value -match 'Start_WorldPreviewFromBeginning\(\)' -or
	$loadDocumentMatch.Value -match 'Stage_WorldPreview\(' -or
	$loadDocumentMatch.Value -notmatch 'Release_WorldPreview\(true\)' -or
	$loadDocumentMatch.Value -notmatch 'm_bPreviewVisibleRequested = false' -or
    $effectToolSource -notmatch 'Load = inspect saved data without autoplay' -or
	$effectToolSource -notmatch 'GPU resources are deferred until an explicit preview scope is played' -or
	$effectToolSource -notmatch 'bLivePreview = bDrawable &&\s*m_bPreviewVisibleRequested' -or
    $startPreviewMatch.Value -notmatch 'pObject->Set_Visible\(\s*bRootResolved && Is_ProductCueVisible\(m_fPreviewTimeSeconds\)\)') {
    throw 'Loading an Effect must not stage GPU resources; only explicit Complete, Group, or Solo play may create the character-pivot preview.'
}
$sessionBarMatch = [regex]::Match(
    $effectToolSource,
    'void Client::CEffect_Tool::Render_AuthoringSessionBar\(\)[\s\S]*?void Client::CEffect_Tool::Render_EffectDetailWindow')
if (-not $sessionBarMatch.Success -or
    $sessionBarMatch.Value -match 'Validate_Drawable' -or
    $sessionBarMatch.Value -notmatch 'm_bActiveDocumentDrawable' -or
    $effectToolHeader -notmatch 'm_strActiveDocumentDrawableError') {
    throw 'Effect Detail must consume the mutation-time drawable cache instead of validating the complete document every frame.'
}
$allEffectsRefreshMatch = [regex]::Match(
    $effectToolSource,
    'bool_t Client::CEffect_Tool::Refresh_AllEffects\([^)]*\)[\s\S]*?bool_t Client::CEffect_Tool::Refresh_DataFiles')
$dataFilesRefreshMatch = [regex]::Match(
    $effectToolSource,
    'bool_t Client::CEffect_Tool::Refresh_DataFiles\(\)[\s\S]*?bool_t Client::CEffect_Tool::Try_SelectProductCue')
if (-not $allEffectsRefreshMatch.Success -or
    -not $dataFilesRefreshMatch.Success -or
    $allEffectsRefreshMatch.Value -match 'CEffectDocumentCodec::Load' -or
    $dataFilesRefreshMatch.Value -match 'CEffectDocumentCodec::Load' -or
    $effectToolSource -notmatch 'Try_DeriveEffectAssetIdFromFilename' -or
    $effectToolSource -notmatch 'Ensure_PlayerSkillCatalog' -or
    $effectToolSource -notmatch 'Refresh_AllEffects\(true\)') {
    throw 'Initial Effect Tool indexes must not parse every authored/imported Effect JSON document.'
}
foreach ($scopeName in @(
    'EffectTool.InitialIndexStep',
    'EffectTool.AuthoringWindow',
    'EffectTool.ModelViewWindow',
    'EffectTool.DetailWindow',
    'EffectTool.AllEffectsWindow',
    'EffectTool.DataFilesWindow',
    'EffectTool.ThumbnailTrim',
    'EffectTool.DocumentLoad.Parse',
    'EffectTool.DocumentLoad.ValidateDrawable',
    'EffectTool.DocumentLoad.CanonicalBaseline')) {
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
if ($authoringHeader -notmatch 'EFFECT_AUTHORING_FORMAT_VERSION = 13u' -or
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
	$effectToolHeader -notmatch 'SOLO_STANDALONE_MESHES' -or
	$effectToolHeader -notmatch 'SOLO_MESH_EMITTERS' -or
	$effectToolHeader -notmatch 'SOLO_STANDALONE_SPRITES' -or
	$effectToolHeader -notmatch 'SOLO_SPRITE_EMITTERS' -or
    $effectToolSource -notmatch 'Try_SelectParticleSystem' -or
    $effectToolSource -notmatch 'Apply Particle System' -or
    $effectToolSource -notmatch 'Audition Particle System' -or
	$effectToolSource -notmatch 'Play Mesh Particles' -or
	$effectToolSource -notmatch 'Mesh Particle-only preview committed' -or
	$effectToolSource -notmatch 'Standalone Mesh-only preview committed' -or
	$effectToolSource -notmatch 'Standalone Sprite-only preview committed' -or
	$effectToolSource -notmatch 'Sprite Particle-only preview committed' -or
	$effectToolSource -notmatch 'm_bPreviewVisibleRequested && bRootResolved' -or
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
    if ($domainId -eq 'DimensionMaster' -and
        ($models.Count -ne 140 -or $textures.Count -ne 701)) {
        throw "DimensionMaster Mesh workbench catalog changed: expected 140 WModel / 701 DDS, got $($models.Count) / $($textures.Count)"
    }
    $effectPaletteResourceCount += $models.Count + $textures.Count
}
$rendererSource = Get-Content -LiteralPath (
    Require-File 'Client\Private\Effect_DocumentRenderer.cpp') `
    -Raw -Encoding UTF8
$playbackSource = Get-Content -LiteralPath (
    Require-File 'Client\Private\Effect_Playback.cpp') `
    -Raw -Encoding UTF8
if ($rendererSource -match
        'case Client::EFFECT_RENDERER_TYPE::END:[\s\S]{0,500}?if \(Element\.SourceRecipe\.bEnabled\)' -or
    $playbackSource -match
        'case Client::EFFECT_RENDERER_TYPE::END:[\s\S]{0,500}?if \(Element\.SourceRecipe\.bEnabled\)') {
    throw 'Legacy source-recipe Elements without native-v14 renderer identity must retain their kind-derived GPU occurrence family.'
}
$legacySourceRecipeDocumentCount = 0
$legacySourceRecipeGpuOccurrenceCount = 0
$authoredRoot = Join-Path $repoRoot 'Data\Effects\Authored'
Get-ChildItem -LiteralPath $authoredRoot -Recurse -Filter '*.effect.json' -File |
    ForEach-Object {
        $document = Get-Content -LiteralPath $_.FullName -Raw -Encoding UTF8 |
            ConvertFrom-Json
        $documentCount = 0
        foreach ($element in @($document.elements)) {
            $sourceRecipe = $element.sourceRecipe
            $renderer = $element.renderer
            $rendererType = if ($null -eq $renderer) { '' } else { [string]$renderer.type }
            if ($null -ne $sourceRecipe -and [bool]$sourceRecipe.enabled -and
                [string]::IsNullOrWhiteSpace($rendererType) -and
                @('particle', 'decal') -contains [string]$element.kind) {
                ++$documentCount
            }
        }
        if ($documentCount -gt 0) {
            ++$legacySourceRecipeDocumentCount
            $legacySourceRecipeGpuOccurrenceCount += $documentCount
        }
    }
if ($legacySourceRecipeDocumentCount -ne 18 -or
    $legacySourceRecipeGpuOccurrenceCount -ne 1300) {
    throw "Legacy source-recipe/native-renderer denominator changed: documents=$legacySourceRecipeDocumentCount gpuOccurrences=$legacySourceRecipeGpuOccurrenceCount"
}
if ($rendererSource -notmatch 'fSequenceTerm\) \+\s+Element\.Detail\.UV\.iTileIndex' -or
    $effectToolSource -notmatch 'Active\.Detail\.UV\.iTileIndex = iTile') {
    throw 'UV tile clicks must set the authored sequence offset and refresh preview.'
}
if ($rendererSource -match 'unordered_map<const EFFECT_ELEMENT_DESC\*' -or
    $rendererSource -notmatch 'for \(const EFFECT_ELEMENT_DESC& DocumentElement : m_Document\.Elements\)' -or
    $rendererSource -notmatch 'iAfterImageBegin' -or
    $rendererSource -notmatch 'iParticleBegin' -or
    $rendererSource -notmatch 'iTrailBegin' -or
    $rendererSource -notmatch '\.subspan\(iParticleBegin' -or
    $rendererSource -notmatch '\.subspan\(iTrailBegin' -or
    $rendererSource -notmatch '\.subspan\(iAfterImageBegin') {
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
$productRollout = Read-JsonFile `
    'Data\Effects\AuthoredCorrections\Generated\FourClassCombat.authored-product-rollout.json'
if ([string]$productRollout.schema -cne
        'lostark.four-class-authored-product-rollout' -or
    [int]$productRollout.version -ne 2 -or
    [int]$productRollout.summary.visualClipOccurrenceCount -ne 102 -or
    [int]$productRollout.summary.silentClipOccurrenceCount -ne 11 -or
    [int]$productRollout.summary.derivedClipTargetCount -ne 48 -or
    [int]$productRollout.summary.retainedStageTargetCount -ne 53 -or
    [int]$productRollout.summary.productTargetCount -ne 101 -or
    [int]$productRollout.summary.productCueCount -ne 101) {
    throw 'Four-class Authored Product rollout receipt header/counts are invalid.'
}
$expectedCatalogIds = @($productRollout.productTargets | ForEach-Object {
    [string]$_.effectAssetId
} | Sort-Object -Unique)
$expectedEffectCount = $expectedCatalogIds.Count
if ($expectedEffectCount -ne 101) {
    throw "Four-class Authored Product rollout target set is invalid: expected=101 actual=$expectedEffectCount"
}
$canonicalR = @($boundSkillRows | Where-Object {
    [string]$_.inputSlot -ceq 'R'
})
$canonicalA = @($boundSkillRows | Where-Object {
    [string]$_.inputSlot -ceq 'A'
})
$canonicalD = @($boundSkillRows | Where-Object {
    [string]$_.inputSlot -ceq 'D'
})
if ($canonicalR.Count -ne 1 -or [int64]$canonicalR[0].skillId -ne 2050180 -or
    $canonicalA.Count -ne 1 -or [int64]$canonicalA[0].skillId -ne 2050210 -or
    $canonicalD.Count -ne 1 -or [int64]$canonicalD[0].skillId -ne 2050240 -or
    $expectedEffectIds -ccontains 'effect.dimensionmaster.skill.2050190' -or
    $expectedEffectIds -ccontains 'effect.dimensionmaster.skill.2050550') {
    throw 'DimensionMaster canonical R/A/D roster regressed or a stale candidate was admitted.'
}

$catalog = Read-JsonFile 'Data\Effects\EffectCatalog.json'
$artistDebugEffectId = 'effect.artist.skill.31470'
$expectedCatalogCount = $expectedEffectCount + 1
if ($catalog.formatVersion -ne 1 -or
    $catalog.effects.Count -ne $expectedCatalogCount) {
    throw "EffectCatalog.json must contain the admitted Product set plus the one non-Product Artist debug target: expected=$expectedCatalogCount actual=$($catalog.effects.Count)"
}
$catalogIds = @($catalog.effects | ForEach-Object { [string]$_.effectAssetId })
if (@($catalogIds | Sort-Object -Unique).Count -ne $expectedCatalogCount) {
    throw 'EffectCatalog.json contains a duplicate EffectAssetId.'
}
foreach ($effectId in $expectedCatalogIds) {
    if ($catalogIds -cnotcontains $effectId) {
        throw "EffectCatalog.json is missing $effectId"
    }
}
$extraCatalogIds = @($catalogIds | Where-Object {
    $expectedCatalogIds -cnotcontains $_
})
if ($extraCatalogIds.Count -ne 1 -or
    $extraCatalogIds[0] -cne $artistDebugEffectId) {
    throw 'EffectCatalog.json may add only the exact non-Product Artist 31470 debug target outside the admitted Product rollout.'
}

$authoredBaselineCorrectionPath =
    'Data\Effects\AuthoredCorrections\DimensionMaster\effect.dimensionmaster.skill.2050210.authored-baseline.correction.json'
$authoredBaselineCorrection = Read-JsonFile $authoredBaselineCorrectionPath
$canonicalAEffectPath =
    'Data\Effects\Authored\effect.dimensionmaster.skill.2050210.effect.json'
$canonicalAEffectSha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath (
    Require-File $canonicalAEffectPath)).Hash.ToLowerInvariant()
if ($authoredBaselineCorrection.schema -cne 'lostark.effect-authored-correction' -or
    [int]$authoredBaselineCorrection.version -ne 2 -or
    [string]$authoredBaselineCorrection.correctionId -cne
        'dimensionmaster.skill.2050210.authored-baseline' -or
    [string]$authoredBaselineCorrection.sourceEffectAssetId -cne
        'effect.dimensionmaster.skill.2050210' -or
    [string]$authoredBaselineCorrection.sourceEffectSha256 -cne
        $canonicalAEffectSha256 -or
    [string]$authoredBaselineCorrection.targetEffectAssetId -cne
        'effect.dimensionmaster.skill.2050210.authored-baseline' -or
    [double]$authoredBaselineCorrection.authoredSpriteBillboardRollDegrees -ne -90.0 -or
    [string]$authoredBaselineCorrection.playerCue.anchorSlotId -cne 'root' -or
    [string]$authoredBaselineCorrection.playerCue.outerFollowPolicy -cne 'follow' -or
    [string]$authoredBaselineCorrection.playerCue.elementFollowPolicy -cne 'snapshot' -or
    @($authoredBaselineCorrection.hitSnapshots).Count -ne 4 -or
    @($authoredBaselineCorrection.layers).Count -ne 6) {
    throw 'DimensionMaster A Authored Baseline correction contract is invalid.'
}
$expectedAuthoredHitIds = @('hit01', 'hit02', 'hit03', 'hit04')
$expectedAuthoredHitSuffixes = @(
    '',
    '.event_source-event-030',
    '.event_source-event-045',
    '.event_source-event-060'
)
$expectedAuthoredHitTimes = @(0.25, 0.60, 0.90, 1.30)
$expectedAuthoredLayerRoles = @(
    'white-echo', 'flow', 'body', 'afterimage', 'rim', 'sprite'
)
for ($index = 0; $index -lt $expectedAuthoredHitIds.Count; ++$index) {
    $hit = $authoredBaselineCorrection.hitSnapshots[$index]
    if ([string]$hit.id -cne $expectedAuthoredHitIds[$index] -or
        [string]$hit.sourceEventSuffix -cne $expectedAuthoredHitSuffixes[$index] -or
        [double]$hit.sourceTimeSeconds -ne $expectedAuthoredHitTimes[$index] -or
        [double]$hit.transform.position[0] -ge 0.0 -or
        [double]$hit.transform.position[2] -le 0.0) {
        throw "DimensionMaster A Authored Baseline hit snapshot is invalid at index $index."
    }
}
if ((@($authoredBaselineCorrection.layers | ForEach-Object {
    [string]$_.role
}) -join ',') -cne ($expectedAuthoredLayerRoles -join ',') -or
    @($authoredBaselineCorrection.layers | Where-Object {
        [string]$_.sourceElementBaseId -notmatch
            '^fx_pc_swp_00\.par_j_swp_willowrend_swinghit_00_1\.particlespriteemitter_(2|14|15|20|3|9)$' -or
        [string]$_.emissiveFromSlot -notin @('base', 'mask')
    }).Count -ne 0) {
    throw 'DimensionMaster A Authored Baseline layer order or source material seed is invalid.'
}

$authoredBaselineEntry = @($catalog.effects | Where-Object {
    [string]$_.effectAssetId -ceq
        'effect.dimensionmaster.skill.2050210.authored-baseline'
})
if ($authoredBaselineEntry.Count -ne 1 -or
    [string]$authoredBaselineEntry[0].authoringPath -cne
        'Effects/Authored/effect.dimensionmaster.skill.2050210.authored-baseline.effect.json') {
    throw 'DimensionMaster A Authored Baseline catalog routing is invalid.'
}
$authoredBaselineDocument = Read-JsonFile `
    'Data\Effects\Authored\effect.dimensionmaster.skill.2050210.authored-baseline.effect.json'
$authoredBaselineElements = @($authoredBaselineDocument.elements)
$authoredBaselineKinds = @($authoredBaselineElements | ForEach-Object {
    [string]$_.kind
})
if ($authoredBaselineDocument.schema -cne 'lostark.effect-authoring' -or
    [int]$authoredBaselineDocument.version -ne 12 -or
    $authoredBaselineDocument.effectAssetId -cne
        'effect.dimensionmaster.skill.2050210.authored-baseline' -or
    $authoredBaselineElements.Count -ne 24 -or
    @($authoredBaselineKinds | Where-Object { $_ -ceq 'mesh' }).Count -ne 20 -or
    @($authoredBaselineKinds | Where-Object { $_ -ceq 'sprite' }).Count -ne 4 -or
    @($authoredBaselineKinds | Where-Object { $_ -ceq 'particle' }).Count -ne 0 -or
    @($authoredBaselineElements | Where-Object {
        [bool]$_.sourceRecipe.enabled -or
        [bool]$_.sourcePresentation.enabled -or
        -not [bool]$_.actionCueAttachment.enabled -or
        [bool]$_.actionCueAttachment.follow -or
        [string]$_.actionCueAttachment.sourceAnchorSlotId -cne 'root' -or
        [string]$_.actionCueAttachment.runtimeAnchorSlotId -cne 'root' -or
        -not [bool]$_.material.sourceProfile.enabled -or
        [string]::IsNullOrWhiteSpace([string]$_.material.sourceMaterialPath) -or
        @($_.resources | Where-Object { [string]$_.slotId -ceq 'emissive' }).Count -ne 1
    }).Count -ne 0 -or
    [double]$authoredBaselineDocument.particleSystem.yawOffsetDegrees -ne 0.0) {
    throw 'DimensionMaster A Authored Baseline must be twenty Meshes and four Sprites with preserved material profiles on root snapshots.'
}
$expectedAuthoredGroups = [ordered]@{
    'authored.baseline.a.hit01' = 0.25
    'authored.baseline.a.hit02' = 0.60
    'authored.baseline.a.hit03' = 0.90
    'authored.baseline.a.hit04' = 1.30
}
foreach ($groupId in $expectedAuthoredGroups.Keys) {
    $groupElements = @($authoredBaselineElements | Where-Object {
        [string]$_.groupId -ceq $groupId
    })
    if ($groupElements.Count -ne 6 -or
        (@($groupElements | ForEach-Object { [string]$_.id }) -join ',') -cne
            (@($expectedAuthoredLayerRoles | ForEach-Object {
                "$groupId.$_"
            }) -join ',') -or
        (@($groupElements | ForEach-Object { [string]$_.kind }) -join ',') -cne
            'mesh,mesh,mesh,mesh,mesh,sprite' -or
        @($groupElements | Where-Object {
            [double]$_.detail.timing.startDelaySeconds -ne
                [double]$expectedAuthoredGroups[$groupId]
        }).Count -ne 0) {
        throw "DimensionMaster A Authored Baseline group contract is invalid: $groupId"
    }
    $sprite = @($groupElements | Where-Object { [string]$_.kind -ceq 'sprite' })
    if ($sprite.Count -ne 1 -or
        -not [bool]$sprite[0].detail.sprite.billboard -or
        [double]$sprite[0].detail.sprite.billboardRollDegrees -ne -90.0) {
        throw "DimensionMaster A Authored Baseline Sprite roll is invalid: $groupId"
    }
}

$dimensionMasterEvents = Get-Content -LiteralPath (
    Require-File 'Data\Animation\Authored\DimensionMaster\DimensionMaster.animevents'
) -Raw -Encoding UTF8
if ($dimensionMasterEvents -notmatch
    '"pc_sp_m_00_sk_sk_willowrend" EFFECT startms=0 payload="effect\.dimensionmaster\.skill\.2050210\.authored-baseline" effectref=asset anchor="root" follow=follow stop=natural') {
    throw 'DimensionMaster A Authored Baseline cue must feed live root to per-occurrence snapshots.'
}

$documentResourceIds = [Collections.Generic.HashSet[string]]::new(
    [StringComparer]::Ordinal)
foreach ($entry in $catalog.effects) {
	if ([string]$entry.effectAssetId -ceq $artistDebugEffectId) {
		if ([string]$entry.payloadKind -cne
				'IMMUTABLE_RECONSTRUCTED_RUNTIME_PROGRAM' -or
			-not [string]::IsNullOrWhiteSpace([string]$entry.authoringPath) -or
			[string]$entry.reconstructedRuntimeProgramPath -cne
				'Effects/Imported/Artist/Candidates/skill.31470.reconstructed-runtime-program.candidate.json' -or
			[string]$entry.reconstructedRenderResourceAuthorityPath -cne
				'Effects/Imported/Artist/Materials/skill.31470.reconstructed-render-resource-authority.receipt.json') {
			throw 'Artist 31470 debug Catalog entry is not the exact immutable reconstructed payload.'
		}
		continue
	}
    $relativeDocument = [string]$entry.authoringPath
    if ($relativeDocument -notmatch '^Effects/Authored/[a-z0-9._-]+\.effect\.json$') {
        throw "Unsafe catalog document path: $relativeDocument"
    }
    $document = Read-JsonFile (Join-Path 'Data' $relativeDocument)
    if ($document.version -notin @(5, 6, 7, 8, 9, 10, 11, 12) -or
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
if ($completeT.version -notin @(10, 11, 12) -or
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

$completeD = Read-JsonFile `
    'Data\Effects\Authored\effect.dimensionmaster.skill.2050240.effect.json'
$dParticles = @($completeD.elements | Where-Object {
    [string]$_.kind -ceq 'particle'
})
$dLights = @($completeD.elements | Where-Object {
    [string]$_.kind -ceq 'light'
})
$dScreenPosts = @($completeD.elements | Where-Object {
    [string]$_.kind -ceq 'screenPost'
})
$dRuntimeExactProfiles = @($dParticles | Where-Object {
    [string]$_.material.sourceProfile.semanticStatus -ceq 'runtime_exact'
})
$dReconstructedProfileGroups = @($dParticles | Where-Object {
    [string]$_.material.sourceProfile.semanticStatus -ceq 'reconstructed_profile'
} | ForEach-Object {
    [string]$_.material.sourceProfile.profileId
} | Sort-Object -Unique)
if (@($completeD.elements).Count -ne 51 -or $dParticles.Count -ne 46 -or
    $dLights.Count -ne 2 -or $dScreenPosts.Count -ne 3 -or
    $dRuntimeExactProfiles.Count -ne 0 -or
    $dReconstructedProfileGroups.Count -ne 21) {
    throw 'DimensionMaster D 2050240 execution coverage/material restoration boundary changed.'
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
    # formatVersion 3 lets a clips element be that stage's own clip array.
    $bindingClips = @()
    foreach ($element in @($binding.clips)) {
        if ($element -is [Array]) { $bindingClips += @($element) }
        else { $bindingClips += $element }
    }
    foreach ($clipValue in $bindingClips) {
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
        '^"(?<Clip>[^"]+)" EFFECT .* payload="(?<Effect>effect\.dimensionmaster\.skill\.(?<Skill>[0-9]+)(?:(?:\.ba(?<Stage>[0-9]+))|(?:\.authored-baseline))?(?:\.clip(?<ClipStage>[0-9]+))?)" effectref=asset ')
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
    $_ -match ' payload="effect\.dimensionmaster\.skill\.2050500\.authored-baseline" ' -and
    $_ -match ' anchor="root" follow=follow stop=natural '
})
if ($tCue.Count -ne 1) {
    throw 'DimensionMaster T 2050500 must start its Authored Product once with dimensionprison at the Player root.'
}
$dCue = @($admittedRows | Where-Object {
    $_ -match '^"pc_sp_m_00_sk_sk_telekinesisthrust_01" EFFECT startms=0 ' -and
    $_ -match ' payload="effect\.dimensionmaster\.skill\.2050240\.authored-baseline\.clip1" ' -and
    $_ -match ' anchor="root" follow=follow stop=natural '
})
if ($dCue.Count -ne 1) {
    throw 'DimensionMaster D 2050240 must start its Authored Product once at the Player root.'
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
    # A stage owns its own clip array; its Effect cue rides the first clip.
    if ($clipValue -is [Array]) { $clipValue = @($clipValue)[0] }
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

Write-Host ('PASS: final Effect Tool bundle; code={0}, documents={1}, resources={2}, palette={3}, cues={4}; D Particle=46, typed Light/Post=2/3, runtime-exact material=0, reconstructed groups=21.' -f
    $requiredCode.Count, $expectedEffectCount, $documentResourceIds.Count,
    $effectPaletteResourceCount, $admittedRows.Count)
