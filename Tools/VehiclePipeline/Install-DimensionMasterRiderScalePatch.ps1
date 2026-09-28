#Requires -Version 5.1
<#
.SYNOPSIS
Installs the six measured DimensionMaster rider animation donors.
.DESCRIPTION
Run from the extracted patch beside its unchanged repair receipt and Resources
payload. ResourceRoot must be the existing Resources folder used by the Client.
No executable, body model, rendering option, or other resource is changed.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$ResourceRoot,
    [switch]$CheckOnly
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Get-PlainFile([string]$Root, [string]$Relative) {
    # All callers supply an exact allowlisted relative path. Reject links so a
    # payload or destination cannot redirect a replacement outside its root.
    $cursor = $Root
    foreach ($segment in $Relative.Split('/')) {
        $cursor = Join-Path $cursor $segment
        $item = Get-Item -LiteralPath $cursor -Force
        if (($item.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
            throw "Linked resource paths are not supported: $cursor"
        }
    }
    if ($item.PSIsContainer) { throw "Expected a file: $cursor" }
    return $item.FullName
}

function Get-Hash([string]$Path) {
    $stream = [IO.File]::Open($Path, 'Open', 'Read', 'Read, Delete')
    $sha256 = [Security.Cryptography.SHA256]::Create()
    try {
        return [BitConverter]::ToString($sha256.ComputeHash($stream)).Replace('-', '').ToLowerInvariant()
    } finally {
        $sha256.Dispose()
        $stream.Dispose()
    }
}

function Assert-Hash([string]$Path, [string]$Expected, [string]$Reason) {
    if ((Get-Hash $Path) -cne $Expected) { throw "${Reason}: $Path" }
}

$rootItem = Get-Item -LiteralPath $ResourceRoot -Force
if (-not $rootItem.PSIsContainer -or $rootItem.PSProvider.Name -ne 'FileSystem') {
    throw 'ResourceRoot must be an existing filesystem directory.'
}
if (($rootItem.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
    throw 'ResourceRoot must be the physical Resources directory, not a link.'
}
$parent = [IO.Directory]::GetParent($rootItem.FullName)
if ($null -eq $parent) { throw 'ResourceRoot cannot be a volume root.' }
$targetRoot = $rootItem.FullName.TrimEnd([char[]]'\/')
$payloadRoot = Join-Path $PSScriptRoot 'Resources'
$receiptPath = Join-Path $PSScriptRoot 'DimensionMasterRiderScaleRepair.receipt.json'
$receipt = Get-Content -LiteralPath $receiptPath -Raw -Encoding UTF8 | ConvertFrom-Json
$bodyResource = 'Character/DimensionMaster/DimensionMaster_Character.wmodel'
$allowed = @('Dragon2', 'HeavywalkerBm9', 'Horse', 'Hoverboard', 'Swing', 'Tube') |
    ForEach-Object { "Character/DimensionMaster/AnimSets/DimensionMaster_Ride${_}AnimSet.wmodel" }
if ($receipt.schema -cne 'lostark.dimensionmaster-rider-scale-repair' -or
    $receipt.formatVersion -ne 1 -or $receipt.bodyResource -cne $bodyResource -or
    $receipt.bodySha256 -cnotmatch '^[0-9a-f]{64}$' -or @($receipt.files).Count -ne 6) {
    throw 'Unsupported or malformed repair receipt.'
}

$bodyPath = Get-PlainFile $targetRoot $bodyResource
# The body must stay unchanged while its compatible animation donors are installed.
$bodyLock = [IO.File]::Open($bodyPath, 'Open', 'Read', 'Read')
$backupRoot = $null
$written = [Collections.Generic.List[object]]::new()
$prepared = [Collections.Generic.List[object]]::new()
try {
    Assert-Hash $bodyPath $receipt.bodySha256 'Unrecognized DimensionMaster body'
    $seen = @{}
    foreach ($row in $receipt.files) {
        $relative = [string]$row.resource
        if ($allowed -cnotcontains $relative -or $seen.ContainsKey($relative) -or
            $row.beforeSha256 -cnotmatch '^[0-9a-f]{64}$' -or
            $row.afterSha256 -cnotmatch '^[0-9a-f]{64}$' -or
            $row.beforeSha256 -ceq $row.afterSha256 -or
            $row.bytes -isnot [ValueType] -or $row.bytes -le 0) {
            throw "Invalid or duplicate donor in repair receipt: $relative"
        }
        $seen[$relative] = $true
        $payload = Get-PlainFile $payloadRoot $relative
        $target = Get-PlainFile $targetRoot $relative
        if ((Get-Item -LiteralPath $payload).Length -ne $row.bytes -or
            (Get-Item -LiteralPath $target).Length -ne $row.bytes) {
            throw "Unexpected donor file size: $relative"
        }
        Assert-Hash $payload $row.afterSha256 'Corrupt corrected payload'
        $current = Get-Hash $target
        if ($current -cne $row.beforeSha256 -and $current -cne $row.afterSha256) {
            throw "Unrecognized donor; no resources changed: $target"
        }
        $prepared.Add([pscustomobject]@{
            Relative = $relative; Payload = $payload; Target = $target
            Before = $current; After = [string]$row.afterSha256
            Stage = $null; Backup = $null
        })
    }

    $pending = @($prepared | Where-Object { $_.Before -cne $_.After })
    $already = $prepared.Count - $pending.Count
    if ($CheckOnly -or $pending.Count -eq 0) {
        [pscustomobject]@{
            ResourceRoot = $targetRoot; CheckOnly = [bool]$CheckOnly
            Updated = 0; WouldUpdate = $pending.Count; AlreadyCorrected = $already
            BackupPath = $null
        }
        return
    }

    # A sibling directory is outside Resources and on the same filesystem.
    # Keep backups on both success and failure. Never delete backup directories.
    $backupRoot = Join-Path $parent.FullName (
        'DMR-backup-' + (Get-Date -Format 'yyyyMMdd-HHmmss') +
        '-' + [Guid]::NewGuid().ToString('N').Substring(0, 12))
    [IO.Directory]::CreateDirectory($backupRoot) | Out-Null
    foreach ($entry in $pending) {
        $leaf = [IO.Path]::GetFileName($entry.Target)
        $entry.Stage = Join-Path $backupRoot ($leaf + '.pending')
        $entry.Backup = Join-Path $backupRoot $leaf
        [IO.File]::Copy($entry.Payload, $entry.Stage, $false)
        Assert-Hash $entry.Stage $entry.After 'Staged payload changed'
    }
    # No destination is touched until every payload, destination and staged copy
    # is admitted. Recheck every file, including the already-corrected donors.
    foreach ($entry in $prepared) {
        Get-PlainFile $targetRoot $entry.Relative | Out-Null
        Assert-Hash $entry.Target $entry.Before 'Concurrent resource change'
    }
    foreach ($entry in $pending) {
        Get-PlainFile $targetRoot $entry.Relative | Out-Null
        # Deny in-place writes through replacement. Delete sharing is required by
        # File.Replace; verify its actual backup as well to detect a rename race.
        $targetLock = [IO.File]::Open($entry.Target, 'Open', 'Read', 'Read, Delete')
        try {
            Assert-Hash $entry.Target $entry.Before 'Concurrent resource change'
            Assert-Hash $entry.Stage $entry.After 'Staged payload changed'
            [IO.File]::Replace($entry.Stage, $entry.Target, $entry.Backup)
            $written.Add($entry)
            Assert-Hash $entry.Backup $entry.Before 'Concurrent replacement detected'
            Assert-Hash $entry.Target $entry.After 'Installed donor changed'
        } finally {
            $targetLock.Dispose()
        }
    }
    foreach ($entry in $prepared) {
        Assert-Hash $entry.Target $entry.After 'Resource changed during installation'
    }
    [pscustomobject]@{
        ResourceRoot = $targetRoot; CheckOnly = $false
        Updated = $written.Count; WouldUpdate = 0; AlreadyCorrected = $already
        BackupPath = $backupRoot
    }
} catch {
    $failure = $_
    for ($index = $written.Count - 1; $index -ge 0; --$index) {
        $entry = $written[$index]
        try {
            Get-PlainFile $targetRoot $entry.Relative | Out-Null
            $rollbackLock = [IO.File]::Open($entry.Target, 'Open', 'Read', 'Read, Delete')
            try {
                # Preserve an external edit instead of reverting it. The backup
                # is the exact previous file captured atomically by File.Replace.
                if ((Get-Hash $entry.Target) -cne $entry.After) {
                    throw 'Target no longer matches this installation.'
                }
                $rollbackStage = $entry.Backup + '.rollback'
                [IO.File]::Copy($entry.Backup, $rollbackStage, $false)
                Assert-Hash $entry.Target $entry.After 'Concurrent change during rollback'
                [IO.File]::Replace($rollbackStage, $entry.Target, $entry.Backup + '.reverted')
            } finally {
                $rollbackLock.Dispose()
            }
        } catch {
            Write-Warning "Rollback skipped/failed for $($entry.Target): $($_.Exception.Message)"
        }
    }
    if ($backupRoot) { Write-Warning "Preserved backup/staging directory: $backupRoot" }
    throw $failure
} finally {
    $bodyLock.Dispose()
}
