[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateSet('Client', 'Server', 'KoukuSaydon')]
    [string]$Owner,
    [string]$ResourceRoot = '',
    [uint32]$ExpectedKoukuSaydonSourceRevision = 0,
    [ValidateRange(100, 3600000)]
    [int]$LockTimeoutMilliseconds = 300000
)

$ErrorActionPreference = 'Stop'
# Publishers pipe native tool output (git, python) that is UTF-8 and can
# carry this repository's non-ASCII path. Under msbuild the console is the
# ANSI code page, which corrupts that output before ConvertFrom-Json or
# GetFullPath ever sees it, so the whole owner process decodes as UTF-8.
[Console]::OutputEncoding = [Text.UTF8Encoding]::new($false)
$OutputEncoding = [Text.UTF8Encoding]::new($false)
$repositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
if ([string]::IsNullOrWhiteSpace($ResourceRoot)) {
    $ResourceRoot = Join-Path $repositoryRoot 'Client\Bin\Resources'
}
else {
    $ResourceRoot = [IO.Path]::GetFullPath($ResourceRoot)
}
$receiptRoot = Join-Path $repositoryRoot 'out\BuildPipeline\receipts'
$modulePath = Join-Path $PSScriptRoot 'BuildDomainPipeline.psm1'
$manifestPath = Join-Path $PSScriptRoot 'BuildDomains.json'
Import-Module $modulePath -Force
$manifest = Read-BuildDomainManifest $manifestPath

function Assert-KoukuSaydonSourceRevision([string]$Phase) {
    if ($Owner -ne 'KoukuSaydon' -or
        $ExpectedKoukuSaydonSourceRevision -eq 0) {
        return
    }
    $sourcePath = Join-Path $repositoryRoot `
        'Data\KoukuSaydon\Gate1\KoukuSaydonComposition.json'
    try {
        $source = Get-Content -LiteralPath $sourcePath -Raw -Encoding UTF8 |
            ConvertFrom-Json
    }
    catch {
        throw "STALE_REVISION: KoukuSaydon source is unreadable during ${Phase}: $($_.Exception.Message)"
    }
    if ([string]$source.schema -cne 'lostark.kouku-saydon-composition' -or
        [uint32]$source.formatVersion -notin @(1, 2, 3) -or
        [uint32]$source.revision -ne $ExpectedKoukuSaydonSourceRevision) {
        throw "STALE_REVISION: expected KoukuSaydon source revision $ExpectedKoukuSaydonSourceRevision during ${Phase}, found $($source.revision)."
    }
}
# Explicit runtime-data generation; compilation does not call this by default.
# Pure validation domains are selected by diagnostic profiles, not publishers.
$domainIds = if ($Owner -eq 'Client') {
    @(
        'koukusaydon.product',
        'map.kakulsaydon',
        'composition.presentation',
        'world.gameplay',
        'navigation'
    )
}
elseif ($Owner -eq 'KoukuSaydon') {
    @(
        'koukusaydon.product',
        'map.kakulsaydon',
        'world.gameplay',
        'gameplay.balance'
    )
}
else {
    @(
        'koukusaydon.product',
        'world.gameplay',
        'navigation',
        'world.destruction',
        'gameplay.balance',
        'items.catalog',
        'valtan.rewards'
    )
}

function Get-OwnerBuildDomain([string]$Id) {
    $domain = Get-BuildDomainById $manifest $Id
    if ($Owner -ne 'KoukuSaydon' -or $Id -ne 'world.gameplay') { return $domain }
    # Use the same world publisher and codec, with only this encounter's outputs.
    # Its action/output fingerprint distinguishes this receipt from an all-world run.
    $scoped = $domain | ConvertTo-Json -Depth 30 | ConvertFrom-Json
    $scoped.action.arguments = @($scoped.action.arguments) + @('-WorldId', 'KAKULSAYDON_ARENA')
    $scoped.outputs = @(
        'Server/Bin/DataFiles/World/KAKULSAYDON_ARENA.worldbootstrap',
        'Server/Bin/DataFiles/World/KAKULSAYDON_ARENA.spawngroupsbootstrap',
        'Server/Bin/DataFiles/World/KAKULSAYDON_ARENA.encounterpropsbootstrap',
        'Client/Bin/DataFiles/World/KAKULSAYDON_ARENA.npcpresentation.json',
        'Client/Bin/DataFiles/World/KAKULSAYDON_ARENA.stagemarkers.json',
        'Client/Bin/DataFiles/World/LV_LUT_MIDNIGHTC_ED.viewer.world.json',
        'Client/Bin/DataFiles/World/SequenceViewer.labels.json'
    )
    $scoped.requiredOutputPatterns = @(
        'Server/Bin/DataFiles/World/KAKULSAYDON_ARENA.worldbootstrap',
        'Client/Bin/DataFiles/World/KAKULSAYDON_ARENA.npcpresentation.json',
        'Client/Bin/DataFiles/World/KAKULSAYDON_ARENA.stagemarkers.json',
        'Client/Bin/DataFiles/World/LV_LUT_MIDNIGHTC_ED.viewer.world.json',
        'Client/Bin/DataFiles/World/SequenceViewer.labels.json'
    )
    return $scoped
}

function Get-KoukuPublishTransactionPaths {
    $paths = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
    foreach ($domainId in $domainIds) {
        $domain = Get-OwnerBuildDomain $domainId
        foreach ($pattern in @($domain.outputs)) {
            $joined = Join-Path $repositoryRoot ([string]$pattern)
            $parent = [IO.Path]::GetFullPath((Split-Path -Parent $joined))
            $candidate = Join-Path $parent (Split-Path -Leaf $joined)
            if (-not $parent.StartsWith($repositoryRoot + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) {
                throw "Publish transaction output escapes the repository: $candidate"
            }
            if ([Management.Automation.WildcardPattern]::ContainsWildcardCharacters($candidate)) {
                foreach ($file in @(Get-ChildItem -Path $candidate -File -ErrorAction SilentlyContinue)) {
                    $null = $paths.Add($file.FullName)
                }
            }
            else { $null = $paths.Add($candidate) }
        }
        $null = $paths.Add((Join-Path $receiptRoot "$domainId.receipt.json"))
    }
    return $paths
}

# Every domain-owner invocation shares this lock so Client/Server publication
# cannot replace a file while the Kouku pattern/world/balance transaction is rolling back.
$ownerLock = Enter-BuildExclusiveLock (Join-Path $receiptRoot 'locks\runtime-owner.lock') `
    $LockTimeoutMilliseconds 'runtime domain owner'
$backups = @{}
$transactionId = [Guid]::NewGuid().ToString('N')
$backupDirectory = Join-Path $receiptRoot "transactions\kouku-$transactionId"
$preserveBackups = $false
try {
    if ($Owner -eq 'KoukuSaydon') {
        [IO.Directory]::CreateDirectory($backupDirectory) | Out-Null
        foreach ($path in @(Get-KoukuPublishTransactionPaths)) {
            $backup = $null
            if ([IO.File]::Exists($path)) {
                $backup = Join-Path $backupDirectory "$($backups.Count).$([IO.Path]::GetFileName($path)).rollback"
                $backups[$path] = $backup
                [IO.File]::Copy($path, $backup, $false)
            }
            $backups[$path] = $backup
        }
    }
    try {
        foreach ($domainId in $domainIds) {
            Assert-KoukuSaydonSourceRevision "domain $domainId start"
            $domain = Get-OwnerBuildDomain $domainId
            $result = Invoke-BuildDomain $repositoryRoot $domain $ResourceRoot `
                $receiptRoot -LockTimeoutMilliseconds $LockTimeoutMilliseconds
            $state = if ($result.reused) { 'REUSED' } else { 'PASS' }
            Write-Host "Build domain $domainId for ${Owner}: $state"
            Assert-KoukuSaydonSourceRevision "domain $domainId completion"
        }
    }
    catch {
        $publishError = $_
        if ($Owner -eq 'KoukuSaydon') {
            $restoreErrors = [Collections.Generic.List[string]]::new()
            $restorePaths = @(Get-KoukuPublishTransactionPaths) + @($backups.Keys)
            foreach ($path in @($restorePaths | Select-Object -Unique)) {
                try {
                    $resolved = [IO.Path]::GetFullPath($path)
                    if (-not $resolved.StartsWith($repositoryRoot + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) {
                        throw "Rollback path escapes the repository: $resolved"
                    }
                    $backup = $backups[$path]
                    if ($null -eq $backup) {
                        if ([IO.File]::Exists($resolved)) { [IO.File]::Delete($resolved) }
                    }
                    else {
                        $staged = "$resolved.koukupublish.$transactionId.restore"
                        $discarded = "$resolved.koukupublish.$transactionId.discard"
                        [IO.File]::Copy($backup, $staged, $true)
                        try {
                            if ([IO.File]::Exists($resolved)) { [IO.File]::Replace($staged, $resolved, $discarded, $true) }
                            else { [IO.File]::Move($staged, $resolved) }
                        }
                        finally {
                            if ([IO.File]::Exists($staged)) { [IO.File]::Delete($staged) }
                            if ([IO.File]::Exists($discarded)) { [IO.File]::Delete($discarded) }
                        }
                    }
                }
                catch { $restoreErrors.Add("${path}: $($_.Exception.Message)") }
            }
            if ($restoreErrors.Count -gt 0) {
                $preserveBackups = $true
                throw "KoukuSaydon publication failed: $publishError. Rollback incomplete; backups preserved in ${backupDirectory}: $($restoreErrors -join '; ')"
            }
            Write-Host 'KoukuSaydon publication failed; previous Product files, Server data and receipts restored.'
        }
        throw $publishError
    }
}
finally {
    try {
        if (-not $preserveBackups) {
            foreach ($backup in $backups.Values) {
                if ($null -ne $backup -and [IO.File]::Exists($backup)) { [IO.File]::Delete($backup) }
            }
            if ([IO.Directory]::Exists($backupDirectory)) { [IO.Directory]::Delete($backupDirectory, $false) }
        }
    }
    catch { Write-Warning "Publish transaction backup cleanup failed in ${backupDirectory}: $($_.Exception.Message)" }
    finally { $ownerLock.Dispose() }
}
