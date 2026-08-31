[CmdletBinding()]
param(
    [string]$ExtractionRoot,
    [string]$OutputDirectory
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$repoRoot = [System.IO.Path]::GetFullPath(
    [System.IO.Path]::Combine($PSScriptRoot, '..', '..'))
if ([string]::IsNullOrWhiteSpace($ExtractionRoot)) {
    $ExtractionRoot = [System.IO.Path]::Combine(
        $repoRoot, 'out', 'CharacterEquipmentExtraction')
}
if ([string]::IsNullOrWhiteSpace($OutputDirectory)) {
    $OutputDirectory = [System.IO.Path]::Combine(
        $ExtractionRoot, 'Inventory', 'Generated')
}

$extractionRootPath = [System.IO.Path]::GetFullPath($ExtractionRoot)
$outputDirectoryPath = [System.IO.Path]::GetFullPath($OutputDirectory)
if (-not (Test-Path -LiteralPath $extractionRootPath -PathType Container)) {
    throw "Extraction root does not exist: $extractionRootPath"
}
[System.IO.Directory]::CreateDirectory($outputDirectoryPath) | Out-Null

$utf8NoBom = New-Object System.Text.UTF8Encoding($false)

function Write-Utf8NoBom {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,
        [Parameter(Mandatory = $true)]
        [string]$Content
    )

    [System.IO.File]::WriteAllText($Path, $Content, $utf8NoBom)
}

function Convert-ToForwardSlash {
    param([Parameter(Mandatory = $true)][string]$Path)
    return $Path.Replace([System.IO.Path]::DirectorySeparatorChar, '/')
}

function Get-RelativePath {
    param(
        [Parameter(Mandatory = $true)][string]$BasePath,
        [Parameter(Mandatory = $true)][string]$TargetPath
    )

    $baseUri = [System.Uri]::new(
        ([System.IO.Path]::GetFullPath($BasePath).TrimEnd(
            [System.IO.Path]::DirectorySeparatorChar) +
        [System.IO.Path]::DirectorySeparatorChar))
    $targetUri = [System.Uri]::new([System.IO.Path]::GetFullPath($TargetPath))
    return [System.Uri]::UnescapeDataString(
        $baseUri.MakeRelativeUri($targetUri).ToString())
}

function Get-WeaponRole {
    param([Parameter(Mandatory = $true)][string]$ObjectName)

    $name = $ObjectName.ToLowerInvariant()
    if ($name -match '(^|_)sd([0-9_-]|$)' -or $name -match '_shield') {
        return 'WEAPON_SHIELD'
    }
    if ($name -match '(?:^|[_0-9-])h[0-9-]*_sk$') { return 'WEAPON_H' }
    if ($name -match '(?:^|[_0-9-])l[0-9-]*_sk$') { return 'WEAPON_L' }
    if ($name -match '(?:^|[_0-9-])s[0-9-]*_sk$') { return 'WEAPON_S' }
    if ($name -match '(?:^|[_0-9-])p[0-9-]*_sk$') { return 'WEAPON_P' }
    if ($name -match '(?:^|[_0-9-])e[0-9-]*_sk$') { return 'WEAPON_E' }
    return 'WEAPON_MAIN'
}

function Get-MemberKeyHint {
    param([Parameter(Mandatory = $true)][string]$ObjectName)

    $member = $ObjectName.ToLowerInvariant()
    $member = $member -replace '_sk$', ''
    $member = $member -replace '_(loc_int|usa|chn|kor|jpn|rus)$', ''
    $member = $member -replace '_high$', ''
    return $member
}

function Get-MeshClassification {
    param(
        [Parameter(Mandatory = $true)][string]$PackageName,
        [Parameter(Mandatory = $true)][string]$ObjectName,
        [Parameter(Mandatory = $true)][string]$PackageRelativePath
    )

    $package = $PackageName.ToLowerInvariant()
    $object = $ObjectName.ToLowerInvariant()
    $path = $PackageRelativePath.ToLowerInvariant().Replace('\', '/')
    $qualityTier = if ($path -match '(^|/)mesh_high/' -or
        $object -match '(^|_)high(_|$)') { 'HIGH' } else { 'DEFAULT' }
    $regionVariant = 'DEFAULT'
    foreach ($candidate in @('loc_int', 'chn', 'usa', 'kor', 'jpn', 'rus')) {
        if ($object -match "(^|_)$candidate(_|$)") {
            $regionVariant = $candidate.ToUpperInvariant()
            break
        }
    }

    $role = 'UNKNOWN'
    $slotHint = 'NONE'
    $confidence = 'LOW'
    $readiness = 'MANUAL_REVIEW'
    $saveKind = 'NONE'
    $note = '이름만으로 부품 책임을 확정할 수 없어 수동 메시/재질 검토가 필요하다.'

    $knownBasePackage = $package -match '^pc_(ft|gn_f|wr_f|wr|sp|sp_m)_00$'
    $isQuarantinedName = $package -match '(^|_)(test|creation)($|_)' -or
        $object -match '(^|_)(test|hairtest|old|bk)(_|$)'

    if ($isQuarantinedName) {
        $role = 'QUARANTINED_VARIANT'
        $confidence = 'HIGH'
        $readiness = 'DO_NOT_AUTO_ADMIT'
        $note = 'TEST/CREATION/old/backup 계열은 원본 인벤토리에는 남기되 제품 카탈로그에 자동 승격하지 않는다.'
    }
    elseif ($package.StartsWith('wp_')) {
        $role = Get-WeaponRole -ObjectName $object
        $slotHint = 'WEAPON'
        $confidence = 'HIGH'
        $readiness = 'COOK_AND_SOCKET_PROFILE_REQUIRED'
        $saveKind = 'EQUIPMENT_VISUAL_SET'
        $note = '동일 source package의 무기 조각을 한 visual set으로 묶고 class별 socket/stance metadata를 검증해야 한다.'
    }
    elseif (($package -eq 'pc_swp_m_00' -and $object -match 'pc_swp_fx') -or
        $object -match '(^|_)(fx[0-9]*|shadow|grab|dummy|helper|hairtest)(_|$)') {
        $role = 'FX_HELPER'
        $confidence = 'HIGH'
        $readiness = 'DO_NOT_ADMIT'
        $note = '제품 장비 메시가 아닌 FX/그림자/보조 리그다.'
    }
    elseif ($knownBasePackage) {
        $role = 'BASE_BODY_COMPONENT'
        $slotHint = 'BASE_BODY'
        $confidence = 'HIGH'
        $readiness = 'MASTER_SKELETON_REFERENCE_ONLY'
        $saveKind = 'CHARACTER_BASE_BODY'
        $note = '공용 직업군의 기본 body/기본 외형 구성이다. raid reward가 아니라 class master skeleton과 기본 loadout의 기준으로 저장한다.'
    }
    elseif ($package -match '_face($|_)' -or $object -match '(^|_)face[0-9]*(_|$)') {
        $role = 'FACE_APPEARANCE'
        $slotHint = 'FACE'
        $confidence = 'HIGH'
        $readiness = 'MASTER_SKELETON_NORMALIZATION_REQUIRED'
        $saveKind = 'APPEARANCE_PRESET_PART'
        $note = '장비 드롭이 아니라 캐릭터 외형 preset 후보로 분리 저장한다.'
    }
    elseif ($package -match '_hair($|_)' -or
        $object -match '(^|_)(hair[0-9]*|hairdeco[0-9]*)(_|$)') {
        $role = 'HAIR_APPEARANCE'
        $slotHint = 'HAIR'
        $confidence = 'HIGH'
        $readiness = 'MASTER_SKELETON_NORMALIZATION_REQUIRED'
        $saveKind = 'APPEARANCE_PRESET_PART'
        $note = '장비 helmet과 충돌 정책을 가진 외형 preset 후보로 분리 저장한다.'
    }
    elseif ($object -match '(^|_)(head[0-9-]*|haed[0-9-]*)(_|$)') {
        $role = 'HEAD_OR_HAIR'
        $slotHint = 'HEAD_OR_HAIR'
        $confidence = 'MEDIUM'
        $readiness = 'MANUAL_HEAD_COVERAGE_REVIEW_REQUIRED'
        $saveKind = 'APPEARANCE_OR_EQUIPMENT_PART'
        $note = 'head 이름만으로 helmet과 hairstyle/deco를 구분하지 않는다. visual set coverage와 hair hide 정책을 저작해야 한다.'
    }
    elseif ($object -match '(^|_)helmet[0-9-]*(_|$)') {
        $role = 'APPAREL_HEAD'
        $slotHint = 'HEAD'
        $confidence = 'HIGH'
        $readiness = 'MASTER_SKELETON_NORMALIZATION_REQUIRED'
        $saveKind = 'EQUIPMENT_VISUAL_SET'
        $note = '머리 장비 파츠 후보이며 hair visibility/coverage 검증이 필요하다.'
    }
    elseif ($object -match '(^|_)(shoulder[0-9-]*|shouder[0-9-]*)(_|$)') {
        $role = 'APPAREL_SHOULDER'
        $slotHint = 'SHOULDER'
        $confidence = 'HIGH'
        $readiness = 'MASTER_SKELETON_NORMALIZATION_REQUIRED'
        $saveKind = 'EQUIPMENT_VISUAL_SET'
        $note = '어깨 장비 파츠 후보.'
    }
    elseif ($object -match '(^|_)(arm[0-9]*|glove[0-9]*)(_|$)') {
        $role = 'APPAREL_HANDS'
        $slotHint = 'HANDS'
        $confidence = 'HIGH'
        $readiness = 'MASTER_SKELETON_NORMALIZATION_REQUIRED'
        $saveKind = 'EQUIPMENT_VISUAL_SET'
        $note = '팔/장갑 장비 파츠 후보.'
    }
    elseif ($object -match '(^|_)lower[0-9]*(_|$)') {
        $role = 'APPAREL_LOWER'
        $slotHint = 'LOWER'
        $confidence = 'HIGH'
        $readiness = 'MASTER_SKELETON_NORMALIZATION_REQUIRED'
        $saveKind = 'EQUIPMENT_VISUAL_SET'
        $note = '하의 장비 파츠 후보.'
    }
    elseif ($object -match '(^|_)upper[0-9]*(_|$)') {
        $role = 'APPAREL_UPPER'
        $slotHint = 'UPPER'
        $confidence = 'HIGH'
        $readiness = 'MASTER_SKELETON_NORMALIZATION_REQUIRED'
        $saveKind = 'EQUIPMENT_VISUAL_SET'
        $note = '상의 장비 파츠 후보.'
    }
    elseif ($object -match '(^|_)(dress[0-9]*|robe[0-9]*|skirt[0-9]*|cape[0-9]*|cloak[0-9]*)(_|$)' -or
        $object -match '(^|_)high(_|$)') {
        $role = 'APPAREL_COMBINED'
        $slotHint = 'UPPER_LOWER_SET'
        $confidence = 'MEDIUM'
        $readiness = 'MASTER_SKELETON_NORMALIZATION_REQUIRED'
        $saveKind = 'EQUIPMENT_VISUAL_SET'
        $note = '한 메시가 여러 신체 영역을 덮을 수 있으므로 coverage를 visual set에 명시해야 한다.'
    }
    else {
        $packageStem = $package.Replace('-', '_')
        $objectStem = $object.Replace('-', '_')
        if ($package -match 'basebody' -or $object -match '(^|_)basebody(_|$)' -or
            $object -match '(^|_)body(_|$)') {
            $role = 'BASE_BODY'
            $slotHint = 'BASE_BODY'
            $confidence = 'HIGH'
            $readiness = 'MASTER_SKELETON_REFERENCE_ONLY'
            $saveKind = 'CHARACTER_BASE_BODY'
            $note = '장비 아이템이 아니라 이 클래스 파츠를 정규화할 master skeleton/base body 후보.'
        }
        elseif ($package -match '_(av|hr)_' -or $package -match '^pc_.+_[0-9]{2,3}[a-z0-9-]*$') {
            $role = 'APPAREL_COMBINED'
            $slotHint = 'REVIEW_COVERAGE'
            $confidence = 'LOW'
            $readiness = 'MANUAL_COVERAGE_REVIEW_REQUIRED'
            $saveKind = 'EQUIPMENT_VISUAL_SET'
            $note = '의상 package 안의 비표준 이름 메시다. 렌더 확인 없이 특정 슬롯으로 확정하지 않는다.'
        }
    }

    return [pscustomobject][ordered]@{
        partRole = $role
        slotHint = $slotHint
        classificationConfidence = $confidence
        qualityTier = $qualityTier
        regionVariant = $regionVariant
        runtimeReadiness = $readiness
        plannedSaveKind = $saveKind
        note = $note
    }
}

$scopeDefinitions = @(
    [pscustomobject]@{ classId = 'LANCE_MASTER'; className = '창술사'; root = 'Raw/LanceMaster'; packagePattern = '^(PC_FLM|WP_WFLM)'; sourceFamily = 'CLASS_OR_WEAPON' },
    [pscustomobject]@{ classId = 'LANCE_MASTER'; className = '창술사'; root = 'RawShared/PC_FT'; packagePattern = '^PC_FT(?!_M)'; sourceFamily = 'SHARED_BASE' },
    [pscustomobject]@{ classId = 'GUNSLINGER'; className = '건슬링어'; root = 'Raw/Gunslinger'; packagePattern = '^(PC_GDH_F|WP_WGDH)'; sourceFamily = 'CLASS_OR_WEAPON' },
    [pscustomobject]@{ classId = 'GUNSLINGER'; className = '건슬링어'; root = 'RawShared/PC_GN_F'; packagePattern = '^PC_GN_F'; sourceFamily = 'SHARED_BASE' },
    [pscustomobject]@{ classId = 'SLAYER'; className = '슬레이어'; root = 'Raw/Slayer'; packagePattern = '^(PC_WBK_F|WP_WWBK)'; sourceFamily = 'CLASS_OR_WEAPON' },
    [pscustomobject]@{ classId = 'SLAYER'; className = '슬레이어'; root = 'RawShared/PC_WR'; packagePattern = '^PC_WR_F'; sourceFamily = 'SHARED_BASE' },
    [pscustomobject]@{ classId = 'ARTIST'; className = '도화가'; root = 'Raw/Artist'; packagePattern = '^(PC_SDM|WP_WSDM)'; sourceFamily = 'CLASS_OR_WEAPON' },
    [pscustomobject]@{ classId = 'ARTIST'; className = '도화가'; root = 'RawShared/PC_SP'; packagePattern = '^PC_SP(?!_M)'; sourceFamily = 'SHARED_BASE' },
    [pscustomobject]@{ classId = 'WARLORD'; className = '워로드'; root = 'Raw/Warlord'; packagePattern = '^(PC_WGL|WP_WWGL)'; sourceFamily = 'CLASS_OR_WEAPON' },
    [pscustomobject]@{ classId = 'WARLORD'; className = '워로드'; root = 'RawShared/PC_WR'; packagePattern = '^PC_WR(?!_F)'; sourceFamily = 'SHARED_BASE' },
    [pscustomobject]@{ classId = 'DIMENSIONMASTER'; className = '차원술사'; root = 'Raw/DimensionMaster'; packagePattern = '^(PC_SP_M|PC_SWP_M|WP_WSWP_M)'; sourceFamily = 'CLASS_SHARED_AND_WEAPON' }
)

$entriesByKey = @{}
foreach ($scope in $scopeDefinitions) {
    $scopeRoot = [System.IO.Path]::Combine(
        $extractionRootPath, $scope.root.Replace('/', [System.IO.Path]::DirectorySeparatorChar))
    if (-not (Test-Path -LiteralPath $scopeRoot -PathType Container)) {
        throw "Required extraction scope is missing: $scopeRoot"
    }

    foreach ($file in (Get-ChildItem -LiteralPath $scopeRoot -Recurse -Filter '*.gltf' -File)) {
        $relativeToScope = Get-RelativePath -BasePath $scopeRoot -TargetPath $file.FullName
        $segments = $relativeToScope -split '/'
        if ($segments.Count -lt 2) { continue }
        $packageName = $segments[0]
        if ($packageName -notmatch $scope.packagePattern) { continue }

        $objectName = [System.IO.Path]::GetFileNameWithoutExtension($file.Name)
        $classification = Get-MeshClassification `
            -PackageName $packageName `
            -ObjectName $objectName `
            -PackageRelativePath $relativeToScope
        $key = "$($scope.classId)|$packageName|$relativeToScope"
        if ($entriesByKey.ContainsKey($key)) { continue }

        $entriesByKey[$key] = [pscustomobject][ordered]@{
            classId = $scope.classId
            className = $scope.className
            visualSetId = "character.$($scope.classId.ToLowerInvariant()).$($packageName.ToLowerInvariant())"
            sourceFamily = $scope.sourceFamily
            packageName = $packageName
            objectName = $objectName
            memberKeyHint = Get-MemberKeyHint -ObjectName $objectName
            partRole = $classification.partRole
            slotHint = $classification.slotHint
            classificationConfidence = $classification.classificationConfidence
            qualityTier = $classification.qualityTier
            regionVariant = $classification.regionVariant
            extractionState = 'EXTRACTED'
            runtimeReadiness = $classification.runtimeReadiness
            plannedSaveKind = $classification.plannedSaveKind
            sourcePath = Convert-ToForwardSlash (
                Get-RelativePath -BasePath $repoRoot -TargetPath $file.FullName)
            failureReason = ''
            note = $classification.note
        }
    }
}

$blockedObjects = @(
    [pscustomobject]@{ classId = 'LANCE_MASTER'; className = '창술사'; packageName = 'PC_FT_AV_247'; objectName = 'pc_ft_av_247_dress_sk' },
    [pscustomobject]@{ classId = 'LANCE_MASTER'; className = '창술사'; packageName = 'PC_FT_AV_247'; objectName = 'pc_ft_av_247_head_sk' },
    [pscustomobject]@{ classId = 'SLAYER'; className = '슬레이어'; packageName = 'PC_WR_F_AV_220A'; objectName = 'pc_wr_f_av_220a_face2_sk' },
    [pscustomobject]@{ classId = 'SLAYER'; className = '슬레이어'; packageName = 'PC_WR_F_AV_220A'; objectName = 'pc_wr_f_av_220a_head_sk' },
    [pscustomobject]@{ classId = 'SLAYER'; className = '슬레이어'; packageName = 'PC_WR_F_AV_220A'; objectName = 'pc_wr_f_av_220a_lower_sk' },
    [pscustomobject]@{ classId = 'SLAYER'; className = '슬레이어'; packageName = 'PC_WR_F_AV_220A'; objectName = 'pc_wr_f_av_220a_upper_sk' },
    [pscustomobject]@{ classId = 'SLAYER'; className = '슬레이어'; packageName = 'PC_WR_F_AV_220B'; objectName = 'pc_wr_f_av_220b_face2_sk' },
    [pscustomobject]@{ classId = 'SLAYER'; className = '슬레이어'; packageName = 'PC_WR_F_AV_220B'; objectName = 'pc_wr_f_av_220b_head_sk' },
    [pscustomobject]@{ classId = 'SLAYER'; className = '슬레이어'; packageName = 'PC_WR_F_AV_220B'; objectName = 'pc_wr_f_av_220b_lower_sk' },
    [pscustomobject]@{ classId = 'SLAYER'; className = '슬레이어'; packageName = 'PC_WR_F_AV_220B'; objectName = 'pc_wr_f_av_220b_upper_sk' },
    [pscustomobject]@{ classId = 'SLAYER'; className = '슬레이어'; packageName = 'PC_WR_F_AV_220B'; objectName = 'pc_wr_f_av_220b-1_head_sk' }
)

foreach ($blocked in $blockedObjects) {
    $classification = Get-MeshClassification `
        -PackageName $blocked.packageName `
        -ObjectName $blocked.objectName `
        -PackageRelativePath "mesh/$($blocked.objectName).gltf"
    $logFamily = if ($blocked.classId -eq 'LANCE_MASTER') { 'PC_FT' } else { 'PC_WR' }
    $logPath = [System.IO.Path]::Combine(
        $extractionRootPath, 'ObjectRetries', $logFamily,
        "$($blocked.objectName).stderr.log")
    $entriesByKey["$($blocked.classId)|$($blocked.packageName)|BLOCKED|$($blocked.objectName)"] =
        [pscustomobject][ordered]@{
            classId = $blocked.classId
            className = $blocked.className
            visualSetId = "character.$($blocked.classId.ToLowerInvariant()).$($blocked.packageName.ToLowerInvariant())"
            sourceFamily = 'SHARED_BASE'
            packageName = $blocked.packageName
            objectName = $blocked.objectName
            memberKeyHint = Get-MemberKeyHint -ObjectName $blocked.objectName
            partRole = $classification.partRole
            slotHint = $classification.slotHint
            classificationConfidence = $classification.classificationConfidence
            qualityTier = $classification.qualityTier
            regionVariant = $classification.regionVariant
            extractionState = 'BLOCKED_SERIALIZATION'
            runtimeReadiness = 'BLOCKED_EXTRACTION'
            plannedSaveKind = $classification.plannedSaveKind
            sourcePath = ''
            failureReason = 'UModel LostArk v7: Serializing behind stopper'
            note = if (Test-Path -LiteralPath $logPath) {
                "실패 로그: $(Convert-ToForwardSlash (Get-RelativePath -BasePath $repoRoot -TargetPath $logPath))"
            } else {
                '격리 재시도에서도 같은 직렬화 오류가 발생했다.'
            }
        }
}

$entries = @($entriesByKey.Values | Sort-Object classId, packageName, objectName, sourcePath)
$expectedCounts = [ordered]@{
    LANCE_MASTER = 1518
    GUNSLINGER = 1408
    SLAYER = 1220
    ARTIST = 1361
    WARLORD = 1514
    DIMENSIONMASTER = 454
}
foreach ($classId in $expectedCounts.Keys) {
    $actual = @($entries | Where-Object classId -eq $classId).Count
    if ($actual -ne $expectedCounts[$classId]) {
        throw "Source denominator mismatch for ${classId}: expected $($expectedCounts[$classId]), got $actual"
    }
}
if ($entries.Count -ne 7475) {
    throw "Total source denominator mismatch: expected 7475, got $($entries.Count)"
}
$extractedCount = @($entries | Where-Object extractionState -eq 'EXTRACTED').Count
$blockedCount = @($entries | Where-Object extractionState -eq 'BLOCKED_SERIALIZATION').Count
if ($extractedCount -ne 7464 -or $blockedCount -ne 11) {
    throw "Coverage mismatch: expected 7464 extracted / 11 blocked, got $extractedCount / $blockedCount"
}

function Get-GroupReadiness {
    param([Parameter(Mandatory = $true)][object[]]$GroupEntries)

    $states = @($GroupEntries.runtimeReadiness | Sort-Object -Unique)
    if ($states -contains 'BLOCKED_EXTRACTION') {
        if (@($GroupEntries | Where-Object extractionState -eq 'EXTRACTED').Count -gt 0) {
            return 'PARTIAL_BLOCKED_EXTRACTION'
        }
        return 'BLOCKED_EXTRACTION'
    }
    if ($states -contains 'MANUAL_REVIEW' -or
        $states -contains 'MANUAL_COVERAGE_REVIEW_REQUIRED' -or
        $states -contains 'MANUAL_HEAD_COVERAGE_REVIEW_REQUIRED') {
        return 'MANUAL_REVIEW'
    }
    if (@($GroupEntries | Where-Object partRole -like 'WEAPON_*').Count -gt 0) {
        return 'COOK_AND_SOCKET_PROFILE_REQUIRED'
    }
    if (@($GroupEntries | Where-Object partRole -like 'APPAREL_*').Count -gt 0) {
        return 'MASTER_SKELETON_NORMALIZATION_REQUIRED'
    }
    if (@($GroupEntries | Where-Object partRole -in @(
        'HAIR_APPEARANCE', 'FACE_APPEARANCE')).Count -gt 0) {
        return 'MASTER_SKELETON_NORMALIZATION_REQUIRED'
    }
    if (@($GroupEntries | Where-Object partRole -in @(
        'BASE_BODY', 'BASE_BODY_COMPONENT')).Count -gt 0) {
        return 'MASTER_SKELETON_REFERENCE_ONLY'
    }
    return 'DO_NOT_ADMIT'
}

$visualSets = @(
    $entries | Group-Object classId, visualSetId | ForEach-Object {
        $groupEntries = @($_.Group)
        $roles = @($groupEntries.partRole | Sort-Object -Unique)
        $slotHints = @($groupEntries.slotHint | Where-Object { $_ -ne 'NONE' } | Sort-Object -Unique)
        $saveKinds = @($groupEntries.plannedSaveKind | Where-Object { $_ -ne 'NONE' } | Sort-Object -Unique)
        [pscustomobject][ordered]@{
            classId = $groupEntries[0].classId
            className = $groupEntries[0].className
            visualSetId = $groupEntries[0].visualSetId
            packageName = $groupEntries[0].packageName
            sourceObjectCount = $groupEntries.Count
            extractedObjectCount = @($groupEntries | Where-Object extractionState -eq 'EXTRACTED').Count
            blockedObjectCount = @($groupEntries | Where-Object extractionState -eq 'BLOCKED_SERIALIZATION').Count
            partRoles = $roles
            slotHints = $slotHints
            plannedSaveKinds = $saveKinds
            runtimeReadiness = Get-GroupReadiness -GroupEntries $groupEntries
        }
    } | Sort-Object classId, packageName
)

$generatedAt = [System.DateTimeOffset]::UtcNow.ToString('o')
$inventoryDocument = [ordered]@{
    schema = 'lostark.character-equipment-source-inventory'
    formatVersion = 1
    generatedAtUtc = $generatedAt
    sourceRoot = 'out/CharacterEquipmentExtraction'
    sourceDenominator = 7475
    extractedCount = $extractedCount
    blockedCount = $blockedCount
    runtimeAdmittedCount = 0
    warning = 'Raw glTF is not Resources-ready. Skinned parts require class master-skeleton normalization; weapons require cook plus socket/stance metadata.'
    entries = $entries
}
$visualSetDocument = [ordered]@{
    schema = 'lostark.character-equipment-source-visual-sets'
    formatVersion = 1
    generatedAtUtc = $generatedAt
    sourceObjectCount = $entries.Count
    visualSetCount = $visualSets.Count
    runtimeAdmittedCount = 0
    visualSets = $visualSets
}
$receiptDocument = [ordered]@{
    schema = 'lostark.character-equipment-extraction-receipt'
    formatVersion = 1
    generatedAtUtc = $generatedAt
    source = [ordered]@{
        gamePackageRoot = 'C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/Packages'
        packageCount = 2191
        skeletalMeshCount = 7475
        texture2DCount = 10951
        compressedSourceGiB = 6.34
    }
    extractor = [ordered]@{
        name = 'UEViewer LostArk v7'
        executableSha256 = 'B9573CDCBB7E9D26DBF60A0E3AF47FB5AF8543140873DA8483C26D58CF40B249'
        archiveSha256 = '97EE9E856B0F678E84E115889DFD50A9B515110F3814B8CC0A0DCF8FB65A5878'
    }
    coverage = [ordered]@{
        selectedSkeletalMeshes = 7475
        extractedSkeletalMeshes = $extractedCount
        blockedSkeletalMeshes = $blockedCount
        percent = [math]::Round(($extractedCount * 100.0) / 7475.0, 4)
    }
    classes = @(
        foreach ($classId in $expectedCounts.Keys) {
            $classEntries = @($entries | Where-Object classId -eq $classId)
            [ordered]@{
                classId = $classId
                className = $classEntries[0].className
                selected = $classEntries.Count
                extracted = @($classEntries | Where-Object extractionState -eq 'EXTRACTED').Count
                blocked = @($classEntries | Where-Object extractionState -eq 'BLOCKED_SERIALIZATION').Count
            }
        }
    )
    outputs = [ordered]@{
        rawAndDependencyFileCount = 93063
        rawAndDependencyGltfCount = 7863
        rawAndDependencyBytes = 30793114057
        note = 'Raw output includes dependency exports; the selected class source denominator is the filtered 7475-entry inventory.'
    }
    recoveryAudit = [ordered]@{
        path = 'out/CharacterEquipmentExtraction/BlockedRecoveryAudit_20260901'
        blockedObjectCount = 11
        attemptsPerObject = 4
        totalAttempts = 44
        recoveredObjectCount = 0
        modes = @('minimal-gltf', 'minimal-psk', 'minimal-md5mesh', 'physical-upk-direct')
        terminal = 'USkeletalMesh3::Serialize -> FStaticLODModel3 -> Serializing behind stopper'
        sourcePackagesMatchInstallManifest = $true
    }
    resourcesAdmission = [ordered]@{
        admittedWmodelCount = 0
        reason = 'Direct glTF cook bone palettes do not match the current class body palettes. No unsafe raw or direct-cook asset was copied into Client/Bin/Resources.'
    }
    blockedObjects = @($entries | Where-Object extractionState -eq 'BLOCKED_SERIALIZATION' | Select-Object classId, packageName, objectName, failureReason, note)
}

$inventoryJsonPath = [System.IO.Path]::Combine(
    $outputDirectoryPath, 'character-equipment-source-inventory.json')
$inventoryCsvPath = [System.IO.Path]::Combine(
    $outputDirectoryPath, 'character-equipment-source-inventory.csv')
$visualSetJsonPath = [System.IO.Path]::Combine(
    $outputDirectoryPath, 'character-equipment-visual-sets.json')
$summaryMarkdownPath = [System.IO.Path]::Combine(
    $outputDirectoryPath, 'character-equipment-source-inventory.md')
$receiptPath = [System.IO.Path]::Combine(
    $extractionRootPath, 'character-equipment-extraction.receipt.json')

Write-Utf8NoBom -Path $inventoryJsonPath -Content (
    $inventoryDocument | ConvertTo-Json -Depth 8)
Write-Utf8NoBom -Path $visualSetJsonPath -Content (
    $visualSetDocument | ConvertTo-Json -Depth 8)
Write-Utf8NoBom -Path $receiptPath -Content (
    $receiptDocument | ConvertTo-Json -Depth 8)
$csvText = ($entries | ConvertTo-Csv -NoTypeInformation) -join [Environment]::NewLine
Write-Utf8NoBom -Path $inventoryCsvPath -Content $csvText

$markdown = [System.Text.StringBuilder]::new()
[void]$markdown.AppendLine('# 6개 클래스 장비·외형 원본 인벤토리')
[void]$markdown.AppendLine()
[void]$markdown.AppendLine('이 문서는 추출된 SkeletalMesh glTF와 직렬화 실패 객체를 source package 단위로 정리한 자동 생성 보고서다.')
[void]$markdown.AppendLine('`Resources`에 바로 넣을 수 있다는 뜻은 아니다. 현재 runtime admission은 0개이며, 의상은 클래스 master skeleton 정규화, 무기는 socket/stance metadata와 WModel cook을 통과해야 한다.')
[void]$markdown.AppendLine()
[void]$markdown.AppendLine('## 전체 요약')
[void]$markdown.AppendLine()
[void]$markdown.AppendLine('| 클래스 | 원본 분모 | 추출 | 차단 | visual set | 의상 후보 | 외형 후보 | 무기 후보 | 수동 검토 | 제외 |')
[void]$markdown.AppendLine('|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|')
foreach ($classId in $expectedCounts.Keys) {
    $classEntries = @($entries | Where-Object classId -eq $classId)
    $classSets = @($visualSets | Where-Object classId -eq $classId)
    $apparel = @($classEntries | Where-Object partRole -like 'APPAREL_*').Count
    $appearance = @($classEntries | Where-Object partRole -in @(
        'HAIR_APPEARANCE', 'HEAD_OR_HAIR', 'FACE_APPEARANCE',
        'BASE_BODY', 'BASE_BODY_COMPONENT')).Count
    $weapons = @($classEntries | Where-Object partRole -like 'WEAPON_*').Count
    $review = @($classEntries | Where-Object runtimeReadiness -in @(
        'MANUAL_REVIEW', 'MANUAL_COVERAGE_REVIEW_REQUIRED',
        'MANUAL_HEAD_COVERAGE_REVIEW_REQUIRED')).Count
    $excluded = @($classEntries | Where-Object runtimeReadiness -in @(
        'DO_NOT_ADMIT', 'DO_NOT_AUTO_ADMIT')).Count
    [void]$markdown.AppendLine(
        "| $($classEntries[0].className) ($classId) | $($classEntries.Count) | " +
        "$(@($classEntries | Where-Object extractionState -eq 'EXTRACTED').Count) | " +
        "$(@($classEntries | Where-Object extractionState -eq 'BLOCKED_SERIALIZATION').Count) | " +
        "$($classSets.Count) | $apparel | $appearance | $weapons | $review | $excluded |")
}
[void]$markdown.AppendLine()
[void]$markdown.AppendLine('## 상태 해석')
[void]$markdown.AppendLine()
[void]$markdown.AppendLine('- `MASTER_SKELETON_NORMALIZATION_REQUIRED`: 교체·저장 후보지만 bone name/order와 inverse bind를 현재 클래스 body 기준으로 정규화한 뒤에만 승격할 수 있다.')
[void]$markdown.AppendLine('- `COOK_AND_SOCKET_PROFILE_REQUIRED`: 동일 package의 무기 조각을 한 세트로 cook하고 socket/stance visibility를 저장해야 한다.')
[void]$markdown.AppendLine('- `MASTER_SKELETON_REFERENCE_ONLY`: 장비가 아니라 정규화 기준 body다.')
[void]$markdown.AppendLine('- `MANUAL_REVIEW`: 이름만으로 coverage/부품 책임을 확정할 수 없다.')
[void]$markdown.AppendLine('- `DO_NOT_ADMIT`: FX, shadow, grab, helper rig이므로 장비 카탈로그에서 제외한다.')
[void]$markdown.AppendLine('- `DO_NOT_AUTO_ADMIT`: TEST, CREATION, old, backup 이름은 원본에는 보존하지만 제품 후보에서 격리한다.')
[void]$markdown.AppendLine('- `PARTIAL_BLOCKED_EXTRACTION`: package 일부 객체가 UModel 직렬화 오류로 빠져 있어 완전한 visual set으로 저장할 수 없다.')
[void]$markdown.AppendLine()

foreach ($classId in $expectedCounts.Keys) {
    $classSets = @($visualSets | Where-Object classId -eq $classId)
    if ($classSets.Count -eq 0) { continue }
    [void]$markdown.AppendLine("## $($classSets[0].className) ($classId)")
    [void]$markdown.AppendLine()
    [void]$markdown.AppendLine('| source package / visual set | 객체 | 추출/차단 | 파츠 역할 | 저장 슬롯 힌트 | 다음 상태 |')
    [void]$markdown.AppendLine('|---|---:|---:|---|---|---|')
    foreach ($set in $classSets) {
        $roles = ($set.partRoles -join ', ').Replace('|', '\|')
        $slots = ($set.slotHints -join ', ').Replace('|', '\|')
        [void]$markdown.AppendLine(
            "| $($set.packageName) / $($set.visualSetId) | $($set.sourceObjectCount) | " +
            "$($set.extractedObjectCount)/$($set.blockedObjectCount) | $roles | $slots | $($set.runtimeReadiness) |")
    }
    [void]$markdown.AppendLine()
}

[void]$markdown.AppendLine('## 직렬화 차단 객체')
[void]$markdown.AppendLine()
[void]$markdown.AppendLine('| 클래스 | package | object | 상태 |')
[void]$markdown.AppendLine('|---|---|---|---|')
foreach ($entry in ($entries | Where-Object extractionState -eq 'BLOCKED_SERIALIZATION')) {
    [void]$markdown.AppendLine(
        "| $($entry.className) | $($entry.packageName) | $($entry.objectName) | $($entry.extractionState) |")
}
[void]$markdown.AppendLine()
[void]$markdown.AppendLine('객체 단위 전체 7,475행은 `character-equipment-source-inventory.csv`와 `.json`, package/visual-set 저장 후보는 `character-equipment-visual-sets.json`을 사용한다.')
Write-Utf8NoBom -Path $summaryMarkdownPath -Content $markdown.ToString()

[pscustomobject]@{
    SourceObjects = $entries.Count
    Extracted = $extractedCount
    Blocked = $blockedCount
    VisualSets = $visualSets.Count
    InventoryJson = $inventoryJsonPath
    InventoryCsv = $inventoryCsvPath
    VisualSetsJson = $visualSetJsonPath
    SummaryMarkdown = $summaryMarkdownPath
    Receipt = $receiptPath
}
