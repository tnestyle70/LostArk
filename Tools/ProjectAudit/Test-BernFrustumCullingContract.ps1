param()

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path

function Read-RequiredFile([string]$relativePath) {
    $path = Join-Path $root $relativePath
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Missing required file: $relativePath"
    }
    return Get-Content -LiteralPath $path -Raw
}

function Require-Text(
    [string]$content,
    [string]$pattern,
    [string]$message) {
    if ($content -notmatch $pattern) {
        throw $message
    }
}

$registry = Read-RequiredFile 'Client\Private\LevelRegistry.cpp'
$mapUtils = Read-RequiredFile 'Client\Private\MapAssetRenderUtils.cpp'
$mapObject = Read-RequiredFile 'Client\Private\MapAssetObject.cpp'
$mapBatch = Read-RequiredFile 'Client\Private\MapStaticBatchObject.cpp'
$model = Read-RequiredFile 'Engine\Private\Model.cpp'

$bernScopeMatch = [regex]::Match(
    $registry,
    'MAP_LOAD_SCOPE\s+MakeBernMapScope\(\)\s*\{(?<body>[\s\S]*?)return\s+scope\s*;\s*\}')
if (-not $bernScopeMatch.Success) {
    throw 'Could not isolate MakeBernMapScope for exact policy validation.'
}
$bernScope = $bernScopeMatch.Groups['body'].Value

Require-Text $bernScope 'frustumCulling\.bypass\s*=\s*false\s*;' `
    'Bern product frustum culling is not enabled.'
Require-Text $bernScope 'frustumCulling\.diagnostics\s*=\s*false\s*;' `
    'Bern product diagnostics are still enabled.'
Require-Text $bernScope 'frustumCulling\.baseMargin\s*=\s*0\.25f\s*;' `
    'Bern must keep the validated 0.25 base margin.'
Require-Text $bernScope 'frustumCulling\.largeObjectRadiusThreshold\s*=\s*4\.f\s*;' `
    'Bern must keep the validated 4.0 large-object threshold.'
Require-Text $bernScope 'frustumCulling\.largeObjectAbsoluteMargin\s*=\s*2\.f\s*;' `
    'Bern must keep the validated 2.0 absolute large-object margin.'
Require-Text $bernScope 'frustumCulling\.largeObjectRelativeMargin\s*=\s*0\.12f\s*;' `
    'Bern must keep the validated 0.12 relative large-object margin.'
Require-Text $bernScope 'frustumCulling\.rejectHysteresisFrames\s*=\s*3u\s*;' `
    'Bern must keep the validated three-frame reject grace.'
Require-Text $mapUtils 'planeDistances\[index\]' `
    'Six-plane diagnostic distances are not recorded.'
Require-Text $mapUtils 'placementId=' `
    'Placement IDs are missing from the diagnostic record.'
Require-Text $mapUtils 'baseRadius=' `
    'Base radius is missing from diagnostics.'
Require-Text $mapUtils 'margin=' `
    'Conservative margin is missing from diagnostics.'
Require-Text $mapUtils 'effectiveRadius=' `
    'Effective radius is missing from diagnostics.'
Require-Text $mapUtils 'Bind_CameraCullSnapshot' `
    'The shared cull/render camera snapshot binder is missing.'
Require-Text $mapObject 'HRESULT CMapAssetObject::Render\(\)[\s\S]*Capture_CameraCullSnapshot[\s\S]*Evaluate_FrustumVisibility' `
    'Fallback placements are not culled at final render time.'
Require-Text $mapBatch 'HRESULT CMapStaticBatchObject::Render\(\)[\s\S]*Capture_CameraCullSnapshot[\s\S]*Upload_VisibleInstances' `
    'Static batches are not culled at final render time.'
Require-Text $mapUtils '"landscape"\s*==\s*assetGroupId' `
    'Landscape conservative classification is missing.'
Require-Text $mapUtils 'largeObjectRadiusThreshold' `
    'Large floor and bridge geometry classification is missing.'
Require-Text $mapUtils 'rejectGraceFrames' `
    'Frustum rejection hysteresis is missing.'
Require-Text $model 'for \(const VTXMESH& vertex : mesh\.vertices\)[\s\S]*Include_LocalPosition' `
    'Binary model culling bounds are not derived from all decoded vertices.'

Write-Host 'Bern frustum culling contract: PASS'
