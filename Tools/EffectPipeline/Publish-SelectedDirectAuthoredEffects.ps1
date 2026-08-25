param(
    [ValidateSet('Validate', 'Publish')]
    [string]$Mode = 'Validate',
    [Parameter(Mandatory = $true)]
    [string[]]$EffectAssetId,
    [string]$DataRoot,
    [string]$ResourceRoot,
    [string]$RuntimeRoot,
    [ValidateSet(
        'None',
        'AfterSealInstall',
        'AfterCatalogBackup',
        'AfterCatalogCommit')]
    [string]$TestFaultInjection = 'None'
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
if ([string]::IsNullOrWhiteSpace($DataRoot)) {
    $DataRoot = Join-Path $repoRoot 'Data'
}
if ([string]::IsNullOrWhiteSpace($ResourceRoot)) {
    $ResourceRoot = Join-Path $repoRoot 'Client\Bin\Resources'
}
if ([string]::IsNullOrWhiteSpace($RuntimeRoot)) {
    $RuntimeRoot = Join-Path $repoRoot 'Client\Bin\DataFiles\Effect'
}
$DataRoot = [IO.Path]::GetFullPath($DataRoot)
$ResourceRoot = [IO.Path]::GetFullPath($ResourceRoot)
$RuntimeRoot = [IO.Path]::GetFullPath($RuntimeRoot)
$sourceCatalogPath = Join-Path $DataRoot 'Effects\EffectCatalog.json'
$sourceAuthoredRoot = Join-Path $DataRoot 'Effects\Authored'
$runtimeCatalogPath = Join-Path $RuntimeRoot 'EffectCatalog.runtime.json'
$runtimeAuthoredRoot = Join-Path $RuntimeRoot 'Authored'
$visualProgramPath = Join-Path $RuntimeRoot 'EffectVisualPrograms.runtime.json'
$compactTool = Join-Path $PSScriptRoot 'compact_effect_document.py'
$runtimeValidator = Join-Path $PSScriptRoot `
    'validate_direct_authored_effect_runtime.py'
$utf8NoBomStrict = [Text.UTF8Encoding]::new($false, $true)
$utf8NoBom = [Text.UTF8Encoding]::new($false)
$allowedTargetIds = [Collections.Generic.HashSet[string]]::new(
    [StringComparer]::Ordinal)
[void]$allowedTargetIds.Add('effect.artist.skill.31420.unified')
[void]$allowedTargetIds.Add('effect.artist.skill.31460.linear-reveal.unified')
[void]$allowedTargetIds.Add('effect.artist.skill.31000.ba1.unified')
[void]$allowedTargetIds.Add('effect.artist.skill.31000.ba2.unified')
[void]$allowedTargetIds.Add('effect.artist.skill.31000.ba3.unified')
[void]$allowedTargetIds.Add('effect.artist.skill.31000.ba4.unified')
[void]$allowedTargetIds.Add('effect.artist.skill.31050.clip2.unified')
[void]$allowedTargetIds.Add('effect.artist.skill.31200.unified')
[void]$allowedTargetIds.Add('effect.artist.skill.31210.ba1.unified')
[void]$allowedTargetIds.Add('effect.artist.skill.31210.ba4.unified')
[void]$allowedTargetIds.Add('effect.dimensionmaster.skill.2050010.ba2.unified')
[void]$allowedTargetIds.Add('effect.dimensionmaster.skill.2050010.ba3.unified')
[void]$allowedTargetIds.Add('effect.dimensionmaster.skill.2050100.unified')
[void]$allowedTargetIds.Add('effect.dimensionmaster.skill.2050180.unified')
[void]$allowedTargetIds.Add('effect.dimensionmaster.skill.2050210.a1.unified')
[void]$allowedTargetIds.Add('effect.dimensionmaster.skill.2050220.unified')
[void]$allowedTargetIds.Add('effect.dimensionmaster.skill.2050240.clip2.unified')
[void]$allowedTargetIds.Add('effect.dimensionmaster.skill.2050500.unified')
[void]$allowedTargetIds.Add('effect.lancemaster.skill.34010.ba1.unified')
[void]$allowedTargetIds.Add('effect.lancemaster.skill.34010.ba2.unified')
[void]$allowedTargetIds.Add('effect.lancemaster.skill.34010.ba3.unified')
[void]$allowedTargetIds.Add('effect.lancemaster.skill.34010.ba4.unified')
[void]$allowedTargetIds.Add('effect.lancemaster.skill.34040.clip1.unified')
[void]$allowedTargetIds.Add('effect.lancemaster.skill.34040.clip2.unified')
[void]$allowedTargetIds.Add('effect.lancemaster.skill.34090.unified')
[void]$allowedTargetIds.Add('effect.lancemaster.skill.34100.clip1.unified')
[void]$allowedTargetIds.Add('effect.lancemaster.skill.34100.clip2.unified')
[void]$allowedTargetIds.Add('effect.lancemaster.skill.34100.clip3.unified')
[void]$allowedTargetIds.Add('effect.lancemaster.skill.34110.unified')
[void]$allowedTargetIds.Add('effect.lancemaster.skill.34110.v1.unified')
[void]$allowedTargetIds.Add('effect.lancemaster.skill.34120.clip1.unified')
[void]$allowedTargetIds.Add('effect.lancemaster.skill.34120.clip2.unified')
[void]$allowedTargetIds.Add('effect.lancemaster.skill.34120.clip3.unified')
[void]$allowedTargetIds.Add(
    'effect.lancemaster.skill.34140.ba1.clip1.unified')
[void]$allowedTargetIds.Add(
    'effect.lancemaster.skill.34140.ba2.clip2.unified')
[void]$allowedTargetIds.Add('effect.lancemaster.skill.34150.unified')
[void]$allowedTargetIds.Add('effect.lancemaster.skill.34160.ba1.unified')
[void]$allowedTargetIds.Add('effect.lancemaster.skill.34610.clip1.unified')
[void]$allowedTargetIds.Add('effect.lancemaster.skill.34610.clip2.unified')
[void]$allowedTargetIds.Add('effect.lancemaster.skill.34610.clip3.unified')
[void]$allowedTargetIds.Add('effect.lancemaster.skill.34650.clip1.unified')
[void]$allowedTargetIds.Add('effect.warlord.skill.17100.unified')
[void]$allowedTargetIds.Add('effect.warlord.skill.17110.clip2.unified')
[void]$allowedTargetIds.Add('effect.warlord.skill.17110.clip3.unified')
[void]$allowedTargetIds.Add('effect.warlord.skill.17140.unified')
[void]$allowedTargetIds.Add('effect.warlord.skill.17170.clip1.unified')
[void]$allowedTargetIds.Add('effect.warlord.skill.17170.clip2.unified')
[void]$allowedTargetIds.Add('effect.warlord.skill.17170.clip3.unified')
[void]$allowedTargetIds.Add('effect.warlord.skill.17240.ba1.unified')
[void]$allowedTargetIds.Add('effect.warlord.skill.17240.ba2.unified')
[void]$allowedTargetIds.Add('effect.warlord.skill.17240.ba3.unified')
[void]$allowedTargetIds.Add(
    'effect.valtan.carrier-v1.attack.fist-in-out.inner.clip-01')
[void]$allowedTargetIds.Add(
    'effect.valtan.carrier-v1.attack.high-jump.takeoff.clip-01')
[void]$allowedTargetIds.Add(
    'effect.valtan.carrier-v1.attack.high-jump.land.clip-01')
[void]$allowedTargetIds.Add('effect.valtan.sky-axe.active')

function Test-PathIsSameOrDescendant(
    [string]$Path,
    [string]$Root) {
    $fullPath = [IO.Path]::GetFullPath($Path)
    $fullRoot = [IO.Path]::GetFullPath($Root)
    if ($fullPath.Equals($fullRoot, [StringComparison]::OrdinalIgnoreCase)) {
        return $true
    }
    $prefix = $fullRoot.TrimEnd(
        [IO.Path]::DirectorySeparatorChar,
        [IO.Path]::AltDirectorySeparatorChar) +
        [IO.Path]::DirectorySeparatorChar
    return $fullPath.StartsWith(
        $prefix, [StringComparison]::OrdinalIgnoreCase)
}

function Assert-RegularDirectory([string]$Path, [string]$Label) {
    $item = Get-Item -LiteralPath $Path -Force -ErrorAction SilentlyContinue
    if ($null -eq $item -or -not [bool]$item.PSIsContainer -or
        ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0 -or
        -not [IO.Directory]::Exists($Path)) {
        throw "$Label must be a regular non-reparse directory: $Path"
    }
}

function Assert-RegularFile([string]$Path, [string]$Label) {
    $item = Get-Item -LiteralPath $Path -Force -ErrorAction SilentlyContinue
    if ($null -eq $item -or [bool]$item.PSIsContainer -or
        ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0 -or
        -not [IO.File]::Exists($Path)) {
        throw "$Label must be a regular non-reparse file: $Path"
    }
}

function Assert-AbsentOrRegularFile([string]$Path, [string]$Label) {
    $item = Get-Item -LiteralPath $Path -Force -ErrorAction SilentlyContinue
    if ($null -eq $item) {
        return
    }
    if ([bool]$item.PSIsContainer -or
        ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0 -or
        -not [IO.File]::Exists($Path)) {
        throw "$Label must be absent or a regular non-reparse file: $Path"
    }
}

function Get-Sha256Hex([byte[]]$Payload) {
    $sha = [Security.Cryptography.SHA256]::Create()
    try {
        return ([BitConverter]::ToString(
            $sha.ComputeHash($Payload))).Replace('-', '').ToLowerInvariant()
    }
    finally {
        $sha.Dispose()
    }
}

function Test-ByteArrayEqual([byte[]]$Left, [byte[]]$Right) {
    return [Collections.StructuralComparisons]::StructuralEqualityComparer.Equals(
        $Left, $Right)
}

function Read-Utf8Json(
    [string]$Path,
    [string]$Label,
    [bool]$RequireLf = $false) {
    Assert-RegularFile $Path $Label
    $bytes = [IO.File]::ReadAllBytes($Path)
    if (0 -eq $bytes.LongLength -or
        ($bytes.LongLength -ge 3 -and $bytes[0] -eq 0xef -and
         $bytes[1] -eq 0xbb -and $bytes[2] -eq 0xbf)) {
        throw "$Label must be non-empty UTF-8 without BOM: $Path"
    }
    $text = $utf8NoBomStrict.GetString($bytes)
    if ($RequireLf -and $text.Contains("`r")) {
        throw "$Label must use LF line endings: $Path"
    }
    try {
        return [pscustomobject]@{
            Bytes = $bytes
            Text = $text
            Json = ($text | ConvertFrom-Json)
        }
    }
    catch {
        throw "$Label is not valid JSON: $Path ($($_.Exception.Message))"
    }
}

function Get-PropertyNames([object]$Value) {
    return @($Value.PSObject.Properties | ForEach-Object { $_.Name })
}

function Assert-ExactPropertyOrder(
    [object]$Value,
    [string[]]$Expected,
    [string]$Label) {
    $actual = @(Get-PropertyNames $Value)
    if ($actual.Count -ne $Expected.Count) {
        throw "$Label fields or order are invalid."
    }
    for ($index = 0; $index -lt $Expected.Count; ++$index) {
        if ($actual[$index] -cne $Expected[$index]) {
            throw "$Label fields or order are invalid."
        }
    }
}

function Get-JsonFingerprint([object]$Value) {
    return ($Value | ConvertTo-Json -Depth 100 -Compress)
}

function Assert-StableId([string]$Value, [string]$Label) {
    if ([string]::IsNullOrWhiteSpace($Value) -or $Value.Length -gt 255 -or
        $Value -cnotmatch '^[a-z0-9]+(?:[._-][a-z0-9]+)*$') {
        throw "$Label is not a stable ID: $Value"
    }
}

function Resolve-SafeRelativeFile(
    [string]$Root,
    [string]$RelativePath,
    [string]$Label) {
    if ([string]::IsNullOrWhiteSpace($RelativePath) -or
        [IO.Path]::IsPathRooted($RelativePath) -or
        $RelativePath.Contains('\') -or $RelativePath.Contains(':') -or
        $RelativePath.Contains('//')) {
        throw "$Label is not a safe forward-slash relative path: $RelativePath"
    }
    foreach ($segment in $RelativePath.Split('/')) {
        if ([string]::IsNullOrWhiteSpace($segment) -or
            $segment -eq '.' -or $segment -eq '..') {
            throw "$Label contains an unsafe segment: $RelativePath"
        }
    }
    $resolved = [IO.Path]::GetFullPath((Join-Path `
        $Root $RelativePath.Replace('/', '\')))
    if (-not (Test-PathIsSameOrDescendant $resolved $Root)) {
        throw "$Label escaped its root: $RelativePath"
    }
    Assert-RegularFile $resolved $Label
    return $resolved
}

function Invoke-RuntimeValidator([string]$CatalogPath) {
    $python = Get-Command python -ErrorAction SilentlyContinue
    if ($null -eq $python) {
        throw 'Python is required for selected Effect runtime validation.'
    }
    & $python.Source -B $runtimeValidator --catalog $CatalogPath
    if ($LASTEXITCODE -ne 0) {
        throw "Selected Effect runtime validation failed: $CatalogPath"
    }
}

function Invoke-Compactor([string]$Source, [string]$Destination) {
    $python = Get-Command python -ErrorAction SilentlyContinue
    if ($null -eq $python) {
        throw 'Python is required to compact selected Effect documents.'
    }
    & $python.Source -B $compactTool $Source $Destination
    if ($LASTEXITCODE -ne 0) {
        throw "Selected Effect document compaction failed: $Source"
    }
}

function Assert-AuthoredDocument(
    [string]$EffectId,
    [string]$Path) {
    $loaded = Read-Utf8Json $Path "Selected authored Effect '$EffectId'"
    $document = $loaded.Json
    Assert-ExactPropertyOrder $document @(
        'schema', 'version', 'effectAssetId', 'displayName',
        'particleSystem', 'modelCues', 'elements') `
        "Selected authored Effect '$EffectId'"
    if ([string]$document.schema -cne 'lostark.effect-authoring' -or
        [int]$document.version -ne 13 -or
        [string]$document.effectAssetId -cne $EffectId -or
        $null -eq $document.particleSystem -or
        $null -eq $document.modelCues -or $null -eq $document.elements -or
        @($document.elements).Count -lt 1) {
        throw "Selected authored Effect identity or v13 structure is invalid: $EffectId"
    }
    $elementIds = [Collections.Generic.HashSet[string]]::new(
        [StringComparer]::Ordinal)
    $hasVisibleElement = $false
    foreach ($element in @($document.elements)) {
        $elementId = [string]$element.id
        Assert-StableId $elementId "Element ID in $EffectId"
        if (-not $elementIds.Add($elementId)) {
            throw "Duplicate Element ID in ${EffectId}: $elementId"
        }
        if ($element.visible -isnot [bool]) {
            throw "Element visible flag is not Boolean in ${EffectId}: $elementId"
        }
        if ([bool]$element.visible) {
            $hasVisibleElement = $true
        }
        foreach ($resource in @($element.resources)) {
            $slotId = [string]$resource.slotId
            $assetId = [string]$resource.assetId
            if ([string]::IsNullOrWhiteSpace($slotId) -or
                [string]::IsNullOrWhiteSpace($assetId)) {
                throw "Element resource identity is invalid in ${EffectId}: $elementId"
            }
            $extension = [IO.Path]::GetExtension($assetId).ToLowerInvariant()
            if ($slotId -ceq 'meshModel') {
                if ($extension -cne '.wmodel') {
                    throw "Element resource extension is invalid in ${EffectId}: $elementId/$slotId"
                }
            }
            elseif ($extension -cne '.dds' -or
                -not $assetId.StartsWith(
                    'Effect/', [StringComparison]::Ordinal)) {
                throw "Element resource extension is invalid in ${EffectId}: $elementId/$slotId"
            }
            [void](Resolve-SafeRelativeFile `
                $ResourceRoot $assetId "Effect resource '$assetId'")
        }
    }
    if (-not $hasVisibleElement) {
        throw "Selected authored Effect has no visible Element: $EffectId"
    }
    foreach ($cue in @($document.modelCues)) {
        $modelAssetId = [string]$cue.modelAssetId
        if ([string]::IsNullOrWhiteSpace($modelAssetId) -or
            [IO.Path]::GetExtension($modelAssetId).ToLowerInvariant() -cne
                '.wmodel') {
            throw "Model Cue resource identity is invalid in $EffectId"
        }
        [void](Resolve-SafeRelativeFile `
            $ResourceRoot $modelAssetId "Effect Model Cue '$modelAssetId'")
    }
    return $loaded
}

function Assert-RuntimeDirectSeals([object]$Catalog) {
    foreach ($entry in @($Catalog.effects)) {
        if ([string]$entry.payloadKind -cne
                'DIRECT_AUTHORED_DOCUMENT_V13') {
            continue
        }
        $effectId = [string]$entry.effectAssetId
        $relativePath = [string]$entry.authoredDocumentPath
        $escapedId = [regex]::Escape($effectId)
        if ($relativePath -cnotmatch
                "^Authored/$escapedId\.([0-9a-f]{64})\.effect\.json$") {
            throw "Runtime direct-authored seal path is invalid: $effectId"
        }
        $expectedSha = [string]$Matches[1]
        $sealedPath = Resolve-SafeRelativeFile `
            $RuntimeRoot $relativePath "Runtime authored seal '$effectId'"
        $actualSha = Get-Sha256Hex ([IO.File]::ReadAllBytes($sealedPath))
        if ($actualSha -cne $expectedSha) {
            throw "Runtime authored seal content hash mismatched: $effectId"
        }
    }
}

function Assert-NoTargetBindings(
    [object]$RuntimeCatalog,
    [object]$VisualPrograms,
    [Collections.Generic.HashSet[string]]$Targets) {
    if ($null -ne $RuntimeCatalog.materialPrograms) {
        foreach ($binding in @($RuntimeCatalog.materialPrograms.bindings)) {
            if ($Targets.Contains([string]$binding.effectAssetId)) {
                throw "Selected direct-authored publish rejects material-program bound target: $($binding.effectAssetId)"
            }
        }
    }
    foreach ($program in @($VisualPrograms.programs)) {
        if ($Targets.Contains([string]$program.effectAssetId)) {
            throw "Selected direct-authored publish rejects visual-program bound target: $($program.effectAssetId)"
        }
    }
}

function Assert-SelectedDocumentBundle(
    [object]$Record) {
    $validationRoot = Join-Path ([IO.Path]::GetTempPath()) `
        ('LASE-' + [Guid]::NewGuid().ToString('N'))
    $catalogPath = Join-Path $validationRoot 'EffectCatalog.runtime.json'
    $authoredRoot = Join-Path $validationRoot 'Authored'
    try {
        [IO.Directory]::CreateDirectory($authoredRoot) | Out-Null
        $sealedPath = Join-Path $authoredRoot `
            ([IO.Path]::GetFileName([string]$Record.RelativePath))
        [IO.File]::WriteAllBytes($sealedPath, [byte[]]$Record.PayloadBytes)
        $catalog = [ordered]@{
            schema = 'lostark.effect-runtime-catalog'
            formatVersion = 3
            components = @()
            effects = @([ordered]@{
                payloadKind = 'DIRECT_AUTHORED_DOCUMENT_V13'
                effectAssetId = [string]$Record.EffectAssetId
                authoringFormatVersion = 13
                authoredDocumentPath = [string]$Record.RelativePath
            })
        }
        [IO.File]::WriteAllText(
            $catalogPath,
            ($catalog | ConvertTo-Json -Depth 20 -Compress) + "`n",
            $utf8NoBom)
        Invoke-RuntimeValidator $catalogPath
    }
    finally {
        if (Test-Path -LiteralPath $validationRoot) {
            Remove-Item -LiteralPath $validationRoot -Recurse -Force
        }
    }
}

foreach ($pathCheck in @(
        [pscustomobject]@{ Path = $DataRoot; Label = 'Data root' },
        [pscustomobject]@{ Path = $ResourceRoot; Label = 'Resource root' },
        [pscustomobject]@{ Path = $RuntimeRoot; Label = 'Effect runtime root' },
        [pscustomobject]@{ Path = $sourceAuthoredRoot; Label = 'Authored Effect root' },
        [pscustomobject]@{ Path = $runtimeAuthoredRoot; Label = 'Runtime Authored root' })) {
    Assert-RegularDirectory $pathCheck.Path $pathCheck.Label
}
foreach ($pathCheck in @(
        [pscustomobject]@{ Path = $sourceCatalogPath; Label = 'Source Effect catalog' },
        [pscustomobject]@{ Path = $runtimeCatalogPath; Label = 'Runtime Effect catalog' },
        [pscustomobject]@{ Path = $visualProgramPath; Label = 'Effect visual-program sidecar' },
        [pscustomobject]@{ Path = $compactTool; Label = 'Effect document compactor' },
        [pscustomobject]@{ Path = $runtimeValidator; Label = 'Effect runtime validator' })) {
    Assert-RegularFile $pathCheck.Path $pathCheck.Label
}
if ((Test-PathIsSameOrDescendant $RuntimeRoot $DataRoot) -or
    (Test-PathIsSameOrDescendant $RuntimeRoot $ResourceRoot)) {
    throw 'Effect runtime root must be outside Data and Resources source roots.'
}
if ($TestFaultInjection -cne 'None') {
    if ($Mode -cne 'Publish') {
        throw 'Fault injection is valid only in Publish mode.'
    }
    $temporaryRoot = [IO.Path]::GetFullPath([IO.Path]::GetTempPath())
    if (-not (Test-PathIsSameOrDescendant $RuntimeRoot $temporaryRoot)) {
        throw 'Fault injection is restricted to a temporary runtime root.'
    }
}

$targets = [Collections.Generic.HashSet[string]]::new(
    [StringComparer]::Ordinal)
foreach ($targetId in @($EffectAssetId)) {
    Assert-StableId $targetId 'Selected EffectAssetId'
    if (-not $allowedTargetIds.Contains($targetId)) {
        throw "Selected publisher target is not in the audited direct-authored allowlist: $targetId"
    }
    if (-not $targets.Add($targetId)) {
        throw "Duplicate selected EffectAssetId: $targetId"
    }
}
if (0 -eq $targets.Count) {
    throw 'At least one selected EffectAssetId is required.'
}

$transactionLockPath = Join-Path $RuntimeRoot `
    '.Publish-Effects.transaction.lock'
$transactionLock = $null
try {
    try {
        $transactionLock = [IO.FileStream]::new(
            $transactionLockPath,
            [IO.FileMode]::OpenOrCreate,
            [IO.FileAccess]::ReadWrite,
            [IO.FileShare]::None,
            1,
            [IO.FileOptions]::DeleteOnClose)
    }
    catch {
        throw "Effect publish transaction lock is held or unavailable: $($_.Exception.Message)"
    }

    $sourceCatalogLoaded = Read-Utf8Json `
        $sourceCatalogPath 'Source Effect catalog'
    $runtimeCatalogLoaded = Read-Utf8Json `
        $runtimeCatalogPath 'Runtime Effect catalog' $true
    $visualProgramsLoaded = Read-Utf8Json `
        $visualProgramPath 'Effect visual-program sidecar'
    $sourceCatalog = $sourceCatalogLoaded.Json
    $runtimeCatalog = $runtimeCatalogLoaded.Json
    $visualPrograms = $visualProgramsLoaded.Json
    if ([int]$sourceCatalog.formatVersion -ne 1 -or
        [string]$runtimeCatalog.schema -cne
            'lostark.effect-runtime-catalog' -or
        [int]$runtimeCatalog.formatVersion -notin @(3, 4) -or
        $null -eq $runtimeCatalog.effects -or
        $null -eq $runtimeCatalog.components -or
        $null -eq $visualPrograms.programs) {
        throw 'Selected Effect publisher input catalog identity is invalid.'
    }
    Assert-NoTargetBindings $runtimeCatalog $visualPrograms $targets
    Invoke-RuntimeValidator $runtimeCatalogPath
    Assert-RuntimeDirectSeals $runtimeCatalog

    $records = [Collections.Generic.List[object]]::new()
    foreach ($targetId in @($targets | Sort-Object)) {
        $sourceMatches = @($sourceCatalog.effects | Where-Object {
            [string]$_.effectAssetId -ceq $targetId
        })
        $runtimeMatches = @($runtimeCatalog.effects | Where-Object {
            [string]$_.effectAssetId -ceq $targetId
        })
        if ($sourceMatches.Count -ne 1 -or $runtimeMatches.Count -gt 1) {
            throw "Selected Effect must have exactly one source and at most one runtime entry: $targetId"
        }
        $sourceEntry = $sourceMatches[0]
        $runtimeEntry = if ($runtimeMatches.Count -eq 1) {
            $runtimeMatches[0]
        }
        else {
            $null
        }
        Assert-ExactPropertyOrder $sourceEntry @(
            'effectAssetId', 'payloadKind', 'authoringPath') `
            "Selected source catalog entry '$targetId'"
        if ($null -ne $runtimeEntry) {
            Assert-ExactPropertyOrder $runtimeEntry @(
                'payloadKind', 'effectAssetId', 'authoringFormatVersion',
                'authoredDocumentPath') `
                "Selected runtime catalog entry '$targetId'"
        }
        $expectedAuthoringPath =
            "Effects/Authored/$targetId.effect.json"
        if ([string]$sourceEntry.payloadKind -cne
                'DIRECT_AUTHORED_DOCUMENT_V13' -or
            [string]$sourceEntry.authoringPath -cne $expectedAuthoringPath -or
            ($null -ne $runtimeEntry -and
             ([string]$runtimeEntry.payloadKind -cne
                'DIRECT_AUTHORED_DOCUMENT_V13' -or
              [int]$runtimeEntry.authoringFormatVersion -ne 13))) {
            throw "Selected Effect is not an unbound direct-authored v13 Product target: $targetId"
        }
        $authoringPath = Resolve-SafeRelativeFile `
            $DataRoot $expectedAuthoringPath "Selected authored Effect '$targetId'"
        $loadedDocument = Assert-AuthoredDocument $targetId $authoringPath
        $compactPath = Join-Path ([IO.Path]::GetTempPath()) `
            ('LostArkSelectedEffectCompact-' +
             [Guid]::NewGuid().ToString('N') + '.json')
        try {
            Invoke-Compactor $authoringPath $compactPath
            $payloadBytes = [IO.File]::ReadAllBytes($compactPath)
        }
        finally {
            if (Test-Path -LiteralPath $compactPath) {
                Remove-Item -LiteralPath $compactPath -Force
            }
        }
        $payloadSha = Get-Sha256Hex $payloadBytes
        $relativePath =
            "Authored/$targetId.$payloadSha.effect.json"
        $record = [pscustomobject]@{
            EffectAssetId = $targetId
            SourcePath = $authoringPath
            SourceSha256 = Get-Sha256Hex $loadedDocument.Bytes
            SourceByteCount = $loadedDocument.Bytes.LongLength
            PayloadBytes = $payloadBytes
            PayloadSha256 = $payloadSha
            RelativePath = $relativePath
            IsInsertion = $null -eq $runtimeEntry
            PreviousRelativePath = if ($null -eq $runtimeEntry) {
                ''
            }
            else {
                [string]$runtimeEntry.authoredDocumentPath
            }
            PreviousFragment = if ($null -eq $runtimeEntry) {
                ''
            }
            else {
                Get-JsonFingerprint $runtimeEntry
            }
            NewFragment = Get-JsonFingerprint ([ordered]@{
                payloadKind = 'DIRECT_AUTHORED_DOCUMENT_V13'
                effectAssetId = $targetId
                authoringFormatVersion = 13
                authoredDocumentPath = $relativePath
            })
        }
        Assert-SelectedDocumentBundle $record
        $records.Add($record)
    }

    $stagedCatalogText = [string]$runtimeCatalogLoaded.Text
    foreach ($record in @($records | Where-Object { -not $_.IsInsertion })) {
        $first = $stagedCatalogText.IndexOf(
            $record.PreviousFragment, [StringComparison]::Ordinal)
        $last = $stagedCatalogText.LastIndexOf(
            $record.PreviousFragment, [StringComparison]::Ordinal)
        if ($first -lt 0 -or $first -ne $last) {
            throw "Selected runtime entry was not one exact lexical fragment: $($record.EffectAssetId)"
        }
        $stagedCatalogText =
            $stagedCatalogText.Substring(0, $first) +
            $record.NewFragment +
            $stagedCatalogText.Substring(
                $first + $record.PreviousFragment.Length)
    }
    $baselineEffects = @($runtimeCatalog.effects)
    $insertionRecords = @($records | Where-Object { $_.IsInsertion } |
        Sort-Object EffectAssetId)
    if ($insertionRecords.Count -gt 0) {
        if ($baselineEffects.Count -lt 1) {
            throw 'Selected runtime insertion requires one existing lexical anchor.'
        }
        # The existing publisher does not promise lexical Effect-ID ordering.
        # Append new selected rows in stable target-ID order so every existing
        # entry remains at the same byte position relative to every other
        # non-target entry.
        $anchor = Get-JsonFingerprint $baselineEffects[-1]
        $anchorIndex = $stagedCatalogText.IndexOf(
            $anchor, [StringComparison]::Ordinal)
        if ($anchorIndex -lt 0 -or $anchorIndex -ne
                $stagedCatalogText.LastIndexOf(
                    $anchor, [StringComparison]::Ordinal)) {
            throw 'Could not locate the unique final runtime Effect insertion anchor.'
        }
        $anchorEnd = $anchorIndex + $anchor.Length
        $insertionText = ($insertionRecords | ForEach-Object {
            $_.NewFragment
        }) -join ','
        $stagedCatalogText =
            $stagedCatalogText.Substring(0, $anchorEnd) + ',' +
            $insertionText +
            $stagedCatalogText.Substring($anchorEnd)
    }

    $stagedCatalog = $stagedCatalogText | ConvertFrom-Json
    $originalEffects = @($runtimeCatalog.effects)
    $stagedEffects = @($stagedCatalog.effects)
    $insertionCount = $insertionRecords.Count
    if ($originalEffects.Count + $insertionCount -ne $stagedEffects.Count -or
        (Get-JsonFingerprint $runtimeCatalog.components) -cne
            (Get-JsonFingerprint $stagedCatalog.components) -or
        (Get-JsonFingerprint $runtimeCatalog.materialPrograms) -cne
            (Get-JsonFingerprint $stagedCatalog.materialPrograms)) {
        throw 'Selected runtime patch changed a non-target catalog object.'
    }
    foreach ($baselineEntry in $originalEffects) {
        $baselineId = [string]$baselineEntry.effectAssetId
        $stagedMatches = @($stagedEffects | Where-Object {
            [string]$_.effectAssetId -ceq $baselineId
        })
        if ($stagedMatches.Count -ne 1 -or
            (-not $targets.Contains($baselineId) -and
             (Get-JsonFingerprint $baselineEntry) -cne
                (Get-JsonFingerprint $stagedMatches[0]))) {
            throw "Selected runtime patch changed non-target Effect entry: $baselineId"
        }
    }
    for ($index = 0; $index -lt $insertionRecords.Count; ++$index) {
        $expectedIndex = $originalEffects.Count + $index
        if ([string]$stagedEffects[$expectedIndex].effectAssetId -cne
                [string]$insertionRecords[$index].EffectAssetId) {
            throw 'Selected runtime insertion order is not deterministic.'
        }
    }

    $changedRecords = @($records | Where-Object {
        $_.PreviousRelativePath -cne $_.RelativePath
    })
    if ($Mode -ceq 'Validate') {
        Write-Host (
            "PASS: validated selected direct-authored target(s): " +
            (($records | ForEach-Object {
                "$($_.EffectAssetId)=$($_.PayloadSha256)"
            }) -join ', '))
        return
    }
    if ($changedRecords.Count -eq 0) {
        Write-Host 'PASS: selected runtime targets already match their compacted authored seals.'
        return
    }

    $baselineCatalogSha = Get-Sha256Hex $runtimeCatalogLoaded.Bytes
    $transactionId = [Guid]::NewGuid().ToString('N')
    $catalogTemporary = Join-Path $RuntimeRoot `
        ".EffectCatalog.runtime.$transactionId.tmp"
    $catalogBackup = Join-Path $RuntimeRoot `
        ".EffectCatalog.runtime.$transactionId.bak"
    Assert-AbsentOrRegularFile $catalogTemporary 'Staged Effect runtime catalog'
    Assert-AbsentOrRegularFile $catalogBackup 'Effect runtime catalog rollback'
    $installedSeals = [Collections.Generic.List[string]]::new()
    $catalogBackedUp = $false
    $catalogCommitted = $false
    try {
        foreach ($record in $records) {
            $sourceBytesNow = [IO.File]::ReadAllBytes($record.SourcePath)
            if ($sourceBytesNow.LongLength -ne $record.SourceByteCount -or
                (Get-Sha256Hex $sourceBytesNow) -cne $record.SourceSha256) {
                throw "Selected authored source changed during publish: $($record.EffectAssetId)"
            }
            $destination = [IO.Path]::GetFullPath((Join-Path `
                $RuntimeRoot $record.RelativePath.Replace('/', '\')))
            if (-not (Test-PathIsSameOrDescendant `
                    $destination $runtimeAuthoredRoot) -or
                -not ([IO.Path]::GetDirectoryName($destination)).Equals(
                    $runtimeAuthoredRoot,
                    [StringComparison]::OrdinalIgnoreCase)) {
                throw "Selected authored seal escaped Runtime/Authored: $($record.EffectAssetId)"
            }
            Assert-AbsentOrRegularFile $destination `
                'Selected immutable authored seal'
            if ([IO.File]::Exists($destination)) {
                $existing = [IO.File]::ReadAllBytes($destination)
                if ((Get-Sha256Hex $existing) -cne $record.PayloadSha256 -or
                    -not (Test-ByteArrayEqual $existing $record.PayloadBytes)) {
                    throw "Conflicting selected immutable authored seal: $destination"
                }
                continue
            }
            $sealTemporary = Join-Path $runtimeAuthoredRoot `
                ".$transactionId.$($installedSeals.Count).tmp"
            Assert-AbsentOrRegularFile $sealTemporary `
                'Selected authored seal temporary'
            try {
                [IO.File]::WriteAllBytes(
                    $sealTemporary, [byte[]]$record.PayloadBytes)
                $stagedBytes = [IO.File]::ReadAllBytes($sealTemporary)
                if ((Get-Sha256Hex $stagedBytes) -cne
                        $record.PayloadSha256 -or
                    -not (Test-ByteArrayEqual `
                        $stagedBytes $record.PayloadBytes)) {
                    throw "Selected authored seal staging identity failed: $($record.EffectAssetId)"
                }
                [IO.File]::Move($sealTemporary, $destination)
                $installedSeals.Add($destination)
            }
            finally {
                if ([IO.File]::Exists($sealTemporary)) {
                    [IO.File]::Delete($sealTemporary)
                }
            }
        }
        if ($TestFaultInjection -ceq 'AfterSealInstall') {
            throw 'Injected selected Effect publish failure after seal install.'
        }

        [IO.File]::WriteAllText(
            $catalogTemporary, $stagedCatalogText, $utf8NoBom)
        Invoke-RuntimeValidator $catalogTemporary
        Assert-RuntimeDirectSeals `
            ((Read-Utf8Json $catalogTemporary `
                'Staged Effect runtime catalog' $true).Json)
        $catalogBytesNow = [IO.File]::ReadAllBytes($runtimeCatalogPath)
        if ((Get-Sha256Hex $catalogBytesNow) -cne $baselineCatalogSha -or
            -not (Test-ByteArrayEqual `
                $catalogBytesNow $runtimeCatalogLoaded.Bytes)) {
            throw 'Effect runtime catalog changed during selected publish.'
        }

        [IO.File]::Move($runtimeCatalogPath, $catalogBackup)
        $catalogBackedUp = $true
        if ($TestFaultInjection -ceq 'AfterCatalogBackup') {
            throw 'Injected selected Effect publish failure after catalog backup.'
        }
        [IO.File]::Move($catalogTemporary, $runtimeCatalogPath)
        $catalogCommitted = $true
        if ($TestFaultInjection -ceq 'AfterCatalogCommit') {
            throw 'Injected selected Effect publish failure after catalog commit.'
        }
        [IO.File]::Delete($catalogBackup)
        $catalogBackedUp = $false
    }
    catch {
        $publishFailure = $_
        $rollbackFailures = [Collections.Generic.List[string]]::new()
        if ($catalogCommitted -and [IO.File]::Exists($runtimeCatalogPath)) {
            try {
                [IO.File]::Delete($runtimeCatalogPath)
                $catalogCommitted = $false
            }
            catch {
                $rollbackFailures.Add(
                    "remove committed catalog: $($_.Exception.Message)")
            }
        }
        if ($catalogBackedUp -and [IO.File]::Exists($catalogBackup) -and
            -not [IO.File]::Exists($runtimeCatalogPath)) {
            try {
                [IO.File]::Move($catalogBackup, $runtimeCatalogPath)
                $catalogBackedUp = $false
            }
            catch {
                $rollbackFailures.Add(
                    "restore runtime catalog: $($_.Exception.Message)")
            }
        }
        if ($rollbackFailures.Count -eq 0) {
            foreach ($installedSeal in $installedSeals) {
                if (-not [IO.File]::Exists($installedSeal)) {
                    continue
                }
                try {
                    [IO.File]::Delete($installedSeal)
                }
                catch {
                    $rollbackFailures.Add(
                        "remove installed seal '$installedSeal': $($_.Exception.Message)")
                }
            }
        }
        if ($rollbackFailures.Count -ne 0) {
            throw (
                "Selected Effect publish failed and rollback was incomplete. " +
                "Original failure: $($publishFailure.Exception.Message). " +
                "Rollback failures: $($rollbackFailures -join '; ')")
        }
        throw $publishFailure
    }
    finally {
        foreach ($temporaryPath in @($catalogTemporary)) {
            if ([string]::IsNullOrWhiteSpace($temporaryPath) -or
                -not [IO.File]::Exists($temporaryPath)) {
                continue
            }
            [IO.File]::Delete($temporaryPath)
        }
    }

    Invoke-RuntimeValidator $runtimeCatalogPath
    Write-Host (
        "PASS: published selected direct-authored target(s): " +
        (($changedRecords | ForEach-Object {
            "$($_.EffectAssetId)=$($_.PayloadSha256)"
        }) -join ', '))
}
finally {
    if ($null -ne $transactionLock) {
        $transactionLock.Dispose()
    }
}
