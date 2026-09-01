[CmdletBinding()]
param(
    [ValidateSet('Validate', 'Apply')]
    [string]$Mode = 'Validate',
    [Parameter(Mandatory = $true)]
    [string]$PatternId,
    [Parameter(Mandatory = $true)]
    [string]$EffectAssetId,
    [Parameter(Mandatory = $true)]
    [string]$CueIds,
    [string]$RepositoryRoot = '',
    [ValidateSet('None', 'SourceCommitPostReplace')]
    [string]$FailureInjection = 'None',
    [double]$WriterLockTimeoutSeconds = 30.0
)

$ErrorActionPreference = 'Stop'
$stableIdPattern = '^[A-Za-z0-9_.-]{1,160}$'
$utf8Strict = [Text.UTF8Encoding]::new($false, $true)
$utf8NoBom = [Text.UTF8Encoding]::new($false)
$script:sourceCommitPostReplaceFailureInjected = $false
$formatHelper = Join-Path $PSScriptRoot 'Format-PreservingJsonArray.ps1'
if (-not [IO.File]::Exists($formatHelper)) {
    throw "Missing format-preserving JSON helper: $formatHelper"
}
. $formatHelper

$repoRoot = if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    [IO.Path]::GetFullPath($RepositoryRoot)
}
if (-not [IO.Directory]::Exists($repoRoot)) {
    throw "RepositoryRoot does not exist: $repoRoot"
}
if ([double]::IsNaN($WriterLockTimeoutSeconds) -or
    [double]::IsInfinity($WriterLockTimeoutSeconds) -or
    $WriterLockTimeoutSeconds -lt 0.0) {
    throw 'WriterLockTimeoutSeconds must be finite and non-negative.'
}

function Acquire-ValtanCanonicalWriterLock {
    $lockPath = [IO.Path]::GetFullPath((Join-Path $repoRoot `
        'out\ValtanPatternTransactions\create-pattern.lock'))
    [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($lockPath)) |
        Out-Null
    $stream = [IO.FileStream]::new(
        $lockPath,
        [IO.FileMode]::OpenOrCreate,
        [IO.FileAccess]::ReadWrite,
        [IO.FileShare]::ReadWrite)
    try {
        if ($stream.Length -lt 1) {
            $stream.SetLength(1)
            $stream.Flush($true)
        }
        $deadline = [DateTime]::UtcNow.AddSeconds($WriterLockTimeoutSeconds)
        while ($true) {
            try {
                $stream.Lock(0L, 1L)
                $nonce = [Guid]::NewGuid().ToString('N')
                [byte[]]$markerBytes = [Text.Encoding]::ASCII.GetBytes(
                    "lostark.valtan-canonical-writer-owner-v1:${PID}:${nonce}`n")
                $stream.SetLength(1L + $markerBytes.Length)
                $stream.Position = 0L
                $stream.WriteByte(0)
                $stream.Position = 1L
                $stream.Write($markerBytes, 0, $markerBytes.Length)
                $stream.Flush($true)
                return [pscustomobject]@{
                    Stream = $stream
                    OwnerPid = $PID
                    Nonce = $nonce
                }
            }
            catch [IO.IOException] {
                if ([DateTime]::UtcNow -ge $deadline) {
                    throw 'Timed out waiting for the shared Valtan canonical writer admission.'
                }
                Start-Sleep -Milliseconds 25
            }
        }
    }
    catch {
        $stream.Dispose()
        throw
    }
}

function Assert-StableId([string]$Value, [string]$Context) {
    if ([string]::IsNullOrWhiteSpace($Value) -or
        $Value -cnotmatch $stableIdPattern) {
        throw "$Context is not a stable ID: '$Value'"
    }
}

Assert-StableId $PatternId 'PatternId'
Assert-StableId $EffectAssetId 'EffectAssetId'

$requestedCueIds = [Collections.Generic.List[string]]::new()
$requestedCueSet = [Collections.Generic.HashSet[string]]::new(
    [StringComparer]::Ordinal)
foreach ($rawCueId in @($CueIds.Split(','))) {
    $cueId = $rawCueId.Trim()
    Assert-StableId $cueId 'CueIds entry'
    if (-not $requestedCueSet.Add($cueId)) {
        throw "CueIds contains duplicate stable ID '$cueId'."
    }
    $requestedCueIds.Add($cueId)
}
if ($requestedCueIds.Count -eq 0) {
    throw 'CueIds must contain at least one stable cue ID.'
}

function Test-ByteArraysEqual([byte[]]$Left, [byte[]]$Right) {
    if ($null -eq $Left -or $null -eq $Right -or
        $Left.Length -ne $Right.Length) {
        return $false
    }
    for ($index = 0; $index -lt $Left.Length; ++$index) {
        if ($Left[$index] -ne $Right[$index]) { return $false }
    }
    return $true
}

function Read-StrictUtf8Text([string]$Path, [string]$Context) {
    if (-not [IO.File]::Exists($Path)) {
        throw "Missing ${Context}: $Path"
    }
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
    return [pscustomobject]@{
        Bytes = $bytes
        Text = $text
    }
}

function ConvertFrom-StrictJsonText(
    [string]$Text,
    [string]$Context) {
    try {
        return $Text | ConvertFrom-Json
    }
    catch {
        throw "$Context is not valid JSON.`n$($_.Exception.Message)"
    }
}

function Get-ExactProperty(
    [object]$Value,
    [string]$PropertyName,
    [string]$Context) {
    if ($null -eq $Value) { throw "$Context is null." }
    $matches = @($Value.PSObject.Properties | Where-Object {
        [string]$_.Name -ceq $PropertyName
    })
    if ($matches.Count -ne 1) {
        throw "$Context must contain exact property '$PropertyName'."
    }
    return $matches[0]
}

function Assert-JsonArray(
    [object]$Value,
    [string]$Context) {
    if ($Value -isnot [Array]) {
        throw "$Context must be a JSON array."
    }
}

function Get-CueFingerprint(
    [object]$Cue,
    [string]$Pattern,
    [string]$Stage) {
    $canonical = ConvertTo-FormatPreservingCanonicalNode $Cue |
        ConvertTo-Json -Depth 100 -Compress
    return $Pattern + "`n" + $Stage + "`n" + $canonical
}

function Get-PresentationIndex(
    [object]$Document,
    [string]$Context) {
    $schema = Get-ExactProperty $Document 'schema' $Context
    if ($schema.Value -isnot [string] -or
        [string]$schema.Value -cne 'lostark.valtan-pattern-presentation-authoring') {
        throw "$Context has an unsupported schema."
    }
    $formatVersion = Get-ExactProperty $Document 'formatVersion' $Context
    if ([int]$formatVersion.Value -ne 1) {
        throw "$Context has an unsupported formatVersion."
    }
    $patternsProperty = Get-ExactProperty $Document 'patterns' $Context
    Assert-JsonArray $patternsProperty.Value "$Context patterns"

    $patternIds = [Collections.Generic.HashSet[string]]::new(
        [StringComparer]::Ordinal)
    $patternsById =
        [Collections.Generic.Dictionary[string,object]]::new(
            [StringComparer]::Ordinal)
    $cuesById =
        [Collections.Generic.Dictionary[string,object]]::new(
            [StringComparer]::Ordinal)
    $cueEntries = [Collections.Generic.List[object]]::new()

    foreach ($pattern in @($patternsProperty.Value)) {
        $patternIdProperty = Get-ExactProperty $pattern 'patternId' `
            "$Context pattern"
        if ($patternIdProperty.Value -isnot [string]) {
            throw "$Context patternId must be a JSON string."
        }
        $currentPatternId = [string]$patternIdProperty.Value
        Assert-StableId $currentPatternId "$Context patternId"
        if (-not $patternIds.Add($currentPatternId)) {
            throw "$Context contains duplicate patternId '$currentPatternId'."
        }
        $patternsById[$currentPatternId] = $pattern

        $stagesProperty = Get-ExactProperty $pattern 'stages' `
            "$Context pattern '$currentPatternId'"
        Assert-JsonArray $stagesProperty.Value `
            "$Context pattern '$currentPatternId' stages"
        $stageIds = [Collections.Generic.HashSet[string]]::new(
            [StringComparer]::Ordinal)
        foreach ($stage in @($stagesProperty.Value)) {
            $stageIdProperty = Get-ExactProperty $stage 'stageId' `
                "$Context pattern '$currentPatternId' stage"
            if ($stageIdProperty.Value -isnot [string]) {
                throw "$Context stageId must be a JSON string."
            }
            $stageId = [string]$stageIdProperty.Value
            Assert-StableId $stageId "$Context stageId"
            if (-not $stageIds.Add($stageId)) {
                throw "$Context pattern '$currentPatternId' contains duplicate stageId '$stageId'."
            }
            $effectCuesProperty = Get-ExactProperty $stage 'effectCues' `
                "$Context pattern '$currentPatternId' stage '$stageId'"
            Assert-JsonArray $effectCuesProperty.Value `
                "$Context pattern '$currentPatternId' stage '$stageId' effectCues"
            foreach ($cue in @($effectCuesProperty.Value)) {
                $cueIdProperty = Get-ExactProperty $cue 'cueId' `
                    "$Context Effect cue"
                $assetIdProperty = Get-ExactProperty $cue 'effectAssetId' `
                    "$Context Effect cue"
                if ($cueIdProperty.Value -isnot [string] -or
                    $assetIdProperty.Value -isnot [string]) {
                    throw "$Context Effect cue IDs must be JSON strings."
                }
                $cueId = [string]$cueIdProperty.Value
                $assetId = [string]$assetIdProperty.Value
                Assert-StableId $cueId "$Context cueId"
                Assert-StableId $assetId "$Context effectAssetId"
                if ($cuesById.ContainsKey($cueId)) {
                    throw "$Context contains duplicate cueId '$cueId'."
                }
                $entry = [pscustomobject]@{
                    CueId = $cueId
                    EffectAssetId = $assetId
                    PatternId = $currentPatternId
                    StageId = $stageId
                    Cue = $cue
                    Fingerprint = Get-CueFingerprint `
                        $cue $currentPatternId $stageId
                }
                $cuesById[$cueId] = $entry
                $cueEntries.Add($entry)
            }
        }
    }
    return [pscustomobject]@{
        PatternsById = $patternsById
        PatternIds = $patternIds
        CuesById = $cuesById
        Cues = @($cueEntries)
    }
}

function Assert-RequestedCueSet(
    [object]$Index,
    [string]$Context) {
    if (-not $Index.PatternsById.ContainsKey($PatternId)) {
        throw "$Context does not contain pattern '$PatternId'."
    }
    [string[]]$actualIds = @($Index.Cues | Where-Object {
        [string]$_.PatternId -ceq $PatternId -and
        [string]$_.EffectAssetId -ceq $EffectAssetId
    } | ForEach-Object { [string]$_.CueId })
    [string[]]$expectedIds = @($requestedCueIds)
    [Array]::Sort($actualIds, [StringComparer]::Ordinal)
    [Array]::Sort($expectedIds, [StringComparer]::Ordinal)
    if (($actualIds -join "`n") -cne ($expectedIds -join "`n")) {
        throw (("Requested cueId set is not the exact set for pattern '{0}' and Effect '{1}'. " +
            'expected=[{2}] actual=[{3}]') -f $PatternId, $EffectAssetId,
            ($expectedIds -join ','), ($actualIds -join ','))
    }
}

function Resolve-TargetEffectSnapshot {
    $catalogPath = [IO.Path]::GetFullPath(
        (Join-Path $repoRoot 'Data\Effects\EffectCatalog.json'))
    $catalogSource = Read-StrictUtf8Text $catalogPath 'Effect catalog'
    $catalog = ConvertFrom-StrictJsonText $catalogSource.Text 'Effect catalog'
    $effectsProperty = Get-ExactProperty $catalog 'effects' 'Effect catalog'
    Assert-JsonArray $effectsProperty.Value 'Effect catalog effects'
    $rows = @($effectsProperty.Value | Where-Object {
        $id = Get-ExactProperty $_ 'effectAssetId' 'Effect catalog row'
        $id.Value -is [string] -and
            [string]$id.Value -ceq $EffectAssetId
    })
    if ($rows.Count -ne 1) {
        throw "Effect catalog must contain exactly one '$EffectAssetId' row."
    }
    $authoringPathProperty = Get-ExactProperty $rows[0] 'authoringPath' `
        "Effect catalog row '$EffectAssetId'"
    if ($authoringPathProperty.Value -isnot [string]) {
        throw "Effect catalog authoringPath for '$EffectAssetId' must be a string."
    }
    $relativePath = [string]$authoringPathProperty.Value
    if ([string]::IsNullOrWhiteSpace($relativePath) -or
        [IO.Path]::IsPathRooted($relativePath) -or
        $relativePath.Contains('\') -or
        @($relativePath.Split('/')) -contains '..') {
        throw "Effect catalog authoringPath is not a safe Data-relative path: '$relativePath'"
    }
    $dataRoot = [IO.Path]::GetFullPath((Join-Path $repoRoot 'Data'))
    $effectPath = [IO.Path]::GetFullPath((Join-Path $dataRoot $relativePath))
    $dataPrefix = $dataRoot.TrimEnd('\', '/') +
        [IO.Path]::DirectorySeparatorChar
    if (-not $effectPath.StartsWith(
            $dataPrefix, [StringComparison]::OrdinalIgnoreCase) -or
        -not [IO.File]::Exists($effectPath)) {
        throw "Effect authored document does not resolve inside Data: '$relativePath'"
    }
    return [pscustomobject]@{
        CatalogPath = $catalogPath
        CatalogBytes = [byte[]]$catalogSource.Bytes
        EffectPath = $effectPath
        EffectBytes = [IO.File]::ReadAllBytes($effectPath)
    }
}

function Assert-TargetEffectSnapshotUnchanged(
    [object]$Snapshot,
    [string]$Context) {
    if (-not (Test-ByteArraysEqual $Snapshot.CatalogBytes `
            ([IO.File]::ReadAllBytes($Snapshot.CatalogPath)))) {
        throw "$Context changed EffectCatalog.json; Product Effect unlink must not edit it."
    }
    if (-not (Test-ByteArraysEqual $Snapshot.EffectBytes `
            ([IO.File]::ReadAllBytes($Snapshot.EffectPath)))) {
        throw "$Context changed the shared authored Effect; Product Effect unlink must not edit it."
    }
}

function Assert-CurrentBytes(
    [string]$Path,
    [byte[]]$ExpectedBytes,
    [string]$Context) {
    if (-not [IO.File]::Exists($Path) -or
        -not (Test-ByteArraysEqual `
            ([IO.File]::ReadAllBytes($Path)) $ExpectedBytes)) {
        throw "$Context failed raw-byte CAS because the source changed concurrently."
    }
}

function Replace-FileBytesWithCas(
    [string]$Path,
    [byte[]]$ExpectedBytes,
    [byte[]]$ReplacementBytes,
    [string]$Context,
    [ref]$OutReplaced,
    [ref]$OutRecoveryBackupPath) {
    $OutReplaced.Value = $false
    $OutRecoveryBackupPath.Value = ''
    Assert-CurrentBytes $Path $ExpectedBytes $Context
    $directory = [IO.Path]::GetDirectoryName($Path)
    $stagePath = Join-Path $directory ('.' + [IO.Path]::GetFileName($Path) +
        '.stage.' + [Guid]::NewGuid().ToString('N'))
    $replaceBackupPath = $Path + '.replace-backup.' +
        [Guid]::NewGuid().ToString('N')
    try {
        [IO.File]::WriteAllBytes($stagePath, $ReplacementBytes)
        Assert-CurrentBytes $stagePath $ReplacementBytes "$Context stage"
        Assert-CurrentBytes $Path $ExpectedBytes $Context
        [IO.File]::Replace($stagePath, $Path, $replaceBackupPath, $true)
        $OutReplaced.Value = $true
        $OutRecoveryBackupPath.Value = $replaceBackupPath
        if ($FailureInjection -eq 'SourceCommitPostReplace' -and
            $Context -ceq 'Valtan Pattern Effect unlink source commit' -and
            -not $script:sourceCommitPostReplaceFailureInjected) {
            $script:sourceCommitPostReplaceFailureInjected = $true
            throw 'injected source commit post-replace verification failure'
        }
        Assert-CurrentBytes $Path $ReplacementBytes "$Context commit"
        if ([IO.File]::Exists($replaceBackupPath)) {
            [IO.File]::Delete($replaceBackupPath)
        }
        $OutRecoveryBackupPath.Value = ''
    }
    finally {
        if ([IO.File]::Exists($stagePath)) { [IO.File]::Delete($stagePath) }
    }
}

function Invoke-ValtanProjector([string]$ProjectorMode) {
    $projector = [IO.Path]::GetFullPath((Join-Path $repoRoot `
        'Tools\ValtanPipeline\Project-ValtanPatternMaster.ps1'))
    if (-not [IO.File]::Exists($projector)) {
        throw "Missing Valtan split projector: $projector"
    }
    try {
        if ($ProjectorMode -ceq 'PublishV2') {
            & $projector -Mode $ProjectorMode -RepositoryRoot $repoRoot `
                -WriterLockAlreadyHeld `
                -WriterLockOwnerNonce ([string]$writerLock.Nonce)
        }
        else {
            & $projector -Mode $ProjectorMode -RepositoryRoot $repoRoot
        }
    }
    catch {
        throw "Valtan split projector $ProjectorMode failed: $($_.Exception.Message)"
    }
}

$presentationPath = [IO.Path]::GetFullPath((Join-Path $repoRoot `
    'Data\Valtan\Valtan.presentation.json'))
$source = Read-StrictUtf8Text $presentationPath 'Valtan presentation authoring'
$document = ConvertFrom-StrictJsonText $source.Text `
    'Valtan presentation authoring'
$baselineIndex = Get-PresentationIndex $document `
    'Valtan presentation authoring'
Assert-RequestedCueSet $baselineIndex 'Valtan presentation authoring'
$effectSnapshot = Resolve-TargetEffectSnapshot

$targetPattern = $baselineIndex.PatternsById[$PatternId]
$stages = (Get-ExactProperty $targetPattern 'stages' `
    "Valtan pattern '$PatternId'").Value
foreach ($stage in @($stages)) {
    $effectCuesProperty = Get-ExactProperty $stage 'effectCues' `
        "Valtan pattern '$PatternId' stage"
    $remaining = [Collections.Generic.List[object]]::new()
    foreach ($cue in @($effectCuesProperty.Value)) {
        $cueId = [string](Get-ExactProperty $cue 'cueId' 'Valtan Effect cue').Value
        if (-not $requestedCueSet.Contains($cueId)) { $remaining.Add($cue) }
    }
    $effectCuesProperty.Value = @($remaining)
}

$candidateText = Update-FormatPreservingJsonArrayRows `
    -Text $source.Text `
    -ArrayProperty 'patterns' `
    -KeyProperty 'patternId' `
    -DesiredRows @($targetPattern)
[byte[]]$candidateBytes = $utf8NoBom.GetBytes($candidateText)
$candidateDocument = ConvertFrom-StrictJsonText $candidateText `
    'candidate Valtan presentation authoring'
$candidateIndex = Get-PresentationIndex $candidateDocument `
    'candidate Valtan presentation authoring'
if ($candidateIndex.PatternIds.Count -ne $baselineIndex.PatternIds.Count -or
    -not $candidateIndex.PatternIds.SetEquals($baselineIndex.PatternIds)) {
    throw 'Candidate Valtan presentation changed the pattern ID set.'
}
foreach ($cueId in @($requestedCueIds)) {
    if ($candidateIndex.CuesById.ContainsKey($cueId)) {
        throw "Candidate Valtan presentation still contains removed cueId '$cueId'."
    }
}
if (@($candidateIndex.Cues | Where-Object {
        [string]$_.PatternId -ceq $PatternId -and
        [string]$_.EffectAssetId -ceq $EffectAssetId
    }).Count -ne 0) {
    throw 'Candidate Valtan presentation still contains a target Pattern/Effect cue.'
}
foreach ($entry in @($baselineIndex.Cues)) {
    if ($requestedCueSet.Contains([string]$entry.CueId)) { continue }
    if (-not $candidateIndex.CuesById.ContainsKey([string]$entry.CueId) -or
        [string]$candidateIndex.CuesById[[string]$entry.CueId].Fingerprint -cne
            [string]$entry.Fingerprint) {
        throw "Candidate Valtan presentation changed unrelated cueId '$($entry.CueId)'."
    }
}
if ($candidateIndex.CuesById.Count -ne
    $baselineIndex.CuesById.Count - $requestedCueIds.Count) {
    throw 'Candidate Valtan presentation removed an unexpected Effect cue.'
}

$writerLock = $null
$writerLockHeld = $false
try {
    $writerLock = Acquire-ValtanCanonicalWriterLock
    $writerLockHeld = $true

    Assert-CurrentBytes $presentationPath $source.Bytes `
        'Valtan Pattern Effect unlink preflight'
    Assert-TargetEffectSnapshotUnchanged $effectSnapshot `
        'Valtan Pattern Effect unlink preflight'

    if ($Mode -eq 'Validate') {
        Invoke-ValtanProjector 'Validate'
        Assert-CurrentBytes $presentationPath $source.Bytes `
            'Valtan Pattern Effect unlink validation'
        Assert-TargetEffectSnapshotUnchanged $effectSnapshot `
            'Valtan Pattern Effect unlink validation'
        Write-Host (("Valtan Pattern Effect unlink candidate is structurally valid and " +
            "the current split graph passed Validate: pattern={0} effect={1} cues={2}. " +
            "Apply validates and publishes the candidate.") -f
            $PatternId, $EffectAssetId, $requestedCueIds.Count)
        return
    }

    $sourceCommitted = $false
    $sourceCommitBackupPath = ''
    $publishAttempted = $false
    try {
        Replace-FileBytesWithCas $presentationPath $source.Bytes $candidateBytes `
            'Valtan Pattern Effect unlink source commit' `
            ([ref]$sourceCommitted) ([ref]$sourceCommitBackupPath)

        $publishAttempted = $true
        Invoke-ValtanProjector 'PublishV2'
        Assert-CurrentBytes $presentationPath $candidateBytes `
            'Valtan Pattern Effect unlink post-publish'
        Assert-TargetEffectSnapshotUnchanged $effectSnapshot `
            'Valtan Pattern Effect unlink post-publish'

        Invoke-ValtanProjector 'Validate'
        Assert-CurrentBytes $presentationPath $candidateBytes `
            'Valtan Pattern Effect unlink post-validation'
        Assert-TargetEffectSnapshotUnchanged $effectSnapshot `
            'Valtan Pattern Effect unlink post-validation'
        $sourceCommitted = $false
        Write-Host (("Valtan Pattern Effect link removed: pattern={0} effect={1} " +
            "cues={2}; shared Effect asset and catalog were preserved.") -f
            $PatternId, $EffectAssetId, $requestedCueIds.Count)
    }
    catch {
        $operationFailure = $_.Exception.Message
        $recoveryFailure = ''
        if ($sourceCommitted) {
            try {
                $rollbackCommitted = $false
                $rollbackBackupPath = ''
                Replace-FileBytesWithCas $presentationPath $candidateBytes `
                    $source.Bytes 'Valtan Pattern Effect unlink source rollback' `
                    ([ref]$rollbackCommitted) ([ref]$rollbackBackupPath)
                if (-not $rollbackCommitted) {
                    throw 'Source rollback did not reach its atomic replace boundary.'
                }
                $sourceCommitted = $false
                if ($publishAttempted) {
                    Invoke-ValtanProjector 'PublishV2'
                }
                Assert-CurrentBytes $presentationPath $source.Bytes `
                    'Valtan Pattern Effect unlink rollback verification'
                if (-not [string]::IsNullOrWhiteSpace($sourceCommitBackupPath) -and
                    [IO.File]::Exists($sourceCommitBackupPath)) {
                    [IO.File]::Delete($sourceCommitBackupPath)
                }
                $sourceCommitBackupPath = ''
                Assert-TargetEffectSnapshotUnchanged $effectSnapshot `
                    'Valtan Pattern Effect unlink rollback'
            }
            catch {
                $recoveryFailure = $_.Exception.Message
            }
        }
        elseif (-not [IO.File]::Exists($presentationPath) -or
            -not (Test-ByteArraysEqual `
                ([IO.File]::ReadAllBytes($presentationPath)) $source.Bytes)) {
            $recoveryFailure =
                'Source is not the baseline even though no completed replace was observed.'
        }
        if (-not [string]::IsNullOrWhiteSpace($recoveryFailure)) {
            if (-not [string]::IsNullOrWhiteSpace($sourceCommitBackupPath) -and
                [IO.File]::Exists($sourceCommitBackupPath)) {
                $recoveryFailure +=
                    " A recovery backup was preserved at '$sourceCommitBackupPath'."
            }
            throw ("Valtan Pattern Effect unlink failed: {0} Rollback/recovery also failed: {1}" -f
                $operationFailure, $recoveryFailure)
        }
        throw "Valtan Pattern Effect unlink failed and source was restored: $operationFailure"
    }
}
finally {
    if ($writerLockHeld) {
        try {
            $writerLock.Stream.SetLength(1L)
            $writerLock.Stream.Position = 0L
            $writerLock.Stream.WriteByte(0)
            $writerLock.Stream.Flush($true)
            $writerLock.Stream.Unlock(0L, 1L)
        }
        finally { $writerLock.Stream.Dispose() }
    }
    elseif ($null -ne $writerLock) {
        $writerLock.Stream.Dispose()
    }
}
