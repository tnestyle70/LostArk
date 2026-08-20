$ErrorActionPreference = 'Stop'

$publisher = Join-Path $PSScriptRoot 'Publish-Effects.ps1'
$repositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$sourceDataRoot = Join-Path $repositoryRoot 'Data'
$sourceResourceRoot = Join-Path $repositoryRoot 'Client\Bin\Resources'
$fixture = Join-Path ([IO.Path]::GetTempPath()) `
    ('LostArkProductCuePublisher-' + [Guid]::NewGuid().ToString('N'))
$fixture = [IO.Path]::GetFullPath($fixture)
$dataRoot = Join-Path $fixture 'Data'
$resourceRoot = Join-Path $fixture 'Resources'
$output = Join-Path $fixture 'Runtime\EffectCatalog.runtime.json'
$visualOutput = Join-Path $fixture 'Runtime\EffectVisualPrograms.runtime.json'
$admissionOutput = Join-Path $fixture `
    'Runtime\EffectProductCueAdmissions.runtime.json'
$visualSource = Join-Path $dataRoot `
    'Effects\VisualPrograms\effect-visual-program-runtime.v1.json'
$policyPath = Join-Path $dataRoot 'Effects\ProductCueApprovals.json'
$catalogPath = Join-Path $dataRoot 'Effects\EffectCatalog.json'
$utf8NoBom = [Text.UTF8Encoding]::new($false)

function Write-Utf8Json([string]$Path, [object]$Value) {
    [IO.Directory]::CreateDirectory((Split-Path -Parent $Path)) | Out-Null
    [IO.File]::WriteAllText(
        $Path, (($Value | ConvertTo-Json -Depth 100) + "`n"), $utf8NoBom)
}

function Copy-DataFile([string]$RelativePath) {
    $source = Join-Path $sourceDataRoot $RelativePath
    $destination = Join-Path $dataRoot $RelativePath
    if (-not (Test-Path -LiteralPath $source -PathType Leaf)) {
        throw "Missing publisher fixture source: $RelativePath"
    }
    [IO.Directory]::CreateDirectory((Split-Path -Parent $destination)) |
        Out-Null
    [IO.File]::WriteAllBytes($destination, [IO.File]::ReadAllBytes($source))
}

function Copy-JsonValue([object]$Value) {
    return ($Value | ConvertTo-Json -Depth 100 -Compress) | ConvertFrom-Json
}

function Get-RawSha256([string]$Path) {
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant()
}

function Add-DeclaredResourceIds(
    [object]$Value,
    [Collections.Generic.HashSet[string]]$Result) {
    if ($null -eq $Value -or $Value -is [string]) { return }
    if ($Value -is [Collections.IEnumerable] -and
        $Value -isnot [Management.Automation.PSCustomObject]) {
        foreach ($item in $Value) {
            Add-DeclaredResourceIds $item $Result
        }
        return
    }
    if ($Value -isnot [Management.Automation.PSCustomObject]) { return }
    foreach ($property in $Value.PSObject.Properties) {
        if ($property.Name -cin @('assetId', 'modelAssetId', 'compilerAssetId') -and
            $property.Value -is [string] -and
            ($property.Value.StartsWith('Effect/', [StringComparison]::Ordinal) -or
             $property.Value.StartsWith('Character/', [StringComparison]::Ordinal))) {
            [void]$Result.Add([string]$property.Value)
        }
        Add-DeclaredResourceIds $property.Value $Result
    }
}

function Link-DeclaredResource([string]$AssetId) {
    $source = Join-Path $sourceResourceRoot $AssetId
    $destination = Join-Path $resourceRoot $AssetId
    if (-not (Test-Path -LiteralPath $source -PathType Leaf)) {
        throw "Missing declared publisher fixture resource: $AssetId"
    }
    [IO.Directory]::CreateDirectory((Split-Path -Parent $destination)) |
        Out-Null
    try {
        New-Item -ItemType HardLink -Path $destination -Target $source `
            -ErrorAction Stop | Out-Null
    }
    catch {
        Copy-Item -LiteralPath $source -Destination $destination
    }
}

function Read-Bytes([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "Expected committed publisher output is missing: $Path"
    }
    return [IO.File]::ReadAllBytes($Path)
}

function Assert-ExactBytes(
    [string]$Name,
    [byte[]]$Expected,
    [string]$Path) {
    $actual = Read-Bytes $Path
    if ([Convert]::ToBase64String($Expected) -cne
        [Convert]::ToBase64String($actual)) {
        throw "$Name did not preserve exact committed bytes: $Path"
    }
}

function Invoke-Publish([string]$TargetOutput) {
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $TargetOutput
    if ($LASTEXITCODE) {
        throw "Effect publisher returned exit code $LASTEXITCODE."
    }
}

function Assert-PublishRejected(
    [string]$Name,
    [string]$TargetOutput,
    [string]$ExpectedMessage = '') {
    $failed = $false
    $failureMessage = ''
    try {
        Invoke-Publish $TargetOutput
    }
    catch {
        $failed = $true
        $failureMessage = $_.Exception.Message
    }
    if (-not $failed) {
        throw "$Name was accepted by Publish-Effects.ps1."
    }
    if (-not [string]::IsNullOrWhiteSpace($ExpectedMessage) -and
        $failureMessage.IndexOf(
            $ExpectedMessage, [StringComparison]::Ordinal) -lt 0) {
        throw "$Name failed for the wrong reason: $failureMessage"
    }
}

function Assert-PathAbsent([string]$Name, [string]$Path) {
    if (Test-Path -LiteralPath $Path) {
        throw "$Name created or retained an unexpected path: $Path"
    }
}

function Assert-PublishRejectedAndPreserved(
    [string]$Name,
    [byte[]]$CatalogBytes,
    [byte[]]$VisualBytes,
    [byte[]]$AdmissionBytes) {
    Assert-PublishRejected $Name $output
    Assert-ExactBytes $Name $CatalogBytes $output
    Assert-ExactBytes $Name $VisualBytes $visualOutput
    Assert-ExactBytes $Name $AdmissionBytes $admissionOutput
}

function Restore-Bytes([string]$Path, [byte[]]$Payload) {
    [IO.File]::WriteAllBytes($Path, $Payload)
}

try {
    [IO.Directory]::CreateDirectory($dataRoot) | Out-Null
    [IO.Directory]::CreateDirectory($resourceRoot) | Out-Null
    [IO.Directory]::CreateDirectory(
        (Join-Path $dataRoot 'Effects\Assemblies')) | Out-Null
    [IO.Directory]::CreateDirectory(
        (Join-Path $dataRoot 'Effects\Components')) | Out-Null

    Copy-DataFile 'Effects\VisualPrograms\effect-visual-program-runtime.v1.json'
    Copy-DataFile 'Balance\PlayerSkills.json'
    Copy-DataFile 'Effects\ProductCueApprovals.json'

    $policy = Get-Content -LiteralPath $policyPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    if (@($policy.approvals).Count -ne 5) {
        throw 'Current Product cue policy no longer contains exactly five canary approvals.'
    }
    $sourceCatalog = Get-Content -LiteralPath `
        (Join-Path $sourceDataRoot 'Effects\EffectCatalog.json') `
        -Raw -Encoding UTF8 | ConvertFrom-Json
    $sourceEntries = @{}
    foreach ($entry in @($sourceCatalog.effects)) {
        $sourceEntries[[string]$entry.effectAssetId] = $entry
    }

    $fixtureCatalogEntries = [Collections.Generic.List[object]]::new()
    $directDocuments = [Collections.Generic.List[string]]::new()
    $catalogIds = [Collections.Generic.HashSet[string]]::new(
        [StringComparer]::Ordinal)
    foreach ($approval in @($policy.approvals)) {
        $effectId = [string]$approval.effectAssetId
        $rollbackId = [string]$approval.rollbackEffectAssetId
        if (-not $sourceEntries.ContainsKey($effectId)) {
            throw "Current canary Effect source is missing: $effectId"
        }
        $candidateEntry = Copy-JsonValue $sourceEntries[$effectId]
        if ([string]$candidateEntry.payloadKind -cne
            'DIRECT_AUTHORED_DOCUMENT_V13') {
            throw "Current canary is not direct authored v13: $effectId"
        }
        if ($catalogIds.Add($effectId)) {
            $fixtureCatalogEntries.Add($candidateEntry)
            Copy-DataFile ([string]$candidateEntry.authoringPath).Replace('/', '\')
            $directDocuments.Add((Join-Path $dataRoot `
                ([string]$candidateEntry.authoringPath)))
        }

        if ($rollbackId -ceq 'effect.artist.skill.31470') {
            $rollbackEntry = Copy-JsonValue $sourceEntries[$rollbackId]
            if ($catalogIds.Add($rollbackId)) {
                $fixtureCatalogEntries.Add($rollbackEntry)
                Copy-DataFile ([string]$rollbackEntry.reconstructedRuntimeProgramPath).Replace('/', '\')
                Copy-DataFile ([string]$rollbackEntry.reconstructedRenderResourceAuthorityPath).Replace('/', '\')
                if ($null -ne $rollbackEntry.PSObject.Properties['occurrenceTuningPath']) {
                    Copy-DataFile ([string]$rollbackEntry.occurrenceTuningPath).Replace('/', '\')
                }
            }
        }
        elseif ($catalogIds.Add($rollbackId)) {
            $candidatePath = Join-Path $dataRoot ([string]$candidateEntry.authoringPath)
            $candidateDocument = Get-Content -LiteralPath $candidatePath `
                -Raw -Encoding UTF8 | ConvertFrom-Json
            $rollbackDocument = [ordered]@{
                schema = 'lostark.effect-authoring'
                version = 13
                effectAssetId = $rollbackId
                displayName = 'Product cue rollback fixture'
                particleSystem = Copy-JsonValue $candidateDocument.particleSystem
                modelCues = @()
                elements = @()
            }
            $rollbackPathRelative = "Effects/Authored/$rollbackId.effect.json"
            $rollbackPath = Join-Path $dataRoot $rollbackPathRelative
            Write-Utf8Json $rollbackPath $rollbackDocument
            $fixtureCatalogEntries.Add([ordered]@{
                effectAssetId = $rollbackId
                payloadKind = 'DIRECT_AUTHORED_DOCUMENT_V13'
                authoringPath = $rollbackPathRelative
            })
            $directDocuments.Add($rollbackPath)
        }

        $animationAssetId = [string]$approval.animationAssetId
        foreach ($suffix in @('skillbindings.json', 'animevents')) {
            Copy-DataFile ("Animation\Authored\$animationAssetId\" +
                "$animationAssetId.$suffix")
        }
    }

    $artistApproval = @($policy.approvals | Where-Object {
        $_.effectAssetId -ceq 'effect.artist.skill.31470.unified'
    })
    if ($artistApproval.Count -ne 1) {
        throw 'Artist 31470 Product canary approval is missing or duplicated.'
    }
    $artistPath = Join-Path $dataRoot `
        'Effects\Authored\effect.artist.skill.31470.unified.effect.json'
    $artistDocument = Get-Content -LiteralPath $artistPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $fullArtist = Read-Bytes $artistPath
    $approximateCarrier = @($artistDocument.elements | Where-Object {
        [bool]$_.visible -and
        $null -ne $_.material.sourceProfile -and
        [bool]$_.material.sourceProfile.enabled -and
        $null -eq $_.material.PSObject.Properties['execution']
    })
    if ($approximateCarrier.Count -lt 1) {
        throw 'Artist canary has no safe visible Source Profile carrier for the Approximate fixture.'
    }
    $approximateCarrier[0].material | Add-Member -NotePropertyName execution `
        -NotePropertyValue ([ordered]@{
            enabled = $false
            failClosed = $true
            authoringApproximate = $true
        })
    Write-Utf8Json $artistPath $artistDocument
    $artistApproval[0].effectContentSha256 = Get-RawSha256 $artistPath
    Write-Utf8Json $policyPath $policy
    Write-Utf8Json $catalogPath ([ordered]@{
        formatVersion = 1
        effects = @($fixtureCatalogEntries)
    })

    $resourceIds = [Collections.Generic.HashSet[string]]::new(
        [StringComparer]::Ordinal)
    foreach ($documentPath in $directDocuments) {
        $document = Get-Content -LiteralPath $documentPath -Raw -Encoding UTF8 |
            ConvertFrom-Json
        Add-DeclaredResourceIds $document $resourceIds
    }
    foreach ($assetId in @($resourceIds | Sort-Object)) {
        Link-DeclaredResource $assetId
    }

    $sourceCatalogBytes = Read-Bytes $catalogPath
    Assert-PublishRejected 'Explicit source alias output' $catalogPath `
        'aliases an Effect publisher source input'
    Assert-ExactBytes 'Explicit source alias output' `
        $sourceCatalogBytes $catalogPath

    $dataRootOutput = Join-Path $dataRoot `
        'Effects\UnsafeEffectCatalog.runtime.json'
    Assert-PublishRejected 'DataRoot descendant output' $dataRootOutput `
        'must be outside publisher source root'
    Assert-PathAbsent 'DataRoot descendant output' $dataRootOutput

    $resourceRootOutput = Join-Path $resourceRoot `
        'Effect\UnsafeEffectCatalog.runtime.json'
    Assert-PublishRejected 'ResourceRoot descendant output' `
        $resourceRootOutput 'must be outside publisher source root'
    Assert-PathAbsent 'ResourceRoot descendant output' $resourceRootOutput

    foreach ($destinationCase in @(
            [pscustomobject]@{
                Name = 'Catalog directory destination'
                BadDestinationIndex = 0
            },
            [pscustomobject]@{
                Name = 'Visual directory destination'
                BadDestinationIndex = 1
            },
            [pscustomobject]@{
                Name = 'Admission directory destination'
                BadDestinationIndex = 2
            })) {
        $caseRoot = Join-Path $fixture `
            ('DestinationPreflight-' + $destinationCase.BadDestinationIndex)
        [IO.Directory]::CreateDirectory($caseRoot) | Out-Null
        $caseOutput = Join-Path $caseRoot 'EffectCatalog.runtime.json'
        $caseDestinations = @(
            $caseOutput,
            (Join-Path $caseRoot 'EffectVisualPrograms.runtime.json'),
            (Join-Path $caseRoot `
                'EffectProductCueAdmissions.runtime.json'))
        $badDestination = $caseDestinations[
            [int]$destinationCase.BadDestinationIndex]
        [IO.Directory]::CreateDirectory($badDestination) | Out-Null
        Assert-PublishRejected $destinationCase.Name $caseOutput `
            'regular non-reparse file'
        $badItem = Get-Item -LiteralPath $badDestination -Force
        if (-not [bool]$badItem.PSIsContainer -or
            @(Get-ChildItem -LiteralPath $badDestination -Force).Count -ne 0) {
            throw "$($destinationCase.Name) was mutated before rejection."
        }
        Remove-Item -LiteralPath $caseRoot -Recurse -Force
    }

    $junctionCaseRoot = Join-Path $fixture 'DestinationPreflight-Reparse'
    $junctionTarget = Join-Path $junctionCaseRoot 'JunctionTarget'
    [IO.Directory]::CreateDirectory($junctionTarget) | Out-Null
    $junctionOutput = Join-Path $junctionCaseRoot `
        'Runtime\EffectCatalog.runtime.json'
    [IO.Directory]::CreateDirectory((Split-Path -Parent $junctionOutput)) |
        Out-Null
    $junctionVisual = Join-Path (Split-Path -Parent $junctionOutput) `
        'EffectVisualPrograms.runtime.json'
    New-Item -ItemType Junction -Path $junctionVisual -Target $junctionTarget |
        Out-Null
    if (((Get-Item -LiteralPath $junctionVisual -Force).Attributes -band
            [IO.FileAttributes]::ReparsePoint) -eq 0) {
        throw 'Reparse destination fixture is not a reparse point.'
    }
    Assert-PublishRejected 'Reparse destination' $junctionOutput `
        'regular non-reparse file'

    Invoke-Publish $output
    $runtime = Get-Content -LiteralPath $output -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $receipt = Get-Content -LiteralPath $admissionOutput -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $policySha256 = Get-RawSha256 $policyPath
    if ([string]$runtime.schema -cne 'lostark.effect-runtime-catalog' -or
        [int]$runtime.formatVersion -ne 3 -or
        -not [bool]$runtime.productCueAdmissionsRequired -or
        [string]$runtime.productCuePolicySha256 -cne $policySha256 -or
        [string]$receipt.schema -cne
            'lostark.effect-product-cue-admissions' -or
        [int]$receipt.formatVersion -ne 1 -or
        [string]$receipt.runtimeCatalogSha256 -cne
            (Get-RawSha256 $output) -or
        [string]$receipt.sourcePolicySha256 -cne $policySha256 -or
        [string]$receipt.sourcePolicyUtf8Json -cne
            [IO.File]::ReadAllText($policyPath, [Text.Encoding]::UTF8) -or
        @($receipt.approvals).Count -ne 5 -or
        -not (Test-Path -LiteralPath $visualOutput -PathType Leaf)) {
        throw 'Approved Product cue publish did not commit the catalog/visual/admission set.'
    }
    $artistReceipt = @($receipt.approvals | Where-Object {
        $_.effectAssetId -ceq 'effect.artist.skill.31470.unified'
    })
    if ($artistReceipt.Count -ne 1 -or
        [string]$artistReceipt[0].admission -cne
            'PRODUCT_APPROVED_APPROXIMATE' -or
        [int]$artistReceipt[0].approximateElementCount -ne 1) {
        throw 'Approved Approximate cue was not labeled truthfully in the admission sidecar.'
    }

    $committedCatalog = Read-Bytes $output
    $committedVisual = Read-Bytes $visualOutput
    $committedAdmission = Read-Bytes $admissionOutput
    $validPolicy = Read-Bytes $policyPath
    $validArtist = Read-Bytes $artistPath

    Remove-Item -LiteralPath $policyPath -Force
    Assert-PublishRejectedAndPreserved 'Missing source policy with committed admission receipt' `
        $committedCatalog $committedVisual $committedAdmission
    Restore-Bytes $policyPath $validPolicy

    try {
        Remove-Item -LiteralPath $policyPath -Force
        Remove-Item -LiteralPath $admissionOutput -Force
        Restore-Bytes $artistPath $fullArtist
        Assert-PublishRejected `
            'Missing policy and receipt with marker-bearing catalog' `
            $output 'Existing runtime catalog requires Product cue admissions'
        Assert-ExactBytes `
            'Missing policy and receipt with marker-bearing catalog' `
            $committedCatalog $output
        Assert-ExactBytes `
            'Missing policy and receipt with marker-bearing visual' `
            $committedVisual $visualOutput
        Assert-PathAbsent `
            'Missing policy and receipt with marker-bearing admission' `
            $admissionOutput
    }
    finally {
        Restore-Bytes $artistPath $validArtist
        Restore-Bytes $policyPath $validPolicy
        Restore-Bytes $admissionOutput $committedAdmission
    }

    $missingPolicy = Get-Content -LiteralPath $policyPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $missingPolicy.approvals = @($missingPolicy.approvals | Where-Object {
        $_.effectAssetId -cne 'effect.artist.skill.31470.unified'
    })
    Write-Utf8Json $policyPath $missingPolicy
    Assert-PublishRejectedAndPreserved 'Missing Approximate approval' `
        $committedCatalog $committedVisual $committedAdmission
    Restore-Bytes $policyPath $validPolicy

    $stalePolicy = Get-Content -LiteralPath $policyPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $stalePolicy.approvals[0].effectContentSha256 = ('0' * 64)
    Write-Utf8Json $policyPath $stalePolicy
    Assert-PublishRejectedAndPreserved 'Stale approved authored SHA' `
        $committedCatalog $committedVisual $committedAdmission
    Restore-Bytes $policyPath $validPolicy

    $wrongCuePolicy = Get-Content -LiteralPath $policyPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $wrongCuePolicy.approvals[0].startMs =
        [int]$wrongCuePolicy.approvals[0].startMs + 1
    Write-Utf8Json $policyPath $wrongCuePolicy
    Assert-PublishRejectedAndPreserved 'Wrong Product cue tuple' `
        $committedCatalog $committedVisual $committedAdmission
    Restore-Bytes $policyPath $validPolicy

    $hardDocument = Get-Content -LiteralPath $artistPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $hardCarrier = @($hardDocument.elements | Where-Object {
        $null -ne $_.material.execution -and
        [bool]$_.material.execution.failClosed -and
        $null -eq $_.material.execution.PSObject.Properties['authoringApproximate']
    })
    if ($hardCarrier.Count -lt 1) {
        throw 'Artist fixture has no suppressed Hard carrier for the rejection case.'
    }
    $hardCarrier[0].visible = $true
    Write-Utf8Json $artistPath $hardDocument
    $hardPolicy = Get-Content -LiteralPath $policyPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $hardPolicy.approvals[0].effectContentSha256 = Get-RawSha256 $artistPath
    Write-Utf8Json $policyPath $hardPolicy
    Assert-PublishRejectedAndPreserved 'Visible Hard carrier' `
        $committedCatalog $committedVisual $committedAdmission
    Restore-Bytes $artistPath $validArtist
    Restore-Bytes $policyPath $validPolicy

    $transactionLockPath = Join-Path (Split-Path -Parent $output) `
        '.Publish-Effects.transaction.lock'
    Assert-PathAbsent 'Pre-lock publisher residue' $transactionLockPath
    $heldTransactionLock = [IO.File]::Open(
        $transactionLockPath,
        [IO.FileMode]::OpenOrCreate,
        [IO.FileAccess]::ReadWrite,
        [IO.FileShare]::None)
    try {
        Assert-PublishRejected 'Concurrent publish transaction' $output `
            'transaction lock is held or unavailable'
        Assert-ExactBytes 'Concurrent publish transaction catalog' `
            $committedCatalog $output
        Assert-ExactBytes 'Concurrent publish transaction visual' `
            $committedVisual $visualOutput
        Assert-ExactBytes 'Concurrent publish transaction admission' `
            $committedAdmission $admissionOutput
    }
    finally {
        $heldTransactionLock.Dispose()
        Remove-Item -LiteralPath $transactionLockPath -Force
    }
    Assert-PathAbsent 'Released publisher transaction lock' `
        $transactionLockPath
    $transactionResidue = @(Get-ChildItem `
        -LiteralPath (Split-Path -Parent $output) -Force | Where-Object {
            $_.Name.EndsWith('.tmp', [StringComparison]::OrdinalIgnoreCase) -or
            $_.Name.EndsWith('.bak', [StringComparison]::OrdinalIgnoreCase)
        })
    if ($transactionResidue.Count -ne 0) {
        throw ('Concurrent publish transaction left staging residue: ' +
            (($transactionResidue | ForEach-Object Name) -join ', '))
    }

    $faultDocument = Get-Content -LiteralPath $artistPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $faultDocument.displayName = [string]$faultDocument.displayName + ' Fault'
    Write-Utf8Json $artistPath $faultDocument
    $faultPolicy = Get-Content -LiteralPath $policyPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $faultPolicy.decisionSetId = [string]$faultPolicy.decisionSetId + '.fault'
    $faultPolicy.approvals[0].effectContentSha256 = Get-RawSha256 $artistPath
    Write-Utf8Json $policyPath $faultPolicy
    [IO.File]::AppendAllText($visualSource, " `n", $utf8NoBom)

    $faultFailed = $false
    try {
        & $publisher -Mode Publish -DataRoot $dataRoot `
            -ResourceRoot $resourceRoot -OutputPath $output `
            -TestFaultInjection AfterAdmissionSidecarCommitMove
    }
    catch {
        $faultFailed = $true
    }
    if (-not $faultFailed) {
        throw 'AfterAdmissionSidecarCommitMove fault injection was accepted.'
    }
    Assert-ExactBytes 'AfterAdmissionSidecarCommitMove catalog rollback' `
        $committedCatalog $output
    Assert-ExactBytes 'AfterAdmissionSidecarCommitMove visual rollback' `
        $committedVisual $visualOutput
    Assert-ExactBytes 'AfterAdmissionSidecarCommitMove admission rollback' `
        $committedAdmission $admissionOutput

    Write-Host (
        'PASS: Product cue publisher admission, sticky policy marker, safe ' +
        'output paths, regular-destination preflight, exclusive transaction ' +
        'lock, three-file byte preservation, and final-commit rollback.')
}
finally {
    $temporaryPrefix = [IO.Path]::GetFullPath([IO.Path]::GetTempPath()).TrimEnd(
        [IO.Path]::DirectorySeparatorChar,
        [IO.Path]::AltDirectorySeparatorChar) +
        [IO.Path]::DirectorySeparatorChar
    if ($fixture.StartsWith(
            $temporaryPrefix, [StringComparison]::OrdinalIgnoreCase) -and
        [IO.Path]::GetFileName($fixture).StartsWith(
            'LostArkProductCuePublisher-', [StringComparison]::Ordinal) -and
        (Test-Path -LiteralPath $fixture)) {
        Remove-Item -LiteralPath $fixture -Recurse -Force
    }
}
