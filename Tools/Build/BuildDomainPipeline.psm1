Set-StrictMode -Version Latest

$script:DomainGraphSchema = 'lostark.build-domain-graph'
$script:DomainReceiptSchema = 'lostark.build-domain-receipt'
$script:ProductReceiptSchema = 'lostark.product-build-receipt'
$script:RunEvidenceSchema = 'lostark.build-run-evidence'
$script:DomainReceiptFormatVersion = 2
$script:ProductReceiptFormatVersion = 2

function Get-BuildSha256Text {
    param([Parameter(Mandatory = $true)][AllowEmptyString()][string]$Text)

    $algorithm = [Security.Cryptography.SHA256]::Create()
    try {
        $bytes = [Text.Encoding]::UTF8.GetBytes($Text)
        return ([BitConverter]::ToString(
            $algorithm.ComputeHash($bytes))).Replace('-', '').ToLowerInvariant()
    }
    finally {
        $algorithm.Dispose()
    }
}

function Get-BuildFileSha256 {
    param([Parameter(Mandatory = $true)][string]$Path)

    $algorithm = [Security.Cryptography.SHA256]::Create()
    $stream = [IO.File]::Open(
        $Path, [IO.FileMode]::Open, [IO.FileAccess]::Read, [IO.FileShare]::Read)
    try {
        return ([BitConverter]::ToString(
            $algorithm.ComputeHash($stream))).Replace('-', '').ToLowerInvariant()
    }
    finally {
        $stream.Dispose()
        $algorithm.Dispose()
    }
}

function ConvertTo-BuildRelativePath {
    param(
        [Parameter(Mandatory = $true)][string]$RepositoryRoot,
        [Parameter(Mandatory = $true)][string]$Path
    )

    $root = [IO.Path]::GetFullPath($RepositoryRoot).TrimEnd('\', '/')
    $full = [IO.Path]::GetFullPath($Path)
    $prefix = $root + [IO.Path]::DirectorySeparatorChar
    if (-not $full.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase)) {
        throw "Build-domain path escaped the repository: $full"
    }
    return $full.Substring($prefix.Length).Replace('\', '/')
}

function Assert-BuildRelativePattern {
    param([Parameter(Mandatory = $true)][string]$Pattern)

    $normalized = $Pattern.Replace('\', '/')
    if ([string]::IsNullOrWhiteSpace($normalized) -or
        [IO.Path]::IsPathRooted($normalized) -or
        $normalized.Contains(':') -or
        @($normalized.Split('/') | Where-Object { $_ -eq '..' }).Count -ne 0) {
        throw "Build-domain pattern must be repository-relative: $Pattern"
    }
    return $normalized.TrimStart('./')
}

function ConvertTo-BuildGlobRegex {
    param([Parameter(Mandatory = $true)][string]$Pattern)

    $pattern = Assert-BuildRelativePattern $Pattern
    $builder = [Text.StringBuilder]::new('^')
    for ($index = 0; $index -lt $pattern.Length; ++$index) {
        $character = $pattern[$index]
        if ($character -eq '*') {
            $isDouble = $index + 1 -lt $pattern.Length -and
                $pattern[$index + 1] -eq '*'
            if ($isDouble) {
                $index++
                if ($index + 1 -lt $pattern.Length -and
                    $pattern[$index + 1] -eq '/') {
                    $index++
                    [void]$builder.Append('(?:.*/)?')
                }
                else {
                    [void]$builder.Append('.*')
                }
            }
            else {
                [void]$builder.Append('[^/]*')
            }
            continue
        }
        if ($character -eq '?') {
            [void]$builder.Append('[^/]')
            continue
        }
        [void]$builder.Append([Text.RegularExpressions.Regex]::Escape(
            [string]$character))
    }
    [void]$builder.Append('$')
    return $builder.ToString()
}

function Get-BuildPatternSearchRoot {
    param(
        [Parameter(Mandatory = $true)][string]$RepositoryRoot,
        [Parameter(Mandatory = $true)][string]$Pattern
    )

    $pattern = Assert-BuildRelativePattern $Pattern
    $wildcard = $pattern.IndexOfAny([char[]]'*?')
    if ($wildcard -lt 0) {
        return [IO.Path]::GetDirectoryName(
            [IO.Path]::GetFullPath((Join-Path $RepositoryRoot $pattern)))
    }
    $prefix = $pattern.Substring(0, $wildcard)
    $slash = $prefix.LastIndexOf('/')
    $relativeRoot = if ($slash -lt 0) { '' } else { $prefix.Substring(0, $slash) }
    return [IO.Path]::GetFullPath((Join-Path $RepositoryRoot $relativeRoot))
}

function Expand-BuildToken {
    param(
        [Parameter(Mandatory = $true)][string]$Value,
        [Parameter(Mandatory = $true)][string]$RepositoryRoot,
        [Parameter(Mandatory = $true)][string]$Configuration,
        [Parameter(Mandatory = $true)][string]$ResourceRoot
    )

    $assimpRuntimeName = if ($Configuration -eq 'Debug') {
        'assimp-vc143-mtd.dll'
    }
    elseif ($Configuration -eq 'Release') {
        'assimp-vc143-mt.dll'
    }
    else {
        ''
    }
    return $Value.Replace('{repositoryRoot}', [IO.Path]::GetFullPath($RepositoryRoot)).
        Replace('{configuration}', $Configuration).
        Replace('{resourceRoot}', [IO.Path]::GetFullPath($ResourceRoot)).
        Replace('{assimpRuntimeName}', $assimpRuntimeName)
}

function Resolve-BuildPatternFiles {
    param(
        [Parameter(Mandatory = $true)][string]$RepositoryRoot,
        [string[]]$Patterns = @(),
        [string]$Configuration = 'Shared',
        [string]$ResourceRoot = ''
    )

    $root = [IO.Path]::GetFullPath($RepositoryRoot)
    if ([string]::IsNullOrWhiteSpace($ResourceRoot)) { $ResourceRoot = $root }
    $paths = [Collections.Generic.Dictionary[string,string]]::new(
        [StringComparer]::OrdinalIgnoreCase)
    foreach ($rawPattern in $Patterns) {
        $expanded = Expand-BuildToken ([string]$rawPattern) $root `
            $Configuration $ResourceRoot
        $pattern = Assert-BuildRelativePattern $expanded
        $hasWildcard = $pattern.IndexOfAny([char[]]'*?') -ge 0
        if (-not $hasWildcard) {
            $full = [IO.Path]::GetFullPath((Join-Path $root $pattern))
            if ([IO.File]::Exists($full)) {
                $relative = ConvertTo-BuildRelativePath $root $full
                $paths[$relative] = $full
            }
            continue
        }
        $searchRoot = Get-BuildPatternSearchRoot $root $pattern
        if (-not [IO.Directory]::Exists($searchRoot)) { continue }
        $regex = [Text.RegularExpressions.Regex]::new(
            (ConvertTo-BuildGlobRegex $pattern),
            [Text.RegularExpressions.RegexOptions]::IgnoreCase)
        foreach ($file in @(Get-ChildItem -LiteralPath $searchRoot -File -Recurse)) {
            $relative = ConvertTo-BuildRelativePath $root $file.FullName
            if ($regex.IsMatch($relative)) { $paths[$relative] = $file.FullName }
        }
    }
    [string[]]$keys = @($paths.Keys)
    [Array]::Sort($keys, [StringComparer]::Ordinal)
    return @($keys | ForEach-Object {
        [pscustomobject][ordered]@{ RelativePath = $_; FullPath = $paths[$_] }
    })
}

function Get-BuildFileRecords {
    param([object[]]$Files = @())

    return @($Files | ForEach-Object {
        $item = Get-Item -LiteralPath $_.FullPath
        [pscustomobject][ordered]@{
            path = [string]$_.RelativePath
            size = [int64]$item.Length
            sha256 = Get-BuildFileSha256 $item.FullName
            lastWriteTimeUtc = $item.LastWriteTimeUtc.ToString('o')
        }
    })
}

function Get-BuildContentIdentityRecords {
    param([object[]]$Records = @())

    return @($Records | ForEach-Object {
        [pscustomobject][ordered]@{
            path = [string]$_.path
            size = [int64]$_.size
            sha256 = [string]$_.sha256
        }
    })
}

function Get-BuildPropertyPathValue {
    param(
        [Parameter(Mandatory = $true)]$Value,
        [Parameter(Mandatory = $true)][string]$PropertyPath
    )

    $current = $Value
    foreach ($part in $PropertyPath.Split('.')) {
        if ($null -eq $current -or
            $current.PSObject.Properties.Name -notcontains $part) {
            return $null
        }
        $current = $current.$part
    }
    return $current
}

function Get-BuildResourceRecords {
    param(
        [Parameter(Mandatory = $true)][string]$RepositoryRoot,
        [Parameter(Mandatory = $true)]$Domain,
        [Parameter(Mandatory = $true)][string]$ResourceRoot
    )

    if ($Domain.PSObject.Properties.Name -notcontains 'resourceReferenceDocuments') {
        return @()
    }
    $documents = @(Resolve-BuildPatternFiles $RepositoryRoot `
        @($Domain.resourceReferenceDocuments))
    $assets = [Collections.Generic.HashSet[string]]::new(
        [StringComparer]::Ordinal)
    foreach ($documentPath in $documents) {
        $document = Get-Content -LiteralPath $documentPath.FullPath -Raw -Encoding UTF8 |
            ConvertFrom-Json
        foreach ($field in @($Domain.resourceReferenceFields)) {
            $assetId = Get-BuildPropertyPathValue $document ([string]$field)
            if ($null -eq $assetId -or [string]::IsNullOrWhiteSpace([string]$assetId)) {
                continue
            }
            $assetId = ([string]$assetId).Replace('\', '/')
            [void](Assert-BuildRelativePattern $assetId)
            [void]$assets.Add($assetId)
        }
    }
    [string[]]$assetIds = @($assets)
    [Array]::Sort($assetIds, [StringComparer]::Ordinal)
    $root = [IO.Path]::GetFullPath($ResourceRoot).TrimEnd('\', '/')
    $prefix = $root + [IO.Path]::DirectorySeparatorChar
    return @($assetIds | ForEach-Object {
        $full = [IO.Path]::GetFullPath((Join-Path $root $_))
        if (-not $full.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase) -or
            -not [IO.File]::Exists($full)) {
            throw "Build-domain resource is missing or escaped Resources: $_"
        }
        $item = Get-Item -LiteralPath $full
        [pscustomobject][ordered]@{
            path = "resource:$_"
            size = [int64]$item.Length
            sha256 = Get-BuildFileSha256 $full
            lastWriteTimeUtc = $item.LastWriteTimeUtc.ToString('o')
        }
    })
}

function Get-BuildCanonicalSha256 {
    param([Parameter(Mandatory = $true)]$Value)

    return Get-BuildSha256Text ($Value | ConvertTo-Json -Depth 30 -Compress)
}

function Read-BuildDomainManifest {
    param([Parameter(Mandatory = $true)][string]$Path)

    $document = Get-Content -LiteralPath $Path -Raw -Encoding UTF8 | ConvertFrom-Json
    if ($document.schema -cne $script:DomainGraphSchema -or
        [int]$document.formatVersion -ne 1) {
        throw "Unsupported build-domain graph: $Path"
    }
    $seen = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($domain in @($document.domains)) {
        if ([string]::IsNullOrWhiteSpace([string]$domain.id) -or
            [string]$domain.id -cnotmatch '^[a-z0-9][a-z0-9._-]*$' -or
            -not $seen.Add([string]$domain.id)) {
            throw "Build-domain IDs must be safe, non-empty, and unique: $($domain.id)"
        }
        if ([string]$domain.kind -notin @('validation', 'publisher')) {
            throw "Unsupported build-domain kind: $($domain.id): $($domain.kind)"
        }
        foreach ($profile in @($domain.profiles)) {
            if ([string]$profile -notin @('Product', 'Core', 'FullDiagnostic')) {
                throw "Unsupported build-domain profile: $($domain.id): $profile"
            }
        }
        foreach ($property in @(
                'inputs', 'tools', 'outputs', 'requiredOutputPatterns', 'action')) {
            if ($domain.PSObject.Properties.Name -notcontains $property) {
                throw "Build-domain definition is incomplete: $($domain.id): $property"
            }
        }
    }
    return $document
}

function Get-BuildDomainById {
    param(
        [Parameter(Mandatory = $true)]$Manifest,
        [Parameter(Mandatory = $true)][string]$DomainId
    )

    $matches = @($Manifest.domains | Where-Object { $_.id -ceq $DomainId })
    if ($matches.Count -ne 1) { throw "Unknown build domain: $DomainId" }
    return $matches[0]
}

function Get-BuildDomainsForProfile {
    param(
        [Parameter(Mandatory = $true)]$Manifest,
        [Parameter(Mandatory = $true)][string]$Profile
    )

    return @($Manifest.domains | Where-Object { @($_.profiles) -contains $Profile })
}

function Get-BuildDomainFingerprint {
    param(
        [Parameter(Mandatory = $true)][string]$RepositoryRoot,
        [Parameter(Mandatory = $true)]$Domain,
        [Parameter(Mandatory = $true)][string]$ResourceRoot
    )

    $inputs = @(Get-BuildFileRecords @(Resolve-BuildPatternFiles $RepositoryRoot `
        @($Domain.inputs)))
    $tools = @(Get-BuildFileRecords @(Resolve-BuildPatternFiles $RepositoryRoot `
        @($Domain.tools)))
    if ($inputs.Count -eq 0 -or $tools.Count -eq 0) {
        throw "Build-domain input/tool closure is empty: $($Domain.id)"
    }
    $resources = @(Get-BuildResourceRecords $RepositoryRoot $Domain $ResourceRoot)
    $outputs = @(Get-BuildFileRecords @(Resolve-BuildPatternFiles $RepositoryRoot `
        @($Domain.outputs)))
    $action = [pscustomobject][ordered]@{
        executable = [string]$Domain.action.executable
        arguments = @($Domain.action.arguments | ForEach-Object { [string]$_ })
    }
    $definition = [pscustomobject][ordered]@{
        id = [string]$Domain.id
        kind = [string]$Domain.kind
        profiles = @($Domain.profiles | ForEach-Object { [string]$_ })
        inputs = @($Domain.inputs | ForEach-Object { [string]$_ })
        tools = @($Domain.tools | ForEach-Object { [string]$_ })
        outputs = @($Domain.outputs | ForEach-Object { [string]$_ })
        requiredOutputPatterns = @($Domain.requiredOutputPatterns |
            ForEach-Object { [string]$_ })
        resourceReferenceDocuments = if (
            $Domain.PSObject.Properties.Name -contains 'resourceReferenceDocuments') {
            @($Domain.resourceReferenceDocuments | ForEach-Object { [string]$_ })
        }
        else { @() }
        resourceReferenceFields = if (
            $Domain.PSObject.Properties.Name -contains 'resourceReferenceFields') {
            @($Domain.resourceReferenceFields | ForEach-Object { [string]$_ })
        }
        else { @() }
        action = $action
    }
    $definitionSha256 = Get-BuildCanonicalSha256 $definition
    $source = [pscustomobject][ordered]@{
        inputs = @(Get-BuildContentIdentityRecords $inputs)
        tools = @(Get-BuildContentIdentityRecords $tools)
        resources = @(Get-BuildContentIdentityRecords $resources)
        definitionSha256 = $definitionSha256
    }
    return [pscustomobject][ordered]@{
        sourceInputSha256 = Get-BuildCanonicalSha256 $source
        fingerprintSha256 = Get-BuildCanonicalSha256 ([pscustomobject][ordered]@{
            source = $source
            outputs = @(Get-BuildContentIdentityRecords $outputs)
        })
        inputs = @($inputs)
        tools = @($tools)
        resources = @($resources)
        outputs = @($outputs)
        actionSha256 = Get-BuildCanonicalSha256 $action
        definitionSha256 = $definitionSha256
    }
}

function Get-BuildGitIdentity {
    param([Parameter(Mandatory = $true)][string]$RepositoryRoot)

    # A bare `powershell.exe` child process (how MSBuild invokes this pre-build
    # step) defaults its console output decoding to the OEM codepage (949 on
    # Korean Windows), not UTF-8. git's own output is UTF-8, so any non-ASCII
    # path captured below (e.g. a Korean-named PLAN/RESULT doc under .md/)
    # comes back mangled and fails [IO.Path]::GetFullPath with "path has
    # invalid characters". Force UTF-8 decoding for this function's git calls.
    $previousOutputEncoding = [Console]::OutputEncoding
    [Console]::OutputEncoding = [Text.Encoding]::UTF8
    try {

    $root = [IO.Path]::GetFullPath($RepositoryRoot)
    $head = (& git -C $root rev-parse --verify HEAD).Trim().ToLowerInvariant()
    if ($LASTEXITCODE -ne 0) { throw 'Cannot resolve build HEAD.' }
    $branch = (& git -C $root branch --show-current).Trim()
    if ($LASTEXITCODE -ne 0) { throw 'Cannot resolve build branch.' }
    [string[]]$dirtyLines = @(& git -C $root status --porcelain=v1 --untracked-files=all)
    if ($LASTEXITCODE -ne 0) { throw 'Cannot resolve build dirty state.' }
    [Array]::Sort($dirtyLines, [StringComparer]::Ordinal)
    [string[]]$trackedWorktreePaths = @(& git -C $root -c core.quotepath=false `
        ls-files --modified --deleted)
    if ($LASTEXITCODE -ne 0) { throw 'Cannot resolve modified build paths.' }
    [string[]]$trackedIndexPaths = @(& git -C $root -c core.quotepath=false `
        diff --cached --name-only --diff-filter=ACDMRTUXB --)
    if ($LASTEXITCODE -ne 0) { throw 'Cannot resolve staged build paths.' }
    [string[]]$untrackedPaths = @(& git -C $root -c core.quotepath=false `
        ls-files --others --exclude-standard)
    if ($LASTEXITCODE -ne 0) { throw 'Cannot resolve untracked build dirty paths.' }
    $dirtyFiles = [Collections.Generic.Dictionary[string,string]]::new(
        [StringComparer]::Ordinal)
    foreach ($path in @($trackedWorktreePaths) + @($trackedIndexPaths)) {
        if (-not [string]::IsNullOrWhiteSpace($path)) { $dirtyFiles[$path] = 'tracked' }
    }
    foreach ($path in $untrackedPaths) {
        if (-not [string]::IsNullOrWhiteSpace($path)) { $dirtyFiles[$path] = 'untracked' }
    }
    [string[]]$dirtyFilePaths = @($dirtyFiles.Keys)
    [Array]::Sort($dirtyFilePaths, [StringComparer]::Ordinal)
    $dirtyContentRecords = @($dirtyFilePaths | ForEach-Object {
        $relative = $_.Replace('\', '/')
        $full = [IO.Path]::GetFullPath((Join-Path $root $relative))
        $exists = [IO.File]::Exists($full)
        [pscustomobject][ordered]@{
            path = $relative
            kind = $dirtyFiles[$_]
            exists = $exists
            size = if ($exists) { [int64](Get-Item -LiteralPath $full).Length } else { 0L }
            sha256 = if ($exists) { Get-BuildFileSha256 $full } else { 'missing' }
        }
    })
    $dirtyIdentity = [pscustomobject][ordered]@{
        status = @($dirtyLines)
        files = @($dirtyContentRecords)
    }
    return [pscustomobject][ordered]@{
        head = $head
        branch = $branch
        dirty = $dirtyLines.Count -ne 0
        dirtyPathCount = $dirtyLines.Count
        dirtyIdentitySha256 = Get-BuildCanonicalSha256 $dirtyIdentity
    }

    } finally {
        [Console]::OutputEncoding = $previousOutputEncoding
    }
}

function Write-BuildAtomicJson {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)]$Value
    )

    $directory = [IO.Path]::GetDirectoryName([IO.Path]::GetFullPath($Path))
    [IO.Directory]::CreateDirectory($directory) | Out-Null
    $temporary = "$Path.tmp.$([Guid]::NewGuid().ToString('N'))"
    $backup = "$Path.rollback.$([Guid]::NewGuid().ToString('N'))"
    $encoding = [Text.UTF8Encoding]::new($false)
    [IO.File]::WriteAllText(
        $temporary, (($Value | ConvertTo-Json -Depth 30) + "`n"), $encoding)
    try {
        if ([IO.File]::Exists($Path)) {
            [IO.File]::Replace($temporary, $Path, $backup, $true)
            [IO.File]::Delete($backup)
        }
        else {
            [IO.File]::Move($temporary, $Path)
        }
    }
    catch {
        if ([IO.File]::Exists($backup) -and -not [IO.File]::Exists($Path)) {
            [IO.File]::Move($backup, $Path)
        }
        throw
    }
    finally {
        if ([IO.File]::Exists($temporary)) { [IO.File]::Delete($temporary) }
    }
}

function Enter-BuildExclusiveLock {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [ValidateRange(100, 3600000)][int]$TimeoutMilliseconds = 300000,
        [string]$Description = 'build operation'
    )

    $full = [IO.Path]::GetFullPath($Path)
    [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($full)) |
        Out-Null
    $timer = [Diagnostics.Stopwatch]::StartNew()
    $lastError = $null
    while ($timer.ElapsedMilliseconds -lt $TimeoutMilliseconds) {
        try {
            return [IO.File]::Open(
                $full,
                [IO.FileMode]::OpenOrCreate,
                [IO.FileAccess]::ReadWrite,
                [IO.FileShare]::None)
        }
        catch [IO.IOException] {
            $lastError = $_.Exception.Message
            Start-Sleep -Milliseconds 100
        }
    }
    throw "Timed out waiting for exclusive $Description lock after ${TimeoutMilliseconds}ms: $full. Last error: $lastError"
}

function Get-BuildDomainReceiptPath {
    param(
        [Parameter(Mandatory = $true)][string]$ReceiptRoot,
        [Parameter(Mandatory = $true)][string]$DomainId
    )

    return Join-Path $ReceiptRoot "$DomainId.receipt.json"
}

function Test-BuildDomainReceipt {
    param(
        [Parameter(Mandatory = $true)][string]$RepositoryRoot,
        [Parameter(Mandatory = $true)]$Domain,
        [Parameter(Mandatory = $true)][string]$ResourceRoot,
        [Parameter(Mandatory = $true)][string]$ReceiptRoot
    )

    $path = Get-BuildDomainReceiptPath $ReceiptRoot ([string]$Domain.id)
    if (-not [IO.File]::Exists($path)) {
        return [pscustomobject]@{ Fresh = $false; Reason = 'missing receipt'; Path = $path }
    }
    try {
        $receipt = Get-Content -LiteralPath $path -Raw -Encoding UTF8 | ConvertFrom-Json
        if ($receipt.schema -cne $script:DomainReceiptSchema -or
            [int]$receipt.formatVersion -ne $script:DomainReceiptFormatVersion -or
            $receipt.domainId -cne [string]$Domain.id -or
            [int]$receipt.exitCode -ne 0) {
            return [pscustomobject]@{ Fresh = $false; Reason = 'invalid receipt header'; Path = $path }
        }
        Assert-BuildRequiredOutputs $RepositoryRoot $Domain
        $current = Get-BuildDomainFingerprint $RepositoryRoot $Domain $ResourceRoot
        if ($receipt.fingerprintSha256 -cne $current.fingerprintSha256) {
            return [pscustomobject]@{ Fresh = $false; Reason = 'input or output fingerprint changed'; Path = $path }
        }
        return [pscustomobject]@{
            Fresh = $true
            Reason = 'matching input/output fingerprint'
            Path = $path
            Receipt = $receipt
            ReceiptSha256 = Get-BuildFileSha256 $path
            Fingerprint = $current
        }
    }
    catch {
        return [pscustomobject]@{ Fresh = $false; Reason = "receipt check failed: $($_.Exception.Message)"; Path = $path }
    }
}

function Assert-BuildRequiredOutputs {
    param(
        [Parameter(Mandatory = $true)][string]$RepositoryRoot,
        [Parameter(Mandatory = $true)]$Domain
    )

    foreach ($pattern in @($Domain.requiredOutputPatterns)) {
        if (@(Resolve-BuildPatternFiles $RepositoryRoot @([string]$pattern)).Count -eq 0) {
            throw "Build-domain required output is missing: $($Domain.id): $pattern"
        }
    }
}

function Invoke-BuildDomain {
    param(
        [Parameter(Mandatory = $true)][string]$RepositoryRoot,
        [Parameter(Mandatory = $true)]$Domain,
        [Parameter(Mandatory = $true)][string]$ResourceRoot,
        [Parameter(Mandatory = $true)][string]$ReceiptRoot,
        [ValidateRange(100, 3600000)][int]$LockTimeoutMilliseconds = 300000
    )

    $operationTimer = [Diagnostics.Stopwatch]::StartNew()
    $lockPath = Join-Path (Join-Path $ReceiptRoot 'locks') `
        "domain.$([string]$Domain.id).lock"
    $lock = Enter-BuildExclusiveLock $lockPath $LockTimeoutMilliseconds `
        "build domain '$([string]$Domain.id)'"
    try {
        # Recheck only after the exclusive lock is held. Another process may
        # have completed the same fingerprint while this caller was waiting.
        $fresh = Test-BuildDomainReceipt $RepositoryRoot $Domain $ResourceRoot `
            $ReceiptRoot
        if ($fresh.Fresh) {
            $operationTimer.Stop()
            return [pscustomobject][ordered]@{
                domainId = [string]$Domain.id
                kind = [string]$Domain.kind
                reused = $true
                elapsedMs = [int64]$operationTimer.ElapsedMilliseconds
                sourceInputSha256 = [string]$fresh.Fingerprint.sourceInputSha256
                fingerprintSha256 = [string]$fresh.Fingerprint.fingerprintSha256
                definitionSha256 = [string]$fresh.Fingerprint.definitionSha256
                receiptSha256 = [string]$fresh.ReceiptSha256
                outputs = @($fresh.Fingerprint.outputs)
                receiptPath = [string]$fresh.Path
            }
        }

        # Capture the complete source closure only after this domain's lock is
        # held. Publishers and validators may legitimately replace outputs,
        # but inputs, tools, resources, and the domain definition must remain
        # identical for the full action. Otherwise a result produced from the
        # old closure could be recorded as fresh for the new closure.
        $sourceBeforeAction = Get-BuildDomainFingerprint $RepositoryRoot $Domain `
            $ResourceRoot
        $actionTimer = [Diagnostics.Stopwatch]::StartNew()
        $arguments = @($Domain.action.arguments | ForEach-Object {
            Expand-BuildToken ([string]$_) $RepositoryRoot 'Shared' $ResourceRoot
        })
        $command = (Get-Command ([string]$Domain.action.executable) `
            -ErrorAction Stop).Source
        $previousPreference = $ErrorActionPreference
        $exitCode = -1
        Push-Location $RepositoryRoot
        try {
            $ErrorActionPreference = 'Continue'
            $global:LASTEXITCODE = 0
            # Keep native/publisher diagnostics visible without allowing their
            # success-stream output to become part of the structured result.
            & $command @arguments | Out-Host
            $exitCode = $global:LASTEXITCODE
        }
        finally {
            $ErrorActionPreference = $previousPreference
            Pop-Location
            $actionTimer.Stop()
        }
        if ($exitCode -ne 0) {
            throw "Build-domain action failed: $($Domain.id): exit=$exitCode"
        }
        Assert-BuildRequiredOutputs $RepositoryRoot $Domain
        $fingerprint = Get-BuildDomainFingerprint $RepositoryRoot $Domain `
            $ResourceRoot
        if ($fingerprint.sourceInputSha256 -cne
            $sourceBeforeAction.sourceInputSha256) {
            throw "Build-domain source closure changed during action; receipt was not written: $($Domain.id)"
        }
        $identity = Get-BuildGitIdentity $RepositoryRoot
        $receipt = [pscustomobject][ordered]@{
            schema = $script:DomainReceiptSchema
            formatVersion = $script:DomainReceiptFormatVersion
            domainId = [string]$Domain.id
            kind = [string]$Domain.kind
            exitCode = 0
            sourceInputSha256 = [string]$fingerprint.sourceInputSha256
            fingerprintSha256 = [string]$fingerprint.fingerprintSha256
            definitionSha256 = [string]$fingerprint.definitionSha256
            inputs = @($fingerprint.inputs)
            tools = @($fingerprint.tools)
            resources = @($fingerprint.resources)
            outputs = @($fingerprint.outputs)
            actionSha256 = [string]$fingerprint.actionSha256
            git = $identity
            elapsedMs = [int64]$actionTimer.ElapsedMilliseconds
            completedUtc = [DateTime]::UtcNow.ToString('o')
        }
        $path = Get-BuildDomainReceiptPath $ReceiptRoot ([string]$Domain.id)
        Write-BuildAtomicJson $path $receipt
        $operationTimer.Stop()
        return [pscustomobject][ordered]@{
            domainId = [string]$Domain.id
            kind = [string]$Domain.kind
            reused = $false
            elapsedMs = [int64]$operationTimer.ElapsedMilliseconds
            sourceInputSha256 = [string]$fingerprint.sourceInputSha256
            fingerprintSha256 = [string]$fingerprint.fingerprintSha256
            definitionSha256 = [string]$fingerprint.definitionSha256
            receiptSha256 = Get-BuildFileSha256 $path
            outputs = @($fingerprint.outputs)
            receiptPath = $path
        }
    }
    finally {
        $operationTimer.Stop()
        $lock.Dispose()
    }
}

function Get-BuildProductConfigurationValues {
    param(
        [Parameter(Mandatory = $true)]$Product,
        [Parameter(Mandatory = $true)][string]$PropertyName,
        [Parameter(Mandatory = $true)][string]$Configuration
    )

    $values = [Collections.Generic.List[object]]::new()
    if ($Product.PSObject.Properties.Name -contains $PropertyName) {
        foreach ($value in @($Product.$PropertyName)) { $values.Add($value) }
    }
    $byConfigurationName = "${PropertyName}ByConfiguration"
    if ($Product.PSObject.Properties.Name -contains $byConfigurationName) {
        $byConfiguration = $Product.$byConfigurationName
        $configurationProperty = $byConfiguration.PSObject.Properties[$Configuration]
        if ($null -ne $configurationProperty) {
            foreach ($value in @($configurationProperty.Value)) { $values.Add($value) }
        }
    }
    return @($values)
}

function Get-BuildExactProductOutputRecords {
    param(
        [Parameter(Mandatory = $true)][string]$RepositoryRoot,
        [Parameter(Mandatory = $true)]$Manifest,
        [Parameter(Mandatory = $true)][string]$Configuration
    )

    $patterns = @(Get-BuildProductConfigurationValues $Manifest.product `
        'outputs' $Configuration)
    $files = [Collections.Generic.List[object]]::new()
    $seen = [Collections.Generic.HashSet[string]]::new(
        [StringComparer]::OrdinalIgnoreCase)
    foreach ($pattern in $patterns) {
        $resolved = @(Resolve-BuildPatternFiles $RepositoryRoot `
            @([string]$pattern) $Configuration)
        if ($resolved.Count -ne 1) {
            throw "Product output pattern must resolve exactly once: ${pattern}: matches=$($resolved.Count)"
        }
        if (-not $seen.Add([string]$resolved[0].RelativePath)) {
            throw "Product output is owned by more than one pattern: $($resolved[0].RelativePath)"
        }
        $files.Add($resolved[0])
    }
    if ($files.Count -eq 0) { throw 'Product binary/PDB closure is empty.' }
    return @(Get-BuildFileRecords @($files))
}

function Assert-BuildProductDeploymentPairs {
    param(
        [Parameter(Mandatory = $true)][string]$RepositoryRoot,
        [Parameter(Mandatory = $true)]$Manifest,
        [Parameter(Mandatory = $true)][string]$Configuration
    )

    $pairs = @(Get-BuildProductConfigurationValues $Manifest.product `
        'deploymentPairs' $Configuration)
    foreach ($pair in $pairs) {
        $source = @(Resolve-BuildPatternFiles $RepositoryRoot `
            @([string]$pair.source) $Configuration)
        $destination = @(Resolve-BuildPatternFiles $RepositoryRoot `
            @([string]$pair.destination) $Configuration)
        if ($source.Count -ne 1 -or $destination.Count -ne 1) {
            throw "Product deployment pair is incomplete: $($pair.source) -> $($pair.destination)"
        }
        $sourceHash = Get-BuildFileSha256 $source[0].FullPath
        $destinationHash = Get-BuildFileSha256 $destination[0].FullPath
        if ($sourceHash -cne $destinationHash) {
            throw "Product deployment hash mismatch: $($pair.source) -> $($pair.destination)"
        }
    }
}

function Get-BuildProductSourceFingerprint {
    param(
        [Parameter(Mandatory = $true)][string]$RepositoryRoot,
        [Parameter(Mandatory = $true)]$Manifest,
        [Parameter(Mandatory = $true)][string]$Configuration
    )

    $inputs = @(Get-BuildFileRecords @(Resolve-BuildPatternFiles $RepositoryRoot `
        @($Manifest.product.inputs) $Configuration))
    if ($inputs.Count -eq 0) { throw 'Product source closure is empty.' }
    $definition = [pscustomobject][ordered]@{
        configuration = $Configuration
        inputs = @($Manifest.product.inputs | ForEach-Object { [string]$_ })
        outputs = @(Get-BuildProductConfigurationValues $Manifest.product `
            'outputs' $Configuration | ForEach-Object { [string]$_ })
        deploymentPairs = @(Get-BuildProductConfigurationValues `
            $Manifest.product 'deploymentPairs' $Configuration)
    }
    $sourceHash = Get-BuildCanonicalSha256 ([pscustomobject][ordered]@{
        definitionSha256 = Get-BuildCanonicalSha256 $definition
        inputs = @(Get-BuildContentIdentityRecords $inputs)
    })
    return [pscustomobject][ordered]@{
        sourceInputSha256 = $sourceHash
        definitionSha256 = Get-BuildCanonicalSha256 $definition
        inputs = @($inputs)
    }
}

function Get-BuildProductFingerprint {
    param(
        [Parameter(Mandatory = $true)][string]$RepositoryRoot,
        [Parameter(Mandatory = $true)]$Manifest,
        [Parameter(Mandatory = $true)][string]$Configuration
    )

    $source = Get-BuildProductSourceFingerprint $RepositoryRoot $Manifest `
        $Configuration
    Assert-BuildProductDeploymentPairs $RepositoryRoot $Manifest $Configuration
    $outputs = @(Get-BuildExactProductOutputRecords $RepositoryRoot $Manifest `
        $Configuration)
    return [pscustomobject][ordered]@{
        sourceInputSha256 = $source.sourceInputSha256
        definitionSha256 = $source.definitionSha256
        fingerprintSha256 = Get-BuildCanonicalSha256 ([pscustomobject][ordered]@{
            configuration = $Configuration
            sourceInputSha256 = $source.sourceInputSha256
            outputs = @(Get-BuildContentIdentityRecords $outputs)
        })
        inputs = @($source.inputs)
        outputs = @($outputs)
    }
}

function Get-BuildProductReceiptPath {
    param(
        [Parameter(Mandatory = $true)][string]$ReceiptRoot,
        [Parameter(Mandatory = $true)][string]$Configuration
    )

    return Join-Path $ReceiptRoot "product.$($Configuration.ToLowerInvariant()).receipt.json"
}

function Write-BuildProductReceipt {
    param(
        [Parameter(Mandatory = $true)][string]$RepositoryRoot,
        [Parameter(Mandatory = $true)]$Manifest,
        [Parameter(Mandatory = $true)][string]$Configuration,
        [Parameter(Mandatory = $true)][string]$ReceiptRoot
    )

    $fingerprint = Get-BuildProductFingerprint $RepositoryRoot $Manifest $Configuration
    $receipt = [pscustomobject][ordered]@{
        schema = $script:ProductReceiptSchema
        formatVersion = $script:ProductReceiptFormatVersion
        configuration = $Configuration
        sourceInputSha256 = $fingerprint.sourceInputSha256
        fingerprintSha256 = $fingerprint.fingerprintSha256
        definitionSha256 = $fingerprint.definitionSha256
        inputs = @($fingerprint.inputs)
        outputs = @($fingerprint.outputs)
        git = Get-BuildGitIdentity $RepositoryRoot
        completedUtc = [DateTime]::UtcNow.ToString('o')
    }
    $path = Get-BuildProductReceiptPath $ReceiptRoot $Configuration
    Write-BuildAtomicJson $path $receipt
    return $path
}

function Test-BuildProductReceipt {
    param(
        [Parameter(Mandatory = $true)][string]$RepositoryRoot,
        [Parameter(Mandatory = $true)]$Manifest,
        [Parameter(Mandatory = $true)][string]$Configuration,
        [Parameter(Mandatory = $true)][string]$ReceiptRoot
    )

    $path = Get-BuildProductReceiptPath $ReceiptRoot $Configuration
    if (-not [IO.File]::Exists($path)) {
        return [pscustomobject]@{ Fresh = $false; Reason = 'missing product receipt'; Path = $path }
    }
    try {
        $receipt = Get-Content -LiteralPath $path -Raw -Encoding UTF8 | ConvertFrom-Json
        if ($receipt.schema -cne $script:ProductReceiptSchema -or
            [int]$receipt.formatVersion -ne $script:ProductReceiptFormatVersion -or
            $receipt.configuration -cne $Configuration) {
            return [pscustomobject]@{ Fresh = $false; Reason = 'invalid product receipt header'; Path = $path }
        }
        $current = Get-BuildProductFingerprint $RepositoryRoot $Manifest $Configuration
        if ($receipt.fingerprintSha256 -cne $current.fingerprintSha256) {
            return [pscustomobject]@{ Fresh = $false; Reason = 'source or binary/PDB fingerprint changed'; Path = $path }
        }
        return [pscustomobject]@{
            Fresh = $true
            Reason = 'matching source and binary/PDB/runtime fingerprint'
            Path = $path
            Receipt = $receipt
            ReceiptSha256 = Get-BuildFileSha256 $path
            Fingerprint = $current
        }
    }
    catch {
        return [pscustomobject]@{ Fresh = $false; Reason = "product receipt check failed: $($_.Exception.Message)"; Path = $path }
    }
}

function Assert-BuildRunStability {
    param(
        [Parameter(Mandatory = $true)][string]$RepositoryRoot,
        [Parameter(Mandatory = $true)]$Manifest,
        [Parameter(Mandatory = $true)][string]$Configuration,
        [Parameter(Mandatory = $true)][string]$ResourceRoot,
        [Parameter(Mandatory = $true)][string]$ReceiptRoot,
        [Parameter(Mandatory = $true)]$StartGitIdentity,
        [Parameter(Mandatory = $true)][string]$StartProductSourceInputSha256,
        [Parameter(Mandatory = $true)][object[]]$DomainResults
    )

    $currentGit = Get-BuildGitIdentity $RepositoryRoot
    if ($currentGit.head -cne $StartGitIdentity.head -or
        $currentGit.dirtyIdentitySha256 -cne
            $StartGitIdentity.dirtyIdentitySha256) {
        throw 'Build source identity changed after compilation started.'
    }
    $currentSource = Get-BuildProductSourceFingerprint $RepositoryRoot `
        $Manifest $Configuration
    if ($currentSource.sourceInputSha256 -cne $StartProductSourceInputSha256) {
        throw 'Product source fingerprint changed after compilation started.'
    }
    $product = Test-BuildProductReceipt $RepositoryRoot $Manifest $Configuration `
        $ReceiptRoot
    if (-not $product.Fresh) {
        throw "Product receipt became stale before evidence: $($product.Reason)"
    }
    if ($product.Fingerprint.sourceInputSha256 -cne
        $StartProductSourceInputSha256) {
        throw 'Product receipt does not belong to the build-start source fingerprint.'
    }

    foreach ($domainResult in $DomainResults) {
        $domain = Get-BuildDomainById $Manifest ([string]$domainResult.domainId)
        $current = Test-BuildDomainReceipt $RepositoryRoot $domain $ResourceRoot `
            $ReceiptRoot
        if (-not $current.Fresh) {
            throw "Build domain became stale before evidence: $($domain.id): $($current.Reason)"
        }
        if ($current.Fingerprint.fingerprintSha256 -cne
                [string]$domainResult.fingerprintSha256 -or
            $current.ReceiptSha256 -cne [string]$domainResult.receiptSha256) {
            throw "Build domain identity changed before evidence: $($domain.id)"
        }
    }

    return [pscustomobject][ordered]@{
        StartGit = $StartGitIdentity
        CurrentGit = $currentGit
        Product = $product
    }
}

function Write-BuildRunEvidence {
    param(
        [Parameter(Mandatory = $true)][string]$RepositoryRoot,
        [Parameter(Mandatory = $true)][string]$Configuration,
        [Parameter(Mandatory = $true)][string]$Profile,
        [Parameter(Mandatory = $true)][bool]$SkipBuild,
        [Parameter(Mandatory = $true)][object[]]$Steps,
        [Parameter(Mandatory = $true)][object[]]$DomainResults,
        [Parameter(Mandatory = $true)][string]$EvidenceRoot,
        [Parameter(Mandatory = $true)]$Stability,
        [Parameter(Mandatory = $true)][string]$StartedUtc,
        [Parameter(Mandatory = $true)][int64]$ElapsedMilliseconds
    )

    $product = $Stability.Product
    $document = [pscustomobject][ordered]@{
        schema = $script:RunEvidenceSchema
        formatVersion = 2
        configuration = $Configuration
        profile = $Profile
        skipBuild = $SkipBuild
        buildStartGit = $Stability.StartGit
        git = $Stability.CurrentGit
        sourceInputSha256 = $product.Receipt.sourceInputSha256
        productFingerprintSha256 = $product.Receipt.fingerprintSha256
        productReceiptSha256 = $product.ReceiptSha256
        binaries = @($product.Receipt.outputs)
        domains = @($DomainResults)
        steps = @($Steps)
        startedUtc = $StartedUtc
        elapsedMs = $ElapsedMilliseconds
        completedUtc = [DateTime]::UtcNow.ToString('o')
    }
    [IO.Directory]::CreateDirectory($EvidenceRoot) | Out-Null
    $stamp = [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffZ')
    $suffix = [Guid]::NewGuid().ToString('N').Substring(0, 8)
    $path = Join-Path $EvidenceRoot `
        "$stamp-$($Configuration.ToLowerInvariant())-$($Profile.ToLowerInvariant())-$suffix.json"
    Write-BuildAtomicJson $path $document
    return $path
}

Export-ModuleMember -Function @(
    'Write-BuildAtomicJson',
    'Read-BuildDomainManifest',
    'Get-BuildDomainById',
    'Get-BuildDomainsForProfile',
    'Get-BuildDomainFingerprint',
    'Test-BuildDomainReceipt',
    'Invoke-BuildDomain',
    'Enter-BuildExclusiveLock',
    'Get-BuildGitIdentity',
    'Get-BuildProductSourceFingerprint',
    'Get-BuildProductFingerprint',
    'Write-BuildProductReceipt',
    'Test-BuildProductReceipt',
    'Assert-BuildRunStability',
    'Write-BuildRunEvidence'
)
