[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$CandidateRoot,

    [ValidateSet('Check', 'Install')]
    [string]$Mode = 'Check',

    [string]$ClientBinRoot = 'Client/Bin',

    [string]$BackupRoot
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Get-FileSha256 {
    param([Parameter(Mandatory = $true)][string]$Path)
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash
}

function Assert-WModel {
    param([Parameter(Mandatory = $true)][string]$Path)

    $stream = [System.IO.File]::OpenRead($Path)
    try {
        $header = New-Object byte[] 20
        $count = $stream.Read($header, 0, $header.Length)
    }
    finally {
        $stream.Dispose()
    }
    if ($count -ne 20 -or [System.Text.Encoding]::ASCII.GetString($header, 0, 4) -ne 'WINT' -or [System.Text.Encoding]::ASCII.GetString($header, 16, 4) -ne 'WMOD') {
        throw "Invalid WModel header: $Path"
    }
}

function Replace-FileAtomically {
    param(
        [Parameter(Mandatory = $true)][string]$Source,
        [Parameter(Mandatory = $true)][string]$Destination
    )

    $temporary = "$Destination.preview-recovery.tmp"
    $replacementBackup = "$Destination.preview-recovery.replace-backup"
    Remove-Item -LiteralPath $temporary -Force -ErrorAction SilentlyContinue
    Remove-Item -LiteralPath $replacementBackup -Force -ErrorAction SilentlyContinue
    try {
        Copy-Item -LiteralPath $Source -Destination $temporary -Force
        if (Test-Path -LiteralPath $Destination -PathType Leaf) {
            [System.IO.File]::Replace($temporary, $Destination, $replacementBackup)
            Remove-Item -LiteralPath $replacementBackup -Force -ErrorAction Stop
        }
        else {
            Move-Item -LiteralPath $temporary -Destination $Destination
        }
    }
    finally {
        Remove-Item -LiteralPath $temporary -Force -ErrorAction SilentlyContinue
        Remove-Item -LiteralPath $replacementBackup -Force -ErrorAction SilentlyContinue
    }
}

$candidate = (Resolve-Path -LiteralPath $CandidateRoot).Path
$clientBin = (Resolve-Path -LiteralPath $ClientBinRoot).Path
$candidateLandscape = Join-Path $candidate 'Resources/Map/LV_BER_BERNCASTLE_T/Landscape'
$targetLandscape = Join-Path $clientBin 'Resources/Map/LV_BER_BERNCASTLE_T/Landscape'
$reportPath = Join-Path $candidate 'Reports/extraction_report.json'

if (-not (Test-Path -LiteralPath $candidateLandscape -PathType Container)) {
    throw "Candidate does not contain the Bern Landscape Resources tree: $candidateLandscape"
}
if (-not (Test-Path -LiteralPath $targetLandscape -PathType Container)) {
    throw "Installed Bern Landscape Resources tree is missing: $targetLandscape"
}
if (-not (Test-Path -LiteralPath $reportPath -PathType Leaf)) {
    throw "Candidate extraction report is missing: $reportPath"
}

$report = Get-Content -LiteralPath $reportPath -Raw | ConvertFrom-Json
if ($report.status -ne 'PASS' -or $report.componentCount -ne 42 -or $report.wmodelCount -ne 42 -or $report.collisionHeightMismatchCount -ne 0) {
    throw 'Candidate extraction report is not a validated 42-component Bern Landscape pack.'
}

$candidateFiles = Get-ChildItem -LiteralPath $candidateLandscape -Recurse -File |
    Where-Object { $_.Extension -in '.wmodel', '.png', '.dds' } |
    Sort-Object FullName
if ($candidateFiles.Count -lt 84) {
    throw "Candidate has too few runtime model/texture files: $($candidateFiles.Count)"
}

$changes = foreach ($source in $candidateFiles) {
    $relativePath = $source.FullName.Substring($candidateLandscape.Length).TrimStart('\', '/')
    $destination = Join-Path $targetLandscape $relativePath
    $destinationExists = Test-Path -LiteralPath $destination -PathType Leaf
    if ($source.Extension -eq '.wmodel') {
        Assert-WModel -Path $source.FullName
        if (-not $destinationExists) {
            throw "Installed Landscape WModel is missing: $destination"
        }
        Assert-WModel -Path $destination
    }
    $sourceHash = Get-FileSha256 -Path $source.FullName
    $destinationHash = if ($destinationExists) { Get-FileSha256 -Path $destination } else { $null }
    [pscustomobject]@{
        RelativePath = $relativePath
        Source = $source.FullName
        Destination = $destination
        SourceSha256 = $sourceHash
        DestinationSha256 = $destinationHash
        DestinationExisted = $destinationExists
        Changed = $sourceHash -ne $destinationHash
    }
}

$changed = @($changes | Where-Object Changed)
$summary = [pscustomobject]@{
    Mode = $Mode
    CandidateRoot = $candidate
    TargetLandscape = $targetLandscape
    ValidatedComponents = $report.componentCount
    ValidatedWModels = $report.wmodelCount
    RuntimeFilesChecked = $changes.Count
    FilesDifferentFromInstalled = $changed.Count
    ChangedWModels = @($changed | Where-Object { $_.RelativePath.EndsWith('.wmodel') }).Count
    ChangedTextures = @($changed | Where-Object { -not $_.RelativePath.EndsWith('.wmodel') }).Count
    PlacementDataChanged = $false
    PublisherRun = $false
}

if ($Mode -eq 'Check') {
    $summary
    return
}

if ([string]::IsNullOrWhiteSpace($BackupRoot)) {
    throw 'Install requires -BackupRoot. The existing runtime files must be backed up before replacement.'
}
if (Test-Path -LiteralPath $BackupRoot) {
    throw "Backup destination already exists: $BackupRoot"
}

New-Item -ItemType Directory -Path $BackupRoot -Force | Out-Null
try {
    foreach ($change in $changed) {
        $backup = Join-Path $BackupRoot $change.RelativePath
        New-Item -ItemType Directory -Path (Split-Path -Parent $backup) -Force | Out-Null
        if ($change.DestinationExisted) {
            Copy-Item -LiteralPath $change.Destination -Destination $backup -Force
        }
    }

    foreach ($change in $changed) {
        Replace-FileAtomically -Source $change.Source -Destination $change.Destination
        if ((Get-FileSha256 -Path $change.Destination) -ne $change.SourceSha256) {
            throw "Post-install hash mismatch: $($change.RelativePath)"
        }
    }

    $summary | Add-Member -NotePropertyName BackupRoot -NotePropertyValue ((Resolve-Path -LiteralPath $BackupRoot).Path)
    $summary
}
catch {
    foreach ($change in $changed) {
        $backup = Join-Path $BackupRoot $change.RelativePath
        if ($change.DestinationExisted -and (Test-Path -LiteralPath $backup -PathType Leaf)) {
            Replace-FileAtomically -Source $backup -Destination $change.Destination
        }
        elseif (-not $change.DestinationExisted -and (Test-Path -LiteralPath $change.Destination -PathType Leaf)) {
            Remove-Item -LiteralPath $change.Destination -Force
        }
    }
    throw
}
