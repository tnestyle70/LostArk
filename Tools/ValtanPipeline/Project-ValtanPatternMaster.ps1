[CmdletBinding()]
param(
    [ValidateSet('Validate', 'Publish', 'PublishV2', 'MigrateV2Preview')]
    [string]$Mode = 'Validate',
    [string]$MasterPath = 'Data/Valtan/Valtan.pattern.json',
    [string]$ReceiptPath = '',
    [string]$V2OutputPath = '',
    [switch]$SkipProductDriftCheck,
    [switch]$WriterLockAlreadyHeld,
    [string]$WriterLockOwnerNonce = '',
    [string]$RepositoryRoot = ''
)

$ErrorActionPreference = 'Stop'
$formatPreservingHelper = Join-Path $PSScriptRoot 'Format-PreservingJsonArray.ps1'
if (-not [IO.File]::Exists($formatPreservingHelper)) {
    throw "Missing formatting-preserving JSON helper: $formatPreservingHelper"
}
. $formatPreservingHelper
$repoRoot = if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    [IO.Path]::GetFullPath($RepositoryRoot)
}
if (-not [IO.Directory]::Exists($repoRoot)) {
    throw "RepositoryRoot does not exist: $repoRoot"
}
if ($Mode -eq 'Publish') {
    throw ('Legacy Valtan.pattern.json projection Publish is retired because it cannot ' +
        'represent the canonical split decision model or rotation v3 Product. Use ' +
        'Publish-ValtanTuningRuntimeSet.ps1 SaveAuthoring/PublishCandidate instead.')
}
if ($WriterLockAlreadyHeld -and $Mode -ne 'PublishV2') {
    throw 'WriterLockAlreadyHeld is valid only for PublishV2.'
}
if ($WriterLockAlreadyHeld -and $WriterLockOwnerNonce -cnotmatch '^[0-9a-f]{32}$') {
    throw 'WriterLockAlreadyHeld requires its exact 32-digit owner nonce.'
}
if (-not $WriterLockAlreadyHeld -and
    -not [string]::IsNullOrWhiteSpace($WriterLockOwnerNonce)) {
    throw 'WriterLockOwnerNonce requires WriterLockAlreadyHeld.'
}
if ($Mode -in @('Validate', 'MigrateV2Preview')) {
    $pipeline = Join-Path $PSScriptRoot 'valtan_tuning_pipeline.py'
    if (-not [IO.File]::Exists($pipeline)) {
        throw "Missing Valtan v2 tuning pipeline: $pipeline"
    }
    $command = @($pipeline, '--repository-root', $repoRoot)
    if ($Mode -eq 'Validate') {
        $command += 'validate'
    }
    else {
        if ([string]::IsNullOrWhiteSpace($V2OutputPath)) {
            throw 'MigrateV2Preview requires -V2OutputPath. The v1 source is never overwritten.'
        }
        $resolvedV2Output = if ([IO.Path]::IsPathRooted($V2OutputPath)) {
            [IO.Path]::GetFullPath($V2OutputPath)
        }
        else {
            [IO.Path]::GetFullPath((Join-Path $repoRoot $V2OutputPath))
        }
        if ($resolvedV2Output -eq [IO.Path]::GetFullPath((Join-Path $repoRoot 'Data/Valtan/Valtan.pattern.json'))) {
            throw 'MigrateV2Preview refuses to overwrite the v1 runtime-compatible master.'
        }
        $command += @('migrate-preview', '--output', $resolvedV2Output)
    }
    & python @command
    if ($global:LASTEXITCODE -ne 0) {
        throw "Valtan v2 pipeline failed with exit code $global:LASTEXITCODE."
    }
    if ($Mode -eq 'Validate') {
        foreach ($validatorName in @(
            'validate_valtan_clip_template_parity.py',
            'validate_valtan_hit_presentation_alignment.py')) {
            $validatorPath = Join-Path $PSScriptRoot $validatorName
            if (-not [IO.File]::Exists($validatorPath)) {
                throw "Missing Valtan product validator: $validatorPath"
            }
            & python $validatorPath --repository-root $repoRoot --check
            if ($global:LASTEXITCODE -ne 0) {
                throw "$validatorName failed with exit code $global:LASTEXITCODE."
            }
        }
    }
    return
}
$stableIdPattern = '^[A-Za-z0-9_.-]{1,160}$'
$allowedMappingBases = [Collections.Generic.HashSet[string]]::new(
    [StringComparer]::Ordinal)
foreach ($mappingBasis in @(
    'CURRENT_PRODUCT_BASELINE','PATTERN_PR_REFERENCE','ANIMATION_PR_127',
    'SOURCE_REVIEWED_DELTA','PROJECT_AUTHORED','LEGACY_V1_MIGRATION')) {
    $null = $allowedMappingBases.Add($mappingBasis)
}
$allowedAnimationEndPolicies = [Collections.Generic.HashSet[string]]::new(
    [StringComparer]::Ordinal)
foreach ($endPolicy in @('EXACT','HOLD_LAST_POSE','LOOP_TO_STAGE_END')) {
    $null = $allowedAnimationEndPolicies.Add($endPolicy)
}
$utf8NoBom = [Text.UTF8Encoding]::new($false)
$utf8Strict = [Text.UTF8Encoding]::new($false, $true)
$performDriftCheck = $Mode -eq 'Validate' -and -not $SkipProductDriftCheck

function Resolve-InputPath([string]$Path, [string]$Context) {
    if ([string]::IsNullOrWhiteSpace($Path)) {
        throw "$Context path is empty."
    }
    $resolved = if ([IO.Path]::IsPathRooted($Path)) {
        [IO.Path]::GetFullPath($Path)
    }
    else {
        [IO.Path]::GetFullPath((Join-Path $repoRoot $Path))
    }
    if (-not [IO.File]::Exists($resolved)) {
        throw "Missing ${Context}: $Path"
    }
    return $resolved
}

function Resolve-RepositoryDocumentPath([string]$RelativePath, [string]$Context) {
    if ([string]::IsNullOrWhiteSpace($RelativePath) -or
        [IO.Path]::IsPathRooted($RelativePath) -or
        $RelativePath.Contains('\') -or
        @($RelativePath.Split('/')) -contains '..') {
        throw "$Context is not a repository-relative path: '$RelativePath'"
    }
    $resolved = [IO.Path]::GetFullPath((Join-Path $repoRoot $RelativePath))
    $rootPrefix = $repoRoot.TrimEnd('\', '/') + [IO.Path]::DirectorySeparatorChar
    if (-not $resolved.StartsWith($rootPrefix, [StringComparison]::OrdinalIgnoreCase) -or
        -not [IO.File]::Exists($resolved)) {
        throw "$Context does not resolve to an existing repository document: '$RelativePath'"
    }
    return $resolved
}

function Read-JsonStrict([string]$Path, [string]$Context) {
    [byte[]]$bytes = [IO.File]::ReadAllBytes($Path)
    if ($bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and
        $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF) {
        throw "$Context must be UTF-8 without BOM: $Path"
    }
    try {
        $text = $utf8Strict.GetString($bytes)
    }
    catch {
        throw "$Context is not valid UTF-8: $Path"
    }
    try {
        return $text | ConvertFrom-Json
    }
    catch {
        throw "$Context is not valid JSON: $Path`n$($_.Exception.Message)"
    }
}

function Assert-ExactProperties([object]$Value, [string[]]$Expected, [string]$Context) {
    if ($null -eq $Value) { throw "$Context is null." }
    [string[]]$actual = @($Value.PSObject.Properties.Name)
    [string[]]$expectedSorted = @($Expected)
    [Array]::Sort($actual, [StringComparer]::Ordinal)
    [Array]::Sort($expectedSorted, [StringComparer]::Ordinal)
    if (($actual -join "`n") -cne ($expectedSorted -join "`n")) {
        throw "$Context fields are invalid. expected=[$($expectedSorted -join ',')] actual=[$($actual -join ',')]"
    }
}

function Assert-StableId([object]$Value, [string]$Context, [bool]$AllowEmpty = $false) {
    if ($Value -isnot [string]) { throw "$Context must be a JSON string." }
    $text = [string]$Value
    if ($AllowEmpty -and $text.Length -eq 0) { return }
    if ($text -notmatch $stableIdPattern) {
        throw "$Context is not a stable ID: '$text'"
    }
}

function Assert-JsonInteger(
    [object]$Value,
    [string]$Context,
    [long]$Minimum = 0,
    [long]$Maximum = [long]::MaxValue) {
    if (($Value -isnot [int]) -and ($Value -isnot [long]) -and
        ($Value -isnot [uint32]) -and ($Value -isnot [uint64])) {
        throw "$Context must be a JSON integer."
    }
    $number = [long]$Value
    if ($number -lt $Minimum -or $number -gt $Maximum) {
        throw "$Context integer is out of range: $number"
    }
}

function Assert-JsonNumber(
    [object]$Value,
    [string]$Context,
    [double]$Minimum,
    [double]$Maximum) {
    if (($Value -isnot [int]) -and ($Value -isnot [long]) -and
        ($Value -isnot [uint32]) -and ($Value -isnot [uint64]) -and
        ($Value -isnot [double]) -and ($Value -isnot [decimal])) {
        throw "$Context must be a JSON number."
    }
    $number = [double]$Value
    if ([double]::IsNaN($number) -or [double]::IsInfinity($number) -or
        $number -lt $Minimum -or $number -gt $Maximum) {
        throw "$Context number is out of range: $number"
    }
}

function Assert-JsonBoolean([object]$Value, [string]$Context) {
    if ($Value -isnot [bool]) { throw "$Context must be a JSON boolean." }
}

function Assert-DisplayText([object]$Value, [string]$Context) {
    if ($Value -isnot [string] -or [string]::IsNullOrWhiteSpace([string]$Value) -or
        [Text.Encoding]::UTF8.GetByteCount([string]$Value) -gt 160) {
        throw "$Context is not bounded display text."
    }
}

function Assert-CueProjection(
    [object]$Projection,
    [object[]]$Occurrences,
    [string]$Context) {
    Assert-ExactProperties $Projection @(
        'clipOccurrenceId','sourceStartMs','sourceEndMs','mappingBasis'
    ) $Context
    Assert-StableId $Projection.clipOccurrenceId "$Context clipOccurrenceId"
    Assert-StableId $Projection.mappingBasis "$Context mappingBasis"
    if (-not $allowedMappingBases.Contains([string]$Projection.mappingBasis)) {
        throw "$Context mappingBasis is outside the public vocabulary."
    }
    Assert-JsonInteger $Projection.sourceStartMs "$Context sourceStartMs" 0 600000
    if ($null -ne $Projection.sourceEndMs) {
        Assert-JsonInteger $Projection.sourceEndMs "$Context sourceEndMs" 1 600000
        if ([int]$Projection.sourceEndMs -le [int]$Projection.sourceStartMs) {
            throw "$Context sourceEndMs must be greater than sourceStartMs."
        }
    }
    if ([string]$Projection.clipOccurrenceId -cnotin @($Occurrences | ForEach-Object {
        [string]$_.clipOccurrenceId
    })) {
        throw "$Context clipOccurrenceId is not owned by its stage."
    }
}

function ConvertTo-CanonicalNode([object]$Value) {
    if ($null -eq $Value) { return $null }
    if ($Value -is [string] -or $Value -is [bool] -or
        $Value -is [byte] -or $Value -is [sbyte] -or
        $Value -is [int16] -or $Value -is [uint16] -or
        $Value -is [int32] -or $Value -is [uint32] -or
        $Value -is [int64] -or $Value -is [uint64] -or
        $Value -is [single] -or $Value -is [double] -or
        $Value -is [decimal]) {
        return $Value
    }
    if ($Value -is [Collections.IDictionary]) {
        $ordered = [ordered]@{}
        [string[]]$keys = @($Value.Keys | ForEach-Object { [string]$_ })
        [Array]::Sort($keys, [StringComparer]::Ordinal)
        foreach ($key in $keys) {
            $ordered[$key] = ConvertTo-CanonicalNode $Value[$key]
        }
        return [pscustomobject]$ordered
    }
    if ($Value -is [Collections.IEnumerable] -and $Value -isnot [string]) {
        return @($Value | ForEach-Object { ConvertTo-CanonicalNode $_ })
    }
    $object = [ordered]@{}
    [string[]]$propertyNames = @($Value.PSObject.Properties.Name)
    [Array]::Sort($propertyNames, [StringComparer]::Ordinal)
    foreach ($propertyName in $propertyNames) {
        $object[$propertyName] = ConvertTo-CanonicalNode $Value.$propertyName
    }
    return [pscustomobject]$object
}

function ConvertTo-CanonicalJson([object]$Value) {
    return (ConvertTo-CanonicalNode $Value) | ConvertTo-Json -Depth 100 -Compress
}

function Assert-SemanticallyEqual(
    [object]$Expected,
    [object]$Actual,
    [string]$Context) {
    $expectedJson = ConvertTo-CanonicalJson $Expected
    $actualJson = ConvertTo-CanonicalJson $Actual
    if ($expectedJson -cne $actualJson) {
        throw "$Context has drifted from Data/Valtan/Valtan.pattern.json."
    }
}

function New-EncounterStageProjection([object]$Stage) {
    $projected = [ordered]@{
        stageId = [string]$Stage.stageId
        actionId = [string]$Stage.actionId
        stageKind = [string]$Stage.stageKind
        durationMs = [int]$Stage.durationMs
        hitShape = [string]$Stage.hitShape
        hitOuterRadius = [double]$Stage.hitOuterRadius
        hitInnerRadius = [double]$Stage.hitInnerRadius
        hitAngleDegrees = [double]$Stage.hitAngleDegrees
        hitLength = [double]$Stage.hitLength
        hitHalfWidth = [double]$Stage.hitHalfWidth
        hitCount = [int]$Stage.hitCount
        hitIntervalMs = [int]$Stage.hitIntervalMs
        hitDelayMs = [int]$Stage.hitDelayMs
    }
    if (@($Stage.hitOffsetsMs).Count -ne 0) {
        $projected.hitOffsetsMs = @($Stage.hitOffsetsMs | ForEach-Object { [int]$_ })
    }
    $projected.serverDamageProfileId = [string]$Stage.serverDamageProfileId
    $projected.pushRangeM = [double]$Stage.pushRangeM
    $projected.pushMs = [int]$Stage.pushMs
    $projected.knockdown = [bool]$Stage.knockdown
    $projected.downMs = [int]$Stage.downMs
    if ($null -ne $Stage.motion) { $projected.motion = $Stage.motion }
    if (@($Stage.actions).Count -ne 0) { $projected.actions = @($Stage.actions) }
    if (@($Stage.branches).Count -ne 0) { $projected.branches = @($Stage.branches) }
    return [pscustomobject]$projected
}

function New-EncounterPatternProjection([object]$Pattern) {
    $projected = [ordered]@{
        patternId = [string]$Pattern.patternId
        category = [string]$Pattern.category
        minimumPhase = [int]$Pattern.minimumPhase
        maximumPhase = [int]$Pattern.maximumPhase
        targetPolicy = [string]$Pattern.targetPolicy
        aimPolicy = [string]$Pattern.aimPolicy
        displayName = [string]$Pattern.displayName
        actionId = [string]$Pattern.actionId
        sourceActionIds = @($Pattern.sourceActionIds | ForEach-Object { [int]$_ })
        selectionMode = [string]$Pattern.selectionMode
        minimumHealthBar = [int]$Pattern.minimumHealthBar
        maximumHealthBar = [int]$Pattern.maximumHealthBar
        triggerHealthBar = [int]$Pattern.triggerHealthBar
        triggerOrder = [int]$Pattern.triggerOrder
        armorRequirement = [string]$Pattern.armorRequirement
        phaseRequirement = [string]$Pattern.phaseRequirement
        invulnerableWhileRunning = [bool]$Pattern.invulnerableWhileRunning
        selectionWeight = [int]$Pattern.selectionWeight
        maximumConsecutiveUses = [int]$Pattern.maximumConsecutiveUses
        minimumRange = [double]$Pattern.minimumRange
        maximumRange = [double]$Pattern.maximumRange
    }
    if ($null -ne $Pattern.serverMotion) { $projected.serverMotion = $Pattern.serverMotion }
    $projected.stages = @($Pattern.stages | ForEach-Object { New-EncounterStageProjection $_ })
    return [pscustomobject]$projected
}

function New-AnimationBindingProjection([object]$Stage) {
    return [pscustomobject][ordered]@{
        actionId = [string]$Stage.actionId
        clips = @($Stage.animation.occurrences | ForEach-Object {
            [pscustomobject][ordered]@{
                clipOccurrenceId = [string]$_.clipOccurrenceId
                clip = [string]$_.clip
                mappingBasis = [string]$_.mappingBasis
                sourceStartMs = [int]$_.sourceStartMs
                playMs = [int]$_.playMs
                playRate = [double]$_.playRate
                loop = [bool]$_.repeatUntilStageEnd
            }
        })
    }
}

function Get-PatternStage([object]$Pattern, [string]$StageId) {
    return @($Pattern.stages | Where-Object { [string]$_.stageId -ceq $StageId })
}

function Write-ProjectionAtomically([object]$Projection, [string]$Destination) {
    $resolved = if ([IO.Path]::IsPathRooted($Destination)) {
        [IO.Path]::GetFullPath($Destination)
    }
    else {
        [IO.Path]::GetFullPath((Join-Path $repoRoot $Destination))
    }
    $directory = Split-Path -Parent $resolved
    if ([string]::IsNullOrWhiteSpace($directory)) {
        throw "Projection output has no parent directory: $Destination"
    }
    [IO.Directory]::CreateDirectory($directory) | Out-Null
    $stagePath = Join-Path $directory ('.' + [IO.Path]::GetFileName($resolved) +
        '.stage.' + [Guid]::NewGuid().ToString('N'))
    $backupPath = $resolved + '.rollback.' + [Guid]::NewGuid().ToString('N')
    try {
        $text = ($Projection | ConvertTo-Json -Depth 100) + "`n"
        [IO.File]::WriteAllText($stagePath, $text, $utf8NoBom)
        $roundTrip = Read-JsonStrict $stagePath 'staged Valtan pattern projection'
        if ([string]$roundTrip.schema -cne 'lostark.valtan-pattern-master-projection') {
            throw 'Staged Valtan pattern projection failed round-trip validation.'
        }
        if ([IO.File]::Exists($resolved)) {
            [IO.File]::Replace($stagePath, $resolved, $backupPath, $true)
            [IO.File]::Delete($backupPath)
        }
        else {
            [IO.File]::Move($stagePath, $resolved)
        }
    }
    catch {
        if ([IO.File]::Exists($stagePath)) { [IO.File]::Delete($stagePath) }
        if ([IO.File]::Exists($backupPath)) {
            if ([IO.File]::Exists($resolved)) { [IO.File]::Delete($resolved) }
            [IO.File]::Move($backupPath, $resolved)
        }
        throw
    }
}

function Assert-ProjectedProductSet(
    [object]$Encounter,
    [object]$AnimationBindings,
    [object]$EffectCues,
    [object]$CombatObjects,
    [object]$PatternRotations,
    [object[]]$ExpectedPatterns,
    [object[]]$ExpectedBindings,
    [object[]]$ExpectedRotations,
    [hashtable]$ExpectedCueOwners,
    [hashtable]$ExpectedCombatObjectLifetimes,
    [string[]]$RetiredPatternIds) {
    foreach ($projectedPattern in @($ExpectedPatterns)) {
        $actualPatterns = @($Encounter.patterns | Where-Object {
            [string]$_.patternId -ceq [string]$projectedPattern.patternId
        })
        if ($actualPatterns.Count -ne 1) {
            throw "Encounter product is missing managed pattern $($projectedPattern.patternId)."
        }
        Assert-SemanticallyEqual $projectedPattern $actualPatterns[0] `
            "Encounter pattern $($projectedPattern.patternId)"
    }
    foreach ($retiredPatternId in @($RetiredPatternIds)) {
        if (@($Encounter.patterns | Where-Object {
            [string]$_.patternId -ceq [string]$retiredPatternId
        }).Count -ne 0) {
            throw "Encounter product still contains retired pattern $retiredPatternId."
        }
    }
    foreach ($projectedBinding in @($ExpectedBindings)) {
        $actualBindings = @($AnimationBindings.bindings | Where-Object {
            [string]$_.actionId -ceq [string]$projectedBinding.actionId
        })
        if ($actualBindings.Count -ne 1) {
            throw "Animation product is missing managed action $($projectedBinding.actionId)."
        }
        Assert-SemanticallyEqual $projectedBinding $actualBindings[0] `
            "Animation binding $($projectedBinding.actionId)"
    }
    foreach ($projectedRotation in @($ExpectedRotations)) {
        $actualRotations = @($PatternRotations.rotations | Where-Object {
            [string]$_.rotationId -ceq [string]$projectedRotation.rotationId
        })
        if ($actualRotations.Count -ne 1) {
            throw "Pattern rotation product is missing master-owned range $($projectedRotation.rotationId)."
        }
        Assert-SemanticallyEqual $projectedRotation $actualRotations[0] `
            "Pattern rotation $($projectedRotation.rotationId)"
    }
    $managedPatternIds = @($ExpectedPatterns | ForEach-Object { [string]$_.patternId }) +
        @($RetiredPatternIds)
    [string[]]$actualManagedCueIds = @($EffectCues.cues | Where-Object {
        [string]$_.patternId -cin $managedPatternIds
    } | ForEach-Object { [string]$_.bindingId })
    [string[]]$expectedManagedCueIds = @($ExpectedCueOwners.Keys | ForEach-Object { [string]$_ })
    [Array]::Sort($actualManagedCueIds, [StringComparer]::Ordinal)
    [Array]::Sort($expectedManagedCueIds, [StringComparer]::Ordinal)
    if (($actualManagedCueIds -join "`n") -cne ($expectedManagedCueIds -join "`n")) {
        throw 'Managed Valtan effect cue set has drifted from the pattern master.'
    }
    foreach ($cueId in @($ExpectedCueOwners.Keys)) {
        $cues = @($EffectCues.cues | Where-Object {
            [string]$_.bindingId -ceq [string]$cueId
        })
        if ($cues.Count -ne 1) { throw "Missing effect cue binding: $cueId" }
        $cue = $cues[0]
        $owner = $ExpectedCueOwners[$cueId]
        if ([string]$cue.patternId -cne [string]$owner.patternId -or
            [string]$cue.stageId -cne [string]$owner.stageId -or
            [string]$cue.actionId -cne [string]$owner.actionId -or
            [string]$cue.clipOccurrenceId -cne [string]$owner.clipOccurrenceId -or
            [int]$cue.sourceStartMs -ne [int]$owner.sourceStartMs -or
            (($null -eq $cue.sourceEndMs) -ne ($null -eq $owner.sourceEndMs)) -or
            ($null -ne $owner.sourceEndMs -and
                [int]$cue.sourceEndMs -ne [int]$owner.sourceEndMs)) {
            throw "Effect cue owner has drifted from the Valtan pattern master: $cueId"
        }
    }
    foreach ($combatObjectArchetypeId in @($ExpectedCombatObjectLifetimes.Keys)) {
        $objects = @($CombatObjects.objects | Where-Object {
            [string]$_.combatObjectArchetypeId -ceq [string]$combatObjectArchetypeId
        })
        if ($objects.Count -ne 1 -or
            [int]$objects[0].lifeMs -ne [int]$ExpectedCombatObjectLifetimes[$combatObjectArchetypeId]) {
            throw "Combat-object lifetime has drifted from the owner stage: $combatObjectArchetypeId"
        }
    }
}

function Stage-JsonTextDocument([string]$Text, [string]$TargetPath, [string]$Context) {
    $directory = Split-Path -Parent $TargetPath
    $stagePath = Join-Path $directory ('.' + [IO.Path]::GetFileName($TargetPath) +
        '.stage.' + [Guid]::NewGuid().ToString('N'))
    [IO.File]::WriteAllText($stagePath, $Text, $utf8NoBom)
    $roundTrip = Read-JsonStrict $stagePath "staged $Context"
    return [pscustomobject]@{
        TargetPath = $TargetPath
        StagePath = $stagePath
        Document = $roundTrip
        BackupPath = $TargetPath + '.rollback.' + [Guid]::NewGuid().ToString('N')
        ReplaceBackupPath = $TargetPath + '.replace-backup.' + [Guid]::NewGuid().ToString('N')
    }
}

function Commit-StagedDocuments([object[]]$Entries) {
    $backedUp = [Collections.Generic.List[object]]::new()
    try {
        foreach ($entry in @($Entries)) {
            [IO.File]::Copy($entry.TargetPath, $entry.BackupPath, $false)
            $backedUp.Add($entry)
        }
        foreach ($entry in @($Entries)) {
            [IO.File]::Replace(
                $entry.StagePath,
                $entry.TargetPath,
                $entry.ReplaceBackupPath,
                $true)
        }
        foreach ($entry in @($Entries)) {
            if ([IO.File]::Exists($entry.BackupPath)) {
                [IO.File]::Delete($entry.BackupPath)
            }
        }
    }
    catch {
        foreach ($entry in @($backedUp)) {
            if ([IO.File]::Exists($entry.BackupPath)) {
                [IO.File]::Copy($entry.BackupPath, $entry.TargetPath, $true)
            }
        }
        throw
    }
    finally {
        foreach ($entry in @($Entries)) {
            if ([IO.File]::Exists($entry.StagePath)) { [IO.File]::Delete($entry.StagePath) }
            if ([IO.File]::Exists($entry.BackupPath)) { [IO.File]::Delete($entry.BackupPath) }
            if ([IO.File]::Exists($entry.ReplaceBackupPath)) {
                [IO.File]::Delete($entry.ReplaceBackupPath)
            }
        }
    }
}

if ($Mode -eq 'PublishV2') {
    $pipeline = Join-Path $PSScriptRoot 'valtan_tuning_pipeline.py'
    if (-not [IO.File]::Exists($pipeline)) {
        throw "Missing Valtan v2 tuning pipeline: $pipeline"
    }
    $projectionParent = [IO.Path]::GetFullPath((Join-Path $repoRoot `
        'Intermediate\ValtanProductProjection'))
    [IO.Directory]::CreateDirectory($projectionParent) | Out-Null
    $projectionRoot = [IO.Path]::GetFullPath((Join-Path $projectionParent `
        ([Guid]::NewGuid().ToString('N'))))
    [IO.Directory]::CreateDirectory($projectionRoot) | Out-Null
    $mutex = $null
    $mutexHeld = $false
    try {
        if (-not $WriterLockAlreadyHeld) {
            $sha256 = [Security.Cryptography.SHA256]::Create()
            try {
                $mutexHash = $sha256.ComputeHash(
                    [Text.Encoding]::UTF8.GetBytes($repoRoot.ToUpperInvariant()))
            }
            finally {
                $sha256.Dispose()
            }
            $mutexName = 'LostArk_ValtanV2ProductPublish_' +
                (([BitConverter]::ToString($mutexHash) -replace '-', '').Substring(0, 24))
            $mutex = [Threading.Mutex]::new($false, $mutexName)
            try {
                $mutexHeld = $mutex.WaitOne([TimeSpan]::FromSeconds(30))
            }
            catch [Threading.AbandonedMutexException] {
                $mutexHeld = $true
            }
            if (-not $mutexHeld) {
                throw 'Timed out waiting for the Valtan split Product publisher.'
            }
        }
        $projectCommand = @($pipeline, '--repository-root', $repoRoot,
            'project-products', '--output-root', $projectionRoot)
        $projectText = (& python @projectCommand | Out-String).Trim()
        if ($global:LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($projectText)) {
            throw "Valtan v2 Product projection failed with exit code $global:LASTEXITCODE."
        }
        $projectResult = $projectText | ConvertFrom-Json
        if (-not [bool]$projectResult.ok -or
            [string]$projectResult.command -cne 'PROJECT_PRODUCTS') {
            throw 'Valtan v2 Product projection returned an invalid result.'
        }

        [string[]]$expectedRelativePaths = @(
            'Data/Encounters/Valtan/ValtanEncounter.json',
            'Data/Encounters/Valtan/ValtanPatternRotations.json',
            'Data/Encounters/Valtan/ValtanCombatObjects.json',
            'Data/Encounters/Valtan/ValtanWorldEvents.json',
            'Data/Animation/Authored/Valtan/Valtan.patternbindings.json',
            'Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json',
            'Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json'
        )
        [string[]]$actualRelativePaths = @(
            $projectResult.payload.files | ForEach-Object { [string]$_.path })
        [Array]::Sort($expectedRelativePaths, [StringComparer]::Ordinal)
        [Array]::Sort($actualRelativePaths, [StringComparer]::Ordinal)
        if (($expectedRelativePaths -join "`n") -cne
            ($actualRelativePaths -join "`n")) {
            throw 'Valtan v2 Product projection artifact set is invalid.'
        }

        $manifestCommand = @($pipeline, '--repository-root', $repoRoot,
            'source-manifest', '--repository-only')
        $manifestText = (& python @manifestCommand | Out-String).Trim()
        if ($global:LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($manifestText)) {
            throw 'Valtan source CAS query failed before Product commit.'
        }
        $manifestResult = $manifestText | ConvertFrom-Json
        if (-not [bool]$manifestResult.ok -or
            [string]$manifestResult.payload.sourceManifestId -cne
                [string]$projectResult.payload.sourceManifestId) {
            throw 'Valtan sources changed before Product commit.'
        }
        $transactionBackend = Join-Path $PSScriptRoot `
            'promote_valtan_animation_chains.py'
        if (-not [IO.File]::Exists($transactionBackend)) {
            throw "Missing shared Valtan Product transaction backend: $transactionBackend"
        }
        $commitCommand = @(
            $transactionBackend,
            '--repo-root', $repoRoot,
            '--mode', 'CommitProjectedProducts',
            '--projected-product-root', $projectionRoot,
            '--expected-source-manifest-id',
                [string]$projectResult.payload.sourceManifestId
        )
        if ($WriterLockAlreadyHeld) {
            $commitCommand += @(
                '--external-lock-owner-pid', [string]$PID,
                '--external-lock-owner-nonce', $WriterLockOwnerNonce)
        }
        else {
            $commitCommand += @('--lock-timeout-seconds', '30')
        }
        $commitText = (& python @commitCommand | Out-String).Trim()
        if ($global:LASTEXITCODE -ne 0 -or
            [string]::IsNullOrWhiteSpace($commitText)) {
            throw "Valtan shared Product commit failed with exit code $global:LASTEXITCODE."
        }
        $commitResult = $commitText | ConvertFrom-Json
        if ([string]$commitResult.schema -cne
                'lostark.valtan-product-transaction-commit-result' -or
            [int]$commitResult.formatVersion -ne 1 -or
            [string]$commitResult.mode -cne 'CommitProjectedProducts' -or
            [string]$commitResult.sourceManifestId -cne
                [string]$projectResult.payload.sourceManifestId -or
            [int]$commitResult.artifactCount -ne $expectedRelativePaths.Count -or
            [int]$commitResult.changedCount -lt 0 -or
            [int]$commitResult.changedCount -gt $expectedRelativePaths.Count) {
            throw 'Valtan shared Product commit returned an invalid result.'
        }
        Write-Host ("Valtan split Products committed: changed={0} artifacts={1}" -f `
            [int]$commitResult.changedCount, $expectedRelativePaths.Count)
    }
    finally {
        if ($mutexHeld) { $mutex.ReleaseMutex() }
        if ($null -ne $mutex) { $mutex.Dispose() }
        $projectionPrefix = $projectionParent.TrimEnd('\', '/') +
            [IO.Path]::DirectorySeparatorChar
        if ([IO.Directory]::Exists($projectionRoot) -and
            $projectionRoot.StartsWith($projectionPrefix,
                [StringComparison]::OrdinalIgnoreCase)) {
            [IO.Directory]::Delete($projectionRoot, $true)
        }
    }
    return
}

$masterFile = Resolve-InputPath $MasterPath 'Valtan pattern master'
$master = Read-JsonStrict $masterFile 'Valtan pattern master'
Assert-ExactProperties $master @(
    'schema','formatVersion','bossArchetypeId','encounterId','scope',
    'previewPaths','retiredPatternIds','normalSelection',
    'counterReactionLayers','independentEffects','patterns'
) 'Valtan pattern master'
if ([string]$master.schema -cne 'lostark.valtan-pattern-master' -or
    [int]$master.formatVersion -ne 1 -or
    [string]$master.bossArchetypeId -cne 'BOSS_VALTAN' -or
    [string]$master.encounterId -cne 'ENCOUNTER_VALTAN' -or
    [string]$master.scope -cne 'PHASE_ONE') {
    throw 'Valtan pattern master header is invalid.'
}

Assert-ExactProperties $master.previewPaths @(
    'encounter','animationBindings','effectCues','combatObjects','bossCatalog',
    'effectCatalog','damageProfiles','cinematicCamera','worldEvents',
    'patternRotations','sourceClipSequences'
) 'Valtan pattern master previewPaths'
$documentPaths = @{}
foreach ($property in $master.previewPaths.PSObject.Properties) {
    $documentPaths[$property.Name] = Resolve-RepositoryDocumentPath ([string]$property.Value) `
        "previewPaths.$($property.Name)"
}

$encounterDocument = Read-JsonStrict $documentPaths.encounter 'Valtan encounter product'
$animationDocument = Read-JsonStrict $documentPaths.animationBindings 'Valtan animation binding product'
$effectCueDocument = Read-JsonStrict $documentPaths.effectCues 'Valtan effect cue product'
$combatObjectDocument = Read-JsonStrict $documentPaths.combatObjects 'Valtan combat object product'
$rotationDocument = Read-JsonStrict $documentPaths.patternRotations 'Valtan pattern rotation product'
$encounterText = [IO.File]::ReadAllText($documentPaths.encounter, $utf8Strict)
$animationText = [IO.File]::ReadAllText($documentPaths.animationBindings, $utf8Strict)
$effectCueText = [IO.File]::ReadAllText($documentPaths.effectCues, $utf8Strict)
$combatObjectText = [IO.File]::ReadAllText($documentPaths.combatObjects, $utf8Strict)
$rotationText = [IO.File]::ReadAllText($documentPaths.patternRotations, $utf8Strict)
$bossCatalogDocument = Read-JsonStrict $documentPaths.bossCatalog 'boss catalog product'
$effectCatalogDocument = Read-JsonStrict $documentPaths.effectCatalog 'effect catalog product'
$damageDocument = Read-JsonStrict $documentPaths.damageProfiles 'damage profile product'
$cameraDocument = Read-JsonStrict $documentPaths.cinematicCamera 'Valtan cinematic camera product'
$worldEventDocument = Read-JsonStrict $documentPaths.worldEvents 'Valtan world event product'
$clipSequenceText = [IO.File]::ReadAllText($documentPaths.sourceClipSequences, $utf8Strict)

$requiredPatternIds = @(
    'VALTAN_WHIRLWIND','VALTAN_DASH_CHARGE','VALTAN_FOUR_SLASH',
    'VALTAN_FIST_IN_OUT','VALTAN_HIGH_JUMP','VALTAN_FLOOR_WIPE_130',
    'VALTAN_ARENA_BREAK_109'
)
$actualPatternIds = @($master.patterns | ForEach-Object { [string]$_.patternId })
[Array]::Sort($requiredPatternIds, [StringComparer]::Ordinal)
[Array]::Sort($actualPatternIds, [StringComparer]::Ordinal)
if (($requiredPatternIds -join "`n") -cne ($actualPatternIds -join "`n")) {
    throw 'Valtan pattern master must contain exactly the seven Phase-1 patterns.'
}
if ((@($master.retiredPatternIds) -join '|') -cne
    'VALTAN_TRIPLE_SLASH|VALTAN_ROTATION_SLASH') {
    throw 'Valtan pattern master retiredPatternIds are invalid.'
}

Assert-ExactProperties $master.normalSelection @(
    'selectionMode','ranges','patternIds'
) 'Valtan master normalSelection'
if ([string]$master.normalSelection.selectionMode -cne 'WEIGHTED_POOL' -or
    $master.normalSelection.ranges -isnot [Array] -or
    @($master.normalSelection.ranges).Count -eq 0 -or
    @($master.normalSelection.ranges).Count -gt 8 -or
    $master.normalSelection.patternIds -isnot [Array] -or
    @($master.normalSelection.patternIds).Count -eq 0) {
    throw 'Valtan master normalSelection contract is invalid.'
}
$requiredWeightedPatternIds = @(
    'VALTAN_WHIRLWIND','VALTAN_DASH_CHARGE','VALTAN_FOUR_SLASH',
    'VALTAN_FIST_IN_OUT','VALTAN_HIGH_JUMP'
)
$actualWeightedPatternIds = @($master.normalSelection.patternIds |
    ForEach-Object { [string]$_ })
[Array]::Sort($requiredWeightedPatternIds, [StringComparer]::Ordinal)
[Array]::Sort($actualWeightedPatternIds, [StringComparer]::Ordinal)
if (($requiredWeightedPatternIds -join "`n") -cne
    ($actualWeightedPatternIds -join "`n")) {
    throw 'Valtan Phase-1 weighted pool must contain exactly the five managed normal patterns.'
}
foreach ($weightedPatternId in @($master.normalSelection.patternIds)) {
    Assert-StableId $weightedPatternId 'Valtan master weighted patternId'
    $weightedOwners = @($master.patterns | Where-Object {
        [string]$_.patternId -ceq [string]$weightedPatternId -and
        [string]$_.selectionMode -ceq 'NORMAL'
    })
    if ($weightedOwners.Count -ne 1) {
        throw "Valtan weighted pool names no managed normal pattern: $weightedPatternId"
    }
}
$rotationProjection = [Collections.Generic.List[object]]::new()
$normalSelectionRangeIds =
    [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$previousNormalSelectionFromBar = [int]::MaxValue
foreach ($range in @($master.normalSelection.ranges)) {
    Assert-ExactProperties $range @(
        'rotationId','fromHealthBar','toHealthBar'
    ) 'Valtan master normalSelection range'
    Assert-StableId $range.rotationId 'Valtan master normalSelection rotationId'
    Assert-JsonInteger $range.fromHealthBar 'Valtan master normalSelection fromHealthBar' 1 1000
    Assert-JsonInteger $range.toHealthBar 'Valtan master normalSelection toHealthBar' 1 1000
    $fromHealthBar = [int]$range.fromHealthBar
    $toHealthBar = [int]$range.toHealthBar
    if (-not $normalSelectionRangeIds.Add([string]$range.rotationId) -or
        $fromHealthBar -le $toHealthBar -or
        $fromHealthBar -gt $previousNormalSelectionFromBar) {
        throw "Valtan master normalSelection range is invalid: $($range.rotationId)"
    }
    $previousNormalSelectionFromBar = $fromHealthBar
    $rotationProjection.Add([pscustomobject][ordered]@{
        rotationId = [string]$range.rotationId
        selectionMode = 'WEIGHTED_POOL'
        fromHealthBar = $fromHealthBar
        toHealthBar = $toHealthBar
        patternIds = @($master.normalSelection.patternIds)
    })
}
if ([string]$rotationDocument.schema -cne 'lostark.valtan-pattern-rotations' -or
    [int]$rotationDocument.formatVersion -ne 2 -or
    [string]$rotationDocument.encounterId -cne [string]$master.encounterId -or
    [string]$rotationDocument.bossArchetypeId -cne [string]$master.bossArchetypeId) {
    throw 'Valtan pattern rotation product header is invalid.'
}

if ($master.counterReactionLayers -isnot [Array] -or
    @($master.counterReactionLayers).Count -eq 0 -or
    @($master.counterReactionLayers).Count -gt 16) {
    throw 'Valtan master counterReactionLayers inventory is invalid.'
}
$counterReactionLayerIds =
    [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$counterReactionOwnerStages =
    [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($reactionLayer in @($master.counterReactionLayers)) {
    Assert-ExactProperties $reactionLayer @(
        'reactionLayerId','admissionScope','ownerPatternId','ownerStageId',
        'windowActionId','successActionId','failureActionId'
    ) 'Valtan master counter reaction layer'
    foreach ($field in @(
        'reactionLayerId','ownerPatternId','ownerStageId','windowActionId',
        'successActionId','failureActionId')) {
        Assert-StableId $reactionLayer.$field "Valtan counter reaction $field"
    }
    if ([string]$reactionLayer.admissionScope -cne 'REFERENCE_ONLY_LEGACY' -or
        -not $counterReactionLayerIds.Add([string]$reactionLayer.reactionLayerId) -or
        [string]$reactionLayer.ownerPatternId -cin @($master.patterns.patternId)) {
        throw "Valtan counter reaction admission is invalid: $($reactionLayer.reactionLayerId)"
    }
    $ownerPatterns = @($encounterDocument.patterns | Where-Object {
        [string]$_.patternId -ceq [string]$reactionLayer.ownerPatternId
    })
    if ($ownerPatterns.Count -ne 1) {
        throw "Valtan counter reaction owner pattern is missing: $($reactionLayer.ownerPatternId)"
    }
    $ownerStages = @($ownerPatterns[0].stages | Where-Object {
        [string]$_.stageId -ceq [string]$reactionLayer.ownerStageId
    })
    if ($ownerStages.Count -ne 1 -or
        [string]$ownerStages[0].actionId -cne [string]$reactionLayer.windowActionId) {
        throw "Valtan counter reaction owner stage/action changed: $($reactionLayer.reactionLayerId)"
    }
    $hasCounterEnter = @($ownerStages[0].actions | Where-Object {
        [string]$_.trigger -ceq 'ENTER' -and
        [string]$_.kind -ceq 'SET_BOSS_FLAG' -and
        [string]$_.targetId -ceq 'boss.flag.counterable' -and
        [int]$_.value -eq 1
    }).Count -eq 1
    $hasCounterExit = @($ownerStages[0].actions | Where-Object {
        [string]$_.trigger -ceq 'EXIT' -and
        [string]$_.kind -ceq 'SET_BOSS_FLAG' -and
        [string]$_.targetId -ceq 'boss.flag.counterable' -and
        [int]$_.value -eq 0
    }).Count -eq 1
    $counterBranches = @($ownerStages[0].branches | Where-Object {
        [string]$_.outcome -ceq 'COUNTER_HIT' -and
        [string]$_.nextActionId -ceq [string]$reactionLayer.successActionId
    })
    $failureBranches = @($ownerStages[0].branches | Where-Object {
        [string]$_.outcome -ceq 'TIMEOUT' -and
        [string]$_.nextActionId -ceq [string]$reactionLayer.failureActionId
    })
    if (-not $hasCounterEnter -or -not $hasCounterExit -or
        $counterBranches.Count -ne 1 -or $failureBranches.Count -ne 1) {
        throw "Valtan counter reaction branch/flag contract changed: $($reactionLayer.reactionLayerId)"
    }
    foreach ($actionField in @('windowActionId','successActionId','failureActionId')) {
        $actionId = [string]$reactionLayer.$actionField
        $actionStageOwners = @($ownerPatterns[0].stages | Where-Object {
            [string]$_.actionId -ceq $actionId
        })
        $bindingOwners = @($animationDocument.bindings | Where-Object {
            [string]$_.actionId -ceq $actionId
        })
        if ($actionStageOwners.Count -ne 1 -or $bindingOwners.Count -ne 1 -or
            @($bindingOwners[0].clips).Count -eq 0) {
            throw "Valtan counter reaction animation action is not exact: $actionId"
        }
    }
    $ownerStageKey = [string]$reactionLayer.ownerPatternId + '/' +
        [string]$reactionLayer.ownerStageId
    if (-not $counterReactionOwnerStages.Add($ownerStageKey)) {
        throw "Valtan counter reaction owner stage is duplicated: $ownerStageKey"
    }
}
$productCounterOwnerStages = [Collections.Generic.HashSet[string]]::new(
    [StringComparer]::Ordinal)
foreach ($encounterPattern in @($encounterDocument.patterns)) {
    foreach ($encounterStage in @($encounterPattern.stages)) {
        if (@($encounterStage.actions | Where-Object {
            [string]$_.trigger -ceq 'ENTER' -and
            [string]$_.kind -ceq 'SET_BOSS_FLAG' -and
            [string]$_.targetId -ceq 'boss.flag.counterable' -and
            [int]$_.value -eq 1
        }).Count -eq 1) {
            $null = $productCounterOwnerStages.Add(
                [string]$encounterPattern.patternId + '/' +
                [string]$encounterStage.stageId)
        }
    }
}
if (($productCounterOwnerStages | Sort-Object) -join "`n" -cne
    (($counterReactionOwnerStages | Sort-Object) -join "`n")) {
    throw 'Valtan counter reaction layer does not cover the exact Product counterable stages.'
}

$patternIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$actionIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$clipOccurrenceIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$cueOwners = @{}
$combatObjectLifeProjection = @{}
$encounterProjection = [Collections.Generic.List[object]]::new()
$animationProjection = [Collections.Generic.List[object]]::new()
$damageProfileIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($profile in @($damageDocument.profiles)) {
    $null = $damageProfileIds.Add([string]$profile.damageProfileId)
}
$effectAssetIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($effect in @($effectCatalogDocument.effects)) {
    $null = $effectAssetIds.Add([string]$effect.effectAssetId)
}
$cueById = @{}
foreach ($cue in @($effectCueDocument.cues)) {
    $cueId = [string]$cue.bindingId
    if ($cueById.ContainsKey($cueId)) { throw "Duplicate effect cue bindingId: $cueId" }
    $cueById[$cueId] = $cue
}

$patternFields = @(
    'patternId','category','minimumPhase','maximumPhase','targetPolicy','aimPolicy',
    'displayName','actionId','sourceActionIds','sourceSequenceIndex','presentationSources',
    'selectionMode',
    'minimumHealthBar','maximumHealthBar','triggerHealthBar','triggerOrder',
    'armorRequirement','phaseRequirement','invulnerableWhileRunning','selectionWeight',
    'maximumConsecutiveUses','minimumRange','maximumRange','serverMotion','reactions',
    'cameraCueIds','worldEventTriggerRefs','stages'
)
$stageFields = @(
    'stageId','sequenceRole','actionId','stageKind','durationMs','hitShape',
    'hitOuterRadius','hitInnerRadius','hitAngleDegrees','hitLength','hitHalfWidth',
    'hitCount','hitIntervalMs','hitDelayMs','hitOffsetsMs','serverDamageProfileId',
    'pushRangeM','pushMs','knockdown','downMs','motion','actions','branches',
    'animation','effectRefs'
)

foreach ($pattern in @($master.patterns)) {
    $patternId = [string]$pattern.patternId
    Assert-ExactProperties $pattern $patternFields "pattern $patternId"
    Assert-StableId $patternId "pattern $patternId patternId"
    if (-not $patternIds.Add($patternId)) { throw "Duplicate patternId: $patternId" }
    Assert-DisplayText $pattern.displayName "pattern $patternId displayName"
    Assert-StableId $pattern.actionId "pattern $patternId actionId"
    Assert-JsonInteger $pattern.minimumPhase "pattern $patternId minimumPhase" 1 3
    Assert-JsonInteger $pattern.maximumPhase "pattern $patternId maximumPhase" 1 3
    if ([int]$pattern.minimumPhase -gt [int]$pattern.maximumPhase) {
        throw "pattern $patternId phase range is inverted."
    }
    foreach ($integerField in @(
        'minimumHealthBar','maximumHealthBar','triggerHealthBar','triggerOrder',
        'selectionWeight','maximumConsecutiveUses','sourceSequenceIndex')) {
        Assert-JsonInteger $pattern.$integerField "pattern $patternId $integerField" 0 100000
    }
    Assert-JsonNumber $pattern.minimumRange "pattern $patternId minimumRange" 0 1000
    Assert-JsonNumber $pattern.maximumRange "pattern $patternId maximumRange" 0 1000
    Assert-JsonBoolean $pattern.invulnerableWhileRunning "pattern $patternId invulnerableWhileRunning"
    if ([double]$pattern.minimumRange -gt [double]$pattern.maximumRange) {
        throw "pattern $patternId range is inverted."
    }
    if (@($pattern.sourceActionIds).Count -eq 0) {
        throw "pattern $patternId has no sourceActionIds."
    }
    foreach ($sourceActionId in @($pattern.sourceActionIds)) {
        Assert-JsonInteger $sourceActionId "pattern $patternId sourceActionId" 1 9999999
    }
    if (@($pattern.presentationSources).Count -eq 0) {
        throw "pattern $patternId has no presentationSources."
    }
    $presentationSourceRoles =
        [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $presentationClipIds =
        [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $hasPrimarySource = $false
    foreach ($presentationSource in @($pattern.presentationSources)) {
        Assert-ExactProperties $presentationSource @(
            'sourceActionId','sequenceIndex','role'
        ) "pattern $patternId presentation source"
        Assert-JsonInteger $presentationSource.sourceActionId `
            "pattern $patternId presentation sourceActionId" 1 9999999
        Assert-JsonInteger $presentationSource.sequenceIndex `
            "pattern $patternId presentation sequenceIndex" 0 100000
        Assert-StableId $presentationSource.role `
            "pattern $patternId presentation role"
        if (-not $presentationSourceRoles.Add([string]$presentationSource.role)) {
            throw "pattern $patternId has duplicate presentation source role $($presentationSource.role)."
        }
        if ([int]$presentationSource.sourceActionId -eq [int]$pattern.sourceActionIds[0] -and
            [int]$presentationSource.sequenceIndex -eq [int]$pattern.sourceSequenceIndex) {
            $hasPrimarySource = $true
        }
        $sourceLinePattern = '(?m)^' +
            [regex]::Escape([string]$presentationSource.sourceActionId) +
            ' .* seq=' + [regex]::Escape([string]$presentationSource.sequenceIndex) +
            ' .* clips="([^"]*)"'
        $sourceLine = [regex]::Match($clipSequenceText, $sourceLinePattern)
        if (-not $sourceLine.Success) {
            throw "pattern $patternId presentation source does not exist in Valtan.clipseq."
        }
        foreach ($sourceClip in @($sourceLine.Groups[1].Value.Split(','))) {
            if (-not [string]::IsNullOrWhiteSpace($sourceClip)) {
                $null = $presentationClipIds.Add([string]$sourceClip)
            }
        }
    }
    if (-not $hasPrimarySource) {
        throw "pattern $patternId presentationSources omit the product primary source."
    }
    if ($null -ne $pattern.serverMotion) {
        [string[]]$motionFields = @($pattern.serverMotion.PSObject.Properties.Name)
        foreach ($requiredMotionField in @(
            'kind','anchorId','landingPosition','apexHeight','travelStageId',
            'takeoffStartMs','takeoffEndMs','travelStartMs','travelEndMs')) {
            if ($requiredMotionField -notin $motionFields) {
                throw "pattern $patternId serverMotion is missing $requiredMotionField."
            }
        }
        foreach ($unknownMotionField in @($motionFields | Where-Object {
            $_ -notin @(
                'kind','anchorId','landingPosition','apexHeight','travelStageId',
                'takeoffStartMs','takeoffEndMs','travelStartMs','travelEndMs')
        })) {
            throw "pattern $patternId serverMotion has unknown field $unknownMotionField."
        }
        Assert-StableId $pattern.serverMotion.anchorId "pattern $patternId serverMotion.anchorId"
        if ([string]$pattern.serverMotion.kind -cne 'LEAP_TO_ANCHOR' -and
            [string]$pattern.serverMotion.kind -cne 'LEAP_TO_TARGET') {
            throw "pattern $patternId serverMotion.kind is unsupported."
        }
        if (@($pattern.serverMotion.landingPosition).Count -ne 3) {
            throw "pattern $patternId serverMotion.landingPosition must have three numbers."
        }
        foreach ($coordinate in @($pattern.serverMotion.landingPosition)) {
            Assert-JsonNumber $coordinate "pattern $patternId serverMotion.landingPosition" -100000 100000
        }
        Assert-JsonNumber $pattern.serverMotion.apexHeight "pattern $patternId serverMotion.apexHeight" 0 1000
        Assert-StableId $pattern.serverMotion.travelStageId "pattern $patternId serverMotion.travelStageId"
        foreach ($windowField in @(
            'takeoffStartMs','takeoffEndMs','travelStartMs','travelEndMs')) {
            Assert-JsonInteger $pattern.serverMotion.$windowField `
                "pattern $patternId serverMotion.$windowField" 0 600000
        }
    }

    $stageIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($stage in @($pattern.stages)) {
        $stageId = [string]$stage.stageId
        $stageContext = "pattern $patternId stage $stageId"
        Assert-ExactProperties $stage $stageFields $stageContext
        Assert-StableId $stageId "$stageContext stageId"
        Assert-StableId $stage.sequenceRole "$stageContext sequenceRole"
        Assert-StableId $stage.actionId "$stageContext actionId"
        if (-not $stageIds.Add($stageId)) { throw "Duplicate $stageContext" }
        if (-not $actionIds.Add([string]$stage.actionId)) {
            throw "Animation actionId is owned by more than one master stage: $($stage.actionId)"
        }
        Assert-JsonInteger $stage.durationMs "$stageContext durationMs" 1 600000
        foreach ($integerField in @('hitCount','hitIntervalMs','hitDelayMs','pushMs','downMs')) {
            Assert-JsonInteger $stage.$integerField "$stageContext $integerField" 0 600000
        }
        foreach ($numberField in @(
            'hitOuterRadius','hitInnerRadius','hitAngleDegrees','hitLength',
            'hitHalfWidth','pushRangeM')) {
            Assert-JsonNumber $stage.$numberField "$stageContext $numberField" 0 100000
        }
        Assert-JsonBoolean $stage.knockdown "$stageContext knockdown"
        Assert-StableId $stage.serverDamageProfileId "$stageContext serverDamageProfileId" $true
        if ([int]$stage.hitCount -gt 0 -and
            [string]::IsNullOrEmpty([string]$stage.serverDamageProfileId)) {
            throw "$stageContext has contacts but no serverDamageProfileId."
        }
        if (-not [string]::IsNullOrEmpty([string]$stage.serverDamageProfileId) -and
            -not $damageProfileIds.Contains([string]$stage.serverDamageProfileId)) {
            throw "$stageContext references missing damage profile $($stage.serverDamageProfileId)."
        }
        foreach ($offset in @($stage.hitOffsetsMs)) {
            Assert-JsonInteger $offset "$stageContext hitOffsetsMs" 0 ([int]$stage.durationMs - 1)
        }
        if (@($stage.hitOffsetsMs).Count -ne 0 -and
            @($stage.hitOffsetsMs).Count -ne [int]$stage.hitCount) {
            throw "$stageContext hitOffsetsMs count must equal hitCount."
        }
        if (@($stage.hitOffsetsMs).Count -eq 0 -and [int]$stage.hitCount -gt 1) {
            $lastContactMs = [int]$stage.hitDelayMs +
                (([int]$stage.hitCount - 1) * [int]$stage.hitIntervalMs)
            if ($lastContactMs -ge [int]$stage.durationMs) {
                throw "$stageContext generated contact is outside the stage."
            }
        }
        $branchOutcomes = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($branch in @($stage.branches)) {
            Assert-ExactProperties $branch @('outcome','nextActionId') "$stageContext branch"
            Assert-StableId $branch.outcome "$stageContext branch.outcome"
            if (-not $branchOutcomes.Add([string]$branch.outcome)) {
                throw "$stageContext has duplicate branch outcome $($branch.outcome)."
            }
            if ($null -ne $branch.nextActionId) {
                Assert-StableId $branch.nextActionId "$stageContext branch.nextActionId"
            }
        }
        Assert-ExactProperties $stage.animation @('endPolicy','repeatCount','occurrences') `
            "$stageContext animation"
        Assert-StableId $stage.animation.endPolicy "$stageContext animation.endPolicy"
        if (-not $allowedAnimationEndPolicies.Contains(
                [string]$stage.animation.endPolicy)) {
            throw "$stageContext animation.endPolicy is outside the public vocabulary."
        }
        Assert-JsonInteger $stage.animation.repeatCount "$stageContext animation.repeatCount" 1 32
        $occurrences = @($stage.animation.occurrences)
        if ($occurrences.Count -eq 0) {
            throw "$stageContext must contain at least one ordered occurrence."
        }
        foreach ($occurrence in $occurrences) {
            Assert-ExactProperties $occurrence @(
                'clipOccurrenceId','clip','mappingBasis','sourceStartMs','playMs',
                'playRate','repeatUntilStageEnd'
            ) "$stageContext animation occurrence"
            Assert-StableId $occurrence.clipOccurrenceId "$stageContext clipOccurrenceId"
            Assert-StableId $occurrence.clip "$stageContext clip"
            Assert-StableId $occurrence.mappingBasis "$stageContext mappingBasis"
            if (-not $allowedMappingBases.Contains([string]$occurrence.mappingBasis)) {
                throw "$stageContext mappingBasis is outside the public vocabulary."
            }
            Assert-JsonInteger $occurrence.sourceStartMs "$stageContext sourceStartMs" 0 600000
            Assert-JsonInteger $occurrence.playMs "$stageContext playMs" 0 600000
            Assert-JsonNumber $occurrence.playRate "$stageContext playRate" 0.000001 1000
            Assert-JsonBoolean $occurrence.repeatUntilStageEnd "$stageContext repeatUntilStageEnd"
            if (-not $clipOccurrenceIds.Add([string]$occurrence.clipOccurrenceId)) {
                throw "Duplicate clipOccurrenceId: $($occurrence.clipOccurrenceId)"
            }
            if (-not $presentationClipIds.Contains([string]$occurrence.clip)) {
                throw "$stageContext clip is absent from declared presentationSources: $($occurrence.clip)"
            }
        }
        $repeatCount = [int]$stage.animation.repeatCount
        if ($repeatCount -gt 1) {
            $repeatClips = @($occurrences | ForEach-Object { [string]$_.clip } |
                Sort-Object -Unique)
            if ($occurrences.Count -ne $repeatCount -or $repeatClips.Count -ne 1) {
                throw "$stageContext finite repeatCount must equal the number of explicit same-clip occurrences."
            }
        }
        [double]$knownWallMs = 0.0
        $unknownOccurrences = @($occurrences | Where-Object { [int]$_.playMs -eq 0 })
        $loopOccurrences = @($occurrences | Where-Object {
            [bool]$_.repeatUntilStageEnd
        })
        foreach ($knownOccurrence in @($occurrences | Where-Object {
            [int]$_.playMs -gt 0
        })) {
            $knownWallMs += [double]$knownOccurrence.playMs /
                [double]$knownOccurrence.playRate
        }
        if ($loopOccurrences.Count -gt 0 -and
            @($loopOccurrences | Where-Object { [int]$_.playMs -ne 0 }).Count -gt 0) {
            throw "$stageContext repeatUntilStageEnd requires playMs 0."
        }
        $endPolicy = [string]$stage.animation.endPolicy
        if ($endPolicy -ceq 'EXACT') {
            if ($unknownOccurrences.Count -ne 0 -or $loopOccurrences.Count -ne 0 -or
                [Math]::Abs($knownWallMs - [double]$stage.durationMs) -gt 2.0) {
                throw "$stageContext EXACT animation must fill its Server stage within 2ms."
            }
        }
        elseif ($endPolicy -ceq 'HOLD_LAST_POSE') {
            if ($unknownOccurrences.Count -gt 1 -or $loopOccurrences.Count -ne 0 -or
                $knownWallMs -ge ([double]$stage.durationMs + 2.0) -or
                ($unknownOccurrences.Count -eq 1 -and
                    $knownWallMs -ge [double]$stage.durationMs)) {
                throw "$stageContext HOLD_LAST_POSE animation budget is invalid."
            }
        }
        elseif ($endPolicy -ceq 'LOOP_TO_STAGE_END') {
            if ($unknownOccurrences.Count -ne 1 -or $loopOccurrences.Count -ne 1 -or
                [string]$unknownOccurrences[0].clipOccurrenceId -cne
                    [string]$loopOccurrences[0].clipOccurrenceId -or
                $knownWallMs -ge [double]$stage.durationMs) {
                throw "$stageContext LOOP_TO_STAGE_END requires one looping remainder."
            }
        }
        foreach ($effectRef in @($stage.effectRefs)) {
            Assert-StableId $effectRef.refId "$stageContext effect refId"
            if ([string]$effectRef.refType -ceq 'CUE_BINDING') {
                Assert-ExactProperties $effectRef @('refType','refId','cueProjection') `
                    "$stageContext effectRef"
                Assert-CueProjection $effectRef.cueProjection $occurrences `
                    "$stageContext cueProjection"
                if ($cueOwners.ContainsKey([string]$effectRef.refId)) {
                    throw "Duplicate cue binding reference: $($effectRef.refId)"
                }
                $cueOwners[[string]$effectRef.refId] = [pscustomobject]@{
                    patternId = $patternId
                    stageId = $stageId
                    actionId = [string]$stage.actionId
                    clipOccurrenceId = [string]$effectRef.cueProjection.clipOccurrenceId
                    sourceStartMs = [int]$effectRef.cueProjection.sourceStartMs
                    sourceEndMs = $effectRef.cueProjection.sourceEndMs
                    mappingBasis = [string]$effectRef.cueProjection.mappingBasis
                }
            }
            elseif ([string]$effectRef.refType -ceq 'INDEPENDENT_EFFECT') {
                Assert-ExactProperties $effectRef @('refType','refId') "$stageContext effectRef"
            }
            else {
                throw "$stageContext effectRef has invalid refType $($effectRef.refType)."
            }
        }
        $animationProjection.Add((New-AnimationBindingProjection $stage))
    }
    foreach ($reaction in @($pattern.reactions)) {
        Assert-ExactProperties $reaction @('triggerKind','stageId') "pattern $patternId reaction"
        Assert-StableId $reaction.triggerKind "pattern $patternId reaction.triggerKind"
        if (@(Get-PatternStage $pattern ([string]$reaction.stageId)).Count -ne 1) {
            throw "pattern $patternId reaction references missing stage $($reaction.stageId)."
        }
    }
    $patternActionIds = @($pattern.stages | ForEach-Object { [string]$_.actionId })
    foreach ($stage in @($pattern.stages)) {
        foreach ($branch in @($stage.branches | Where-Object { $null -ne $_.nextActionId })) {
            if ([string]$branch.nextActionId -cnotin $patternActionIds) {
                throw "pattern $patternId stage $($stage.stageId) branches to an unknown action $($branch.nextActionId)."
            }
        }
    }
    if ($null -ne $pattern.serverMotion) {
		$orderedStages = @($pattern.stages)
		$entryStages = @($orderedStages | Where-Object {
			[string]$_.actionId -ceq [string]$pattern.entryActionId
		})
        $travelStages = @(
            Get-PatternStage $pattern ([string]$pattern.serverMotion.travelStageId)
        )
		$entryStageIndex = if ($entryStages.Count -eq 1) {
			[Array]::IndexOf($orderedStages, $entryStages[0])
		}
		else { -1 }
		$travelStageIndex = if ($travelStages.Count -eq 1) {
			[Array]::IndexOf($orderedStages, $travelStages[0])
		}
		else { -1 }
		if ($entryStages.Count -ne 1 -or $entryStageIndex -ne 0) {
			throw "pattern $patternId entryActionId must own the first ordered serverMotion stage."
		}
		if ($travelStages.Count -ne 1 -or
			$travelStageIndex -le $entryStageIndex) {
			throw "pattern $patternId serverMotion travel stage must follow the entry stage."
        }
		$takeoffStage = $entryStages[0]
        if ([int]$pattern.serverMotion.takeoffStartMs -ge
                [int]$pattern.serverMotion.takeoffEndMs -or
            [int]$pattern.serverMotion.takeoffEndMs -gt [int]$takeoffStage.durationMs -or
            [int]$pattern.serverMotion.travelStartMs -ge
                [int]$pattern.serverMotion.travelEndMs -or
            [int]$pattern.serverMotion.travelEndMs -gt
                [int]$travelStages[0].durationMs) {
            throw "pattern $patternId serverMotion window is outside its authored stage."
        }
    }
    foreach ($cameraCueId in @($pattern.cameraCueIds)) {
        Assert-StableId $cameraCueId "pattern $patternId cameraCueId"
        $cameraCues = @($cameraDocument.cues | Where-Object {
            [string]$_.cueId -ceq [string]$cameraCueId
        })
        if ($cameraCues.Count -ne 1 -or
            [string]$cameraCues[0].patternId -cne $patternId -or
            @(Get-PatternStage $pattern ([string]$cameraCues[0].stageId)).Count -ne 1) {
            throw "pattern $patternId camera cue reference is invalid: $cameraCueId"
        }
    }
    foreach ($worldRef in @($pattern.worldEventTriggerRefs)) {
        Assert-ExactProperties $worldRef @('patternId','stageId','triggerKind') `
            "pattern $patternId worldEventTriggerRef"
        if ([string]$worldRef.patternId -cne $patternId -or
            @(Get-PatternStage $pattern ([string]$worldRef.stageId)).Count -ne 1) {
            throw "pattern $patternId world event trigger reference has invalid owner."
        }
        $worldBindings = @($worldEventDocument.bindings | Where-Object {
            [string]$_.patternId -ceq [string]$worldRef.patternId -and
            [string]$_.stageId -ceq [string]$worldRef.stageId -and
            [string]$_.triggerKind -ceq [string]$worldRef.triggerKind -and
            [bool]$_.enabled
        })
        if ($worldBindings.Count -eq 0) {
            throw "pattern $patternId world event trigger reference resolves to no enabled binding."
        }
    }
    $encounterProjection.Add((New-EncounterPatternProjection $pattern))
}

$independentEffectIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$requiredIndependentEffectIds = @(
    'valtan.independent-effect.target-axe',
    'valtan.independent-effect.donut-in-out'
)
foreach ($independentEffect in @($master.independentEffects)) {
    Assert-ExactProperties $independentEffect @(
        'independentEffectId','displayName','effectAssetId','ownership',
        'ownerPatternId','ownerStageId','triggerPolicy','combatObjectArchetypeId',
        'clientVisualId','effectCueBindingId','cueProjection'
    ) 'independent effect'
    $independentEffectId = [string]$independentEffect.independentEffectId
    Assert-StableId $independentEffectId 'independentEffectId'
    if (-not $independentEffectIds.Add($independentEffectId)) {
        throw "Duplicate independentEffectId: $independentEffectId"
    }
    Assert-DisplayText $independentEffect.displayName "independent effect $independentEffectId displayName"
    Assert-StableId $independentEffect.effectAssetId "independent effect $independentEffectId effectAssetId"
    if (-not $effectAssetIds.Contains([string]$independentEffect.effectAssetId)) {
        throw "Independent effect asset is missing from EffectCatalog: $($independentEffect.effectAssetId)"
    }
    $ownerPatterns = @($master.patterns | Where-Object {
        [string]$_.patternId -ceq [string]$independentEffect.ownerPatternId
    })
    if ($ownerPatterns.Count -ne 1) {
        throw "Independent effect owner pattern is invalid: $independentEffectId"
    }
    $ownerStages = @(Get-PatternStage $ownerPatterns[0] ([string]$independentEffect.ownerStageId))
    if ($ownerStages.Count -ne 1) {
        throw "Independent effect owner stage is invalid: $independentEffectId"
    }
    $stageReferences = @(
        foreach ($candidatePattern in @($master.patterns)) {
            foreach ($candidateStage in @($candidatePattern.stages)) {
                foreach ($candidateRef in @($candidateStage.effectRefs)) {
                    if ([string]$candidateRef.refType -ceq 'INDEPENDENT_EFFECT' -and
                        [string]$candidateRef.refId -ceq $independentEffectId) {
                        $candidateRef
                    }
                }
            }
        }
    )
    $ownerStageReferences = @($ownerStages[0].effectRefs | Where-Object {
        [string]$_.refType -ceq 'INDEPENDENT_EFFECT' -and
        [string]$_.refId -ceq $independentEffectId
    })
    if ($stageReferences.Count -ne 1 -or $ownerStageReferences.Count -ne 1) {
        throw "Independent effect must have exactly one reference on its declared owner stage: $independentEffectId"
    }
    if ([string]$independentEffect.ownership -ceq 'SERVER_COMBAT_OBJECT') {
        Assert-StableId $independentEffect.combatObjectArchetypeId `
            "independent effect $independentEffectId combatObjectArchetypeId"
        Assert-StableId $independentEffect.clientVisualId `
            "independent effect $independentEffectId clientVisualId"
        if ($null -ne $independentEffect.effectCueBindingId) {
            throw "SERVER_COMBAT_OBJECT independent effect must not own an effect cue binding: $independentEffectId"
        }
        if ($null -ne $independentEffect.cueProjection) {
            throw "SERVER_COMBAT_OBJECT independent effect must not own cueProjection: $independentEffectId"
        }
        $objects = @($combatObjectDocument.objects | Where-Object {
            [string]$_.combatObjectArchetypeId -ceq [string]$independentEffect.combatObjectArchetypeId
        })
        if ($objects.Count -ne 1 -or
            [string]$objects[0].ownerPatternId -cne [string]$independentEffect.ownerPatternId -or
            [string]$objects[0].ownerStageActionId -cne [string]$ownerStages[0].actionId -or
            [string]$objects[0].clientVisualId -cne [string]$independentEffect.clientVisualId) {
            throw "SERVER_COMBAT_OBJECT independent effect has invalid combat-object ownership: $independentEffectId"
        }
        $combatObjectArchetypeId = [string]$independentEffect.combatObjectArchetypeId
        $ownerStageDurationMs = [int]$ownerStages[0].durationMs
        $combatObjectLifeProjection[$combatObjectArchetypeId] = $ownerStageDurationMs
        if ($performDriftCheck -and [int]$objects[0].lifeMs -ne $ownerStageDurationMs) {
            throw "Combat-object lifetime has drifted from the owner stage: $combatObjectArchetypeId"
        }
        $boss = @($bossCatalogDocument.bosses | Where-Object {
            [string]$_.archetypeId -ceq [string]$master.bossArchetypeId
        })
        $visuals = @($boss[0].combatObjectVisuals | Where-Object {
            [string]$_.combatObjectArchetypeId -ceq [string]$independentEffect.combatObjectArchetypeId -and
            [string]$_.clientVisualId -ceq [string]$independentEffect.clientVisualId -and
            [string]$_.effectAssetId -ceq [string]$independentEffect.effectAssetId
        })
        if ($boss.Count -ne 1 -or $visuals.Count -ne 1) {
            throw "SERVER_COMBAT_OBJECT independent effect has invalid BossCatalog visual: $independentEffectId"
        }
    }
    elseif ([string]$independentEffect.ownership -ceq 'SERVER_PATTERN_STAGE') {
        if ($null -ne $independentEffect.combatObjectArchetypeId -or
            $null -ne $independentEffect.clientVisualId) {
            throw "SERVER_PATTERN_STAGE independent effect must not own a combat-object identity: $independentEffectId"
        }
        Assert-StableId $independentEffect.effectCueBindingId `
            "independent effect $independentEffectId effectCueBindingId"
        Assert-CueProjection $independentEffect.cueProjection `
            @($ownerStages[0].animation.occurrences) `
            "independent effect $independentEffectId cueProjection"
        $cueId = [string]$independentEffect.effectCueBindingId
        if (-not $cueById.ContainsKey($cueId) -or
            [string]$cueById[$cueId].effectAssetId -cne [string]$independentEffect.effectAssetId) {
            throw "SERVER_PATTERN_STAGE independent effect has invalid effect cue: $independentEffectId"
        }
        if ($cueOwners.ContainsKey($cueId)) {
            throw "Independent effect cue is also stored as a normal stage cue: $cueId"
        }
        $cueOwners[$cueId] = [pscustomobject]@{
            patternId = [string]$independentEffect.ownerPatternId
            stageId = [string]$independentEffect.ownerStageId
            actionId = [string]$ownerStages[0].actionId
            clipOccurrenceId = [string]$independentEffect.cueProjection.clipOccurrenceId
            sourceStartMs = [int]$independentEffect.cueProjection.sourceStartMs
            sourceEndMs = $independentEffect.cueProjection.sourceEndMs
            mappingBasis = [string]$independentEffect.cueProjection.mappingBasis
        }
    }
    else {
        throw "Independent effect ownership is invalid: $($independentEffect.ownership)"
    }
}
[string[]]$actualIndependentEffectIds = @($independentEffectIds)
[Array]::Sort($actualIndependentEffectIds, [StringComparer]::Ordinal)
[Array]::Sort($requiredIndependentEffectIds, [StringComparer]::Ordinal)
if (($actualIndependentEffectIds -join "`n") -cne ($requiredIndependentEffectIds -join "`n")) {
    throw 'Valtan pattern master must contain exactly the axe and donut independent effects.'
}

foreach ($pattern in @($master.patterns)) {
    foreach ($stage in @($pattern.stages)) {
        foreach ($effectRef in @($stage.effectRefs | Where-Object {
            [string]$_.refType -ceq 'INDEPENDENT_EFFECT'
        })) {
            if (-not $independentEffectIds.Contains([string]$effectRef.refId)) {
                throw "Unknown independent effect reference: $($effectRef.refId)"
            }
        }
    }
}

foreach ($cueId in @($cueOwners.Keys)) {
    if (-not $cueById.ContainsKey($cueId)) { throw "Missing effect cue binding: $cueId" }
    $cue = $cueById[$cueId]
    if (-not $effectAssetIds.Contains([string]$cue.effectAssetId)) {
        throw "Effect cue references an asset missing from EffectCatalog: $cueId"
    }
    if ($performDriftCheck) {
        $owner = $cueOwners[$cueId]
        if ([string]$cue.patternId -cne [string]$owner.patternId -or
            [string]$cue.stageId -cne [string]$owner.stageId -or
            [string]$cue.actionId -cne [string]$owner.actionId -or
            [string]$cue.clipOccurrenceId -cne [string]$owner.clipOccurrenceId -or
            [int]$cue.sourceStartMs -ne [int]$owner.sourceStartMs -or
            (($null -eq $cue.sourceEndMs) -ne ($null -eq $owner.sourceEndMs)) -or
            ($null -ne $owner.sourceEndMs -and
                [int]$cue.sourceEndMs -ne [int]$owner.sourceEndMs)) {
            throw "Effect cue owner has drifted from the Valtan pattern master: $cueId"
        }
    }
}

if ($performDriftCheck) {
    Assert-ProjectedProductSet $encounterDocument $animationDocument $effectCueDocument `
        $combatObjectDocument $rotationDocument @($encounterProjection) `
        @($animationProjection) @($rotationProjection) $cueOwners `
        $combatObjectLifeProjection @($master.retiredPatternIds)
}

$cueOwnerProjection = @($cueOwners.Keys | Sort-Object | ForEach-Object {
    $owner = $cueOwners[$_]
    [pscustomobject][ordered]@{
        bindingId = [string]$_
        patternId = [string]$owner.patternId
        stageId = [string]$owner.stageId
        actionId = [string]$owner.actionId
        clipOccurrenceId = [string]$owner.clipOccurrenceId
        sourceStartMs = [int]$owner.sourceStartMs
        sourceEndMs = $owner.sourceEndMs
        mappingBasis = [string]$owner.mappingBasis
    }
})
$combatObjectLifetimeProjection = @($combatObjectLifeProjection.Keys | Sort-Object |
    ForEach-Object {
        [pscustomobject][ordered]@{
            combatObjectArchetypeId = [string]$_
            lifeMs = [int]$combatObjectLifeProjection[$_]
        }
    })
$projection = [pscustomobject][ordered]@{
    schema = 'lostark.valtan-pattern-master-projection'
    formatVersion = 1
    bossArchetypeId = [string]$master.bossArchetypeId
    encounterId = [string]$master.encounterId
    scope = [string]$master.scope
    runtimeAuthority = 'EXISTING_PRODUCT_DOCUMENTS'
    encounterPatterns = @($encounterProjection)
    animationBindings = @($animationProjection)
    normalSelection = $master.normalSelection
    patternRotations = @($rotationProjection)
    counterReactionLayers = @($master.counterReactionLayers)
    effectCueOwners = @($cueOwnerProjection)
    combatObjectLifetimes = @($combatObjectLifetimeProjection)
    independentEffects = @($master.independentEffects)
}

if ($Mode -eq 'Publish') {
    $publishedEncounterText = Update-FormatPreservingJsonArrayRows `
        $encounterText 'patterns' 'patternId' @($encounterProjection) `
        @($master.retiredPatternIds) -InsertMissingAtFirstRemoval
    $publishedAnimationText = Update-FormatPreservingJsonArrayRows `
        $animationText 'bindings' 'actionId' @($animationProjection)
    $publishedRotationText = Update-FormatPreservingJsonArrayRows `
        $rotationText 'rotations' 'rotationId' @($rotationProjection)

    $managedPatternIds = @($master.patterns | ForEach-Object { [string]$_.patternId }) +
        @($master.retiredPatternIds)
    $projectedCueRows = [Collections.Generic.List[object]]::new()
    foreach ($cue in @($effectCueDocument.cues)) {
        $cueId = [string]$cue.bindingId
        if ($cueOwners.ContainsKey($cueId)) {
            $owner = $cueOwners[$cueId]
            $projectedCue = ($cue | ConvertTo-Json -Depth 100) | ConvertFrom-Json
            $projectedCue.patternId = [string]$owner.patternId
            $projectedCue.stageId = [string]$owner.stageId
            $projectedCue.actionId = [string]$owner.actionId
            $projectedCue.clipOccurrenceId = [string]$owner.clipOccurrenceId
            $projectedCue.sourceStartMs = [int]$owner.sourceStartMs
            $projectedCue.sourceEndMs = $owner.sourceEndMs
            $projectedCueRows.Add($projectedCue)
        }
        elseif ([string]$cue.patternId -cin $managedPatternIds) {
            throw "Managed effect cue is not referenced by the master: $cueId"
        }
    }
    $publishedEffectCueText = Update-FormatPreservingJsonArrayRows `
        $effectCueText 'cues' 'bindingId' @($projectedCueRows)

    $projectedCombatObjectRows = [Collections.Generic.List[object]]::new()
    foreach ($combatObject in @($combatObjectDocument.objects)) {
        $combatObjectArchetypeId = [string]$combatObject.combatObjectArchetypeId
        if ($combatObjectLifeProjection.ContainsKey($combatObjectArchetypeId)) {
            $projectedCombatObject =
                ($combatObject | ConvertTo-Json -Depth 100) | ConvertFrom-Json
            $projectedCombatObject.lifeMs =
                [int]$combatObjectLifeProjection[$combatObjectArchetypeId]
            $projectedCombatObjectRows.Add($projectedCombatObject)
        }
    }
    $publishedCombatObjectText = Update-FormatPreservingJsonArrayRows `
        $combatObjectText 'objects' 'combatObjectArchetypeId' `
        @($projectedCombatObjectRows)

    $stagedEntries = [Collections.Generic.List[object]]::new()
    try {
        $stagedEntries.Add((Stage-JsonTextDocument $publishedEncounterText `
            $documentPaths.encounter `
            'Valtan encounter product'))
        $stagedEntries.Add((Stage-JsonTextDocument $publishedAnimationText `
            $documentPaths.animationBindings `
            'Valtan animation binding product'))
        $stagedEntries.Add((Stage-JsonTextDocument $publishedEffectCueText `
            $documentPaths.effectCues `
            'Valtan effect cue product'))
        $stagedEntries.Add((Stage-JsonTextDocument $publishedCombatObjectText `
            $documentPaths.combatObjects `
            'Valtan combat-object product'))
        $stagedEntries.Add((Stage-JsonTextDocument $publishedRotationText `
            $documentPaths.patternRotations `
            'Valtan pattern rotation product'))
        Assert-ProjectedProductSet $stagedEntries[0].Document $stagedEntries[1].Document `
            $stagedEntries[2].Document $stagedEntries[3].Document `
            $stagedEntries[4].Document @($encounterProjection) `
            @($animationProjection) @($rotationProjection) $cueOwners `
            $combatObjectLifeProjection @($master.retiredPatternIds)
        Commit-StagedDocuments @($stagedEntries)
    }
    catch {
        foreach ($entry in @($stagedEntries)) {
            if ([IO.File]::Exists($entry.StagePath)) { [IO.File]::Delete($entry.StagePath) }
            if ([IO.File]::Exists($entry.BackupPath)) { [IO.File]::Delete($entry.BackupPath) }
            if ([IO.File]::Exists($entry.ReplaceBackupPath)) {
                [IO.File]::Delete($entry.ReplaceBackupPath)
            }
        }
        throw
    }
    if (-not [string]::IsNullOrWhiteSpace($ReceiptPath)) {
        Write-ProjectionAtomically $projection $ReceiptPath
        Write-Host "Valtan pattern master audit projection published: $ReceiptPath"
    }
    Write-Host 'Valtan pattern master committed to Encounter, animation bindings, effect cues, combat objects, and normal-selection rotations.'
}

Write-Host (("Valtan pattern master {0} PASS: patterns={1} stages={2} " +
    "independentEffects={3} counterReactions={4} cueRefs={5} driftCheck={6}") -f `
    $Mode, @($master.patterns).Count, @($master.patterns.stages).Count,
    @($master.independentEffects).Count, @($master.counterReactionLayers).Count,
    $cueOwners.Count,
    $performDriftCheck)
