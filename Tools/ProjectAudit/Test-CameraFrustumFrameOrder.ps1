[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path

function Read-RepositorySource {
    param(
        [Parameter(Mandatory = $true)]
        [string]$RelativePath
    )

    $path = Join-Path $repoRoot $RelativePath
    if (-not (Test-Path -LiteralPath $path)) {
        throw "[CameraFrustumFrameOrder] Missing source file: $RelativePath"
    }

    return Get-Content -LiteralPath $path -Raw
}

function Assert-SourceContains {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Source,
        [Parameter(Mandatory = $true)]
        [string]$Expected,
        [Parameter(Mandatory = $true)]
        [string]$FailureMessage
    )

    if (-not $Source.Contains($Expected)) {
        throw "[CameraFrustumFrameOrder] $FailureMessage"
    }
}

$gameInstance = Read-RepositorySource 'Engine/Private/GameInstance.cpp'
$cameraFree = Read-RepositorySource 'Client/Private/Camera_Free.cpp'
$camera = Read-RepositorySource 'Engine/Private/Camera.cpp'
$levelBern = Read-RepositorySource 'Client/Private/Level_Bern.cpp'
$mapStaticBatch = Read-RepositorySource 'Client/Private/MapStaticBatchObject.cpp'
$mapAsset = Read-RepositorySource 'Client/Private/MapAssetObject.cpp'
$objectManagerHeader = Read-RepositorySource 'Engine/Public/Object_Manager.h'
$objectManager = Read-RepositorySource 'Engine/Private/Object_Manager.cpp'
$mapPlacementRuntime = Read-RepositorySource 'Client/Private/MapPlacementRuntime.cpp'

$updateEngineMatch = [regex]::Match(
    $gameInstance,
    '(?s)void\s+CGameInstance::Update_Engine\s*\([^)]*\)\s*\{(?<body>.*?)\r?\n\}\r?\n\r?\nvoid\s+CGameInstance::Refresh_CameraState')
if (-not $updateEngineMatch.Success) {
    throw '[CameraFrustumFrameOrder] Could not isolate CGameInstance::Update_Engine.'
}

$updateBody = $updateEngineMatch.Groups['body'].Value
$postPhysicsIndex = $updateBody.IndexOf(
    'm_pObject_Manager->Post_Physics_Update(fTimeDelta);',
    [System.StringComparison]::Ordinal)
$levelIndex = $updateBody.IndexOf(
    'm_pLevel_Manager->Update(fTimeDelta);',
    [System.StringComparison]::Ordinal)
$lateIndex = $updateBody.IndexOf(
    'm_pObject_Manager->Late_Update(fTimeDelta);',
    [System.StringComparison]::Ordinal)

if ($postPhysicsIndex -lt 0 -or
    $levelIndex -lt 0 -or
    $lateIndex -lt 0 -or
    $postPhysicsIndex -ge $levelIndex -or
    $levelIndex -ge $lateIndex) {
    throw (
        '[CameraFrustumFrameOrder] Required frame order is ' +
        'Post_Physics_Update -> Level_Update -> Late_Update.')
}

$bernUpdateMatch = [regex]::Match(
    $levelBern,
    '(?s)void\s+CLevel_Bern::Update\s*\([^)]*\)\s*\{(?<body>.*?)\r?\n\}')
if (-not $bernUpdateMatch.Success) {
    throw '[CameraFrustumFrameOrder] Could not isolate CLevel_Bern::Update.'
}

$bernUpdateBody = $bernUpdateMatch.Groups['body'].Value
$replicationIndex = $bernUpdateBody.IndexOf(
    'm_Replication.Update()',
    [System.StringComparison]::Ordinal)
$bindIndex = $bernUpdateBody.IndexOf(
    'Bind_CameraToLocalCharacter()',
    [System.StringComparison]::Ordinal)
if ($replicationIndex -lt 0 -or
    $bindIndex -lt 0 -or
    $replicationIndex -ge $bindIndex) {
    throw (
        '[CameraFrustumFrameOrder] Bern must commit replication before ' +
        'binding the follow camera.')
}

$cameraLateUpdateMatch = [regex]::Match(
    $cameraFree,
    '(?s)void\s+CCamera_Free::Late_Update\s*\([^)]*\)\s*\{(?<body>.*?)\r?\n\}')
if (-not $cameraLateUpdateMatch.Success) {
    throw '[CameraFrustumFrameOrder] Could not isolate CCamera_Free::Late_Update.'
}

$cameraLateUpdateBody = $cameraLateUpdateMatch.Groups['body'].Value
$followIndex = $cameraLateUpdateBody.IndexOf(
    'Update_FollowCamera(fTimeDelta);',
    [System.StringComparison]::Ordinal)
$pipelineIndex = $cameraLateUpdateBody.IndexOf(
    '__super::Update_PipeLine();',
    [System.StringComparison]::Ordinal)
if ($followIndex -lt 0 -or
    $pipelineIndex -lt 0 -or
    $followIndex -ge $pipelineIndex) {
    throw (
        '[CameraFrustumFrameOrder] Follow camera must update before ' +
        'publishing the view pipeline.')
}

Assert-SourceContains $camera `
    'CGameInstance::Get().Refresh_CameraState();' `
    'CCamera::Update_PipeLine must refresh inverse matrices and the frustum.'
Assert-SourceContains $mapStaticBatch `
    'const HRESULT hVisibleResult = Upload_VisibleInstances();' `
    'Map static batches must build their visible instance list in Late_Update.'
Assert-SourceContains $mapStaticBatch `
    'isIn_Frustum_InWorldSpace' `
    'Map static batch visibility must consume the engine frustum.'
Assert-SourceContains $mapAsset `
    'isIn_Frustum_InWorldSpace' `
    'Map fallback objects must consume the engine frustum.'
Assert-SourceContains $objectManagerHeader `
    'map<const wstring_t, shared_ptr<CLayer>>' `
    'Object layers must retain their ordered map contract.'
Assert-SourceContains $objectManager `
    'Pair.second->Late_Update(fTimeDelta);' `
    'Object manager must dispatch layer Late_Update.'
Assert-SourceContains $mapPlacementRuntime `
    'TEXT("Layer_MapStaticBatch")' `
    'Static batch layer tag changed unexpectedly.'
Assert-SourceContains $mapPlacementRuntime `
    'L"Layer_MapAsset_"' `
    'Fallback map asset layer prefix changed unexpectedly.'

if ([string]::CompareOrdinal('Layer_Camera', 'Layer_MapStaticBatch') -ge 0 -or
    [string]::CompareOrdinal('Layer_Camera', 'Layer_MapAsset_') -ge 0) {
    throw (
        '[CameraFrustumFrameOrder] Ordered layer tags no longer place the ' +
        'camera before map culling.')
}

Write-Host (
    '[CameraFrustumFrameOrder] PASS: replication and camera binding run ' +
    'before camera pipeline refresh, map culling, and render submission.')
