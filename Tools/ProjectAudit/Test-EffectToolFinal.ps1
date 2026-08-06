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
    $effectToolSource -notmatch 'if \(m_bDetailDraftDirty\)' -or
    $effectToolSource -notmatch 'Apply Detail or Revert Detail before saving') {
    throw 'Effect Create/Load/Discard/Save must preserve unsaved Document and Detail work.'
}
if ($effectToolSource -notmatch 'Loaded editable draft; preview is hidden' -or
    $effectToolSource -notmatch 'Load rejected; active Document and preview were preserved' -or
    $effectToolSource -notmatch 'Apply Detail or Revert Detail before selecting another Element' -or
    $effectToolSource -notmatch 'Apply Detail or Revert Detail before adding an Element' -or
    $effectToolSource -notmatch 'Apply Detail or Revert Detail before deleting an Element' -or
    $effectToolSource -notmatch 'Apply Detail or Revert Detail before clearing Elements') {
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
if ($effectToolSource -notmatch 'bRuntimeMatchesActive' -or
    $effectToolSource -notmatch 'CEffectDocumentCodec::Serialize\(\*pRuntimeDocument\)' -or
    $effectToolSource -notmatch '!Has_UnsavedWork\(\)') {
    throw 'Animation cue transfer must use the exact clean document admitted in the runtime Effect catalog.'
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

$expectedSkillIds = @(
    2050010, 2050110, 2050150, 2050220, 2050190, 2050240,
    2050210, 2050200, 2050500, 2050510, 2050540
)
# The subset the current quick-slot roster binds. An authored Effect document
# outlives the slot it was written for, so the documents, the catalog and the
# animation cues above still cover all eleven; only the PlayerSkills join is
# limited to the skills a loadout can actually cast today.
$boundSkillIds = @(
    2050010, 2050210, 2050220, 2050240, 2050500, 2050540
)
$expectedEffectIds = @($expectedSkillIds | ForEach-Object {
    "effect.dimensionmaster.skill.$_"
})

$catalog = Read-JsonFile 'Data\Effects\EffectCatalog.json'
if ($catalog.formatVersion -ne 1 -or $catalog.effects.Count -ne 11) {
    throw 'EffectCatalog.json must be formatVersion 1 with exactly 11 entries.'
}
$catalogIds = @($catalog.effects | ForEach-Object { [string]$_.effectAssetId })
if (@($catalogIds | Sort-Object -Unique).Count -ne 11) {
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
    if ($document.version -notin @(5, 6) -or
        $document.effectAssetId -cne $entry.effectAssetId -or
        @($document.elements).Count -eq 0) {
        throw "Effect document header/elements are invalid: $relativeDocument"
    }
    $elementIds = @($document.elements | ForEach-Object { [string]$_.id })
    if (@($elementIds | Sort-Object -Unique).Count -ne $elementIds.Count) {
        throw "Duplicate Element ID in $relativeDocument"
    }
    foreach ($element in $document.elements) {
        if ($document.version -eq 6 -and
            ($element.displayName -isnot [string] -or
             $element.groupId -isnot [string] -or
             $element.sourceNode -isnot [string] -or
             $element.visible -isnot [bool] -or
             [string]$element.material.templateId -cne 'effect.standard')) {
            throw "Effect v6 Element metadata/template is invalid: $relativeDocument"
        }
        if (@('mesh','sprite','particle','decal','trail') -cnotcontains
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
}

$skills = Read-JsonFile 'Data\Balance\PlayerSkills.json'
foreach ($skillId in $boundSkillIds) {
    $matches = @($skills.skills | Where-Object {
        [int64]$_.skillId -eq $skillId -and
        [string]$_.characterClass -eq 'DIMENSIONMASTER'
    })
    if ($matches.Count -ne 1) {
        throw "Expected one DimensionMaster PlayerSkills row for $skillId"
    }
    $expectedEffectId = "effect.dimensionmaster.skill.$skillId"
    if ([string]$matches[0].effectId -cne $expectedEffectId) {
        throw "PlayerSkills.effectId mismatch for $skillId"
    }
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
if ($admittedRows.Count -ne 14) {
    throw "Expected 14 admitted animation Effect cues, got $($admittedRows.Count)."
}
foreach ($effectId in $expectedEffectIds) {
    $expectedCueCount = if ($effectId -eq
        'effect.dimensionmaster.skill.2050010') { 4 } else { 1 }
    if (@($admittedRows | Where-Object {
        $_ -match ('payload="' + [regex]::Escape($effectId) + '"')
    }).Count -ne $expectedCueCount) {
        throw "Expected $expectedCueCount admitted animation cues for $effectId"
    }
}
$comboCueTimes = [ordered]@{
    'pc_sp_m_00_sk_att_battle_1_01' = 100
    'pc_sp_m_00_sk_att_battle_1_02' = 43
    'pc_sp_m_00_sk_att_battle_1_03' = 28
    'pc_sp_m_00_sk_att_battle_1_04' = 335
}
foreach ($comboClip in $comboCueTimes.Keys) {
    if (@($admittedRows | Where-Object {
        $_ -match ('^"' + [regex]::Escape($comboClip) + '" ') -and
        $_ -match (' EFFECT startms=' + $comboCueTimes[$comboClip] + ' ') -and
        $_ -match 'payload="effect\.dimensionmaster\.skill\.2050010"'
    }).Count -ne 1) {
        throw "Missing DimensionMaster combo Effect cue: $comboClip"
    }
}

Write-Host ('PASS: final Effect Tool bundle; code={0}, documents=11, resources={1}, palette={2}, cues=14.' -f
    $requiredCode.Count, $documentResourceIds.Count, $effectPaletteResourceCount)
