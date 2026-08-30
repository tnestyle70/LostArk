[CmdletBinding(SupportsShouldProcess = $true, ConfirmImpact = 'Medium')]
param(
    [Parameter(Mandatory = $true)]
    [string]$PackagePath,
    [Parameter(Mandatory = $true)]
    [string]$RepositoryRoot
)

$ErrorActionPreference = 'Stop'

function Get-Sha256([string]$Path) {
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant()
}

function Test-AllowedRelativePath([string]$RelativePath) {
    if ([string]::IsNullOrWhiteSpace($RelativePath) -or
        [IO.Path]::IsPathRooted($RelativePath) -or
        $RelativePath.Contains('\') -or
        $RelativePath.Split('/') -contains '..') {
        return $false
    }
    if ($RelativePath.StartsWith('Client/Bin/Resources/', [StringComparison]::OrdinalIgnoreCase)) {
        return $false
    }
    $allowedPrefixes = @(
        'Client/Bin/Debug/',
        'Client/Bin/Release/',
        'Client/Bin/DataFiles/',
        'Server/Bin/Debug/',
        'Server/Bin/Release/',
        'Server/Bin/DataFiles/'
    )
    foreach ($prefix in $allowedPrefixes) {
        if ($RelativePath.StartsWith($prefix, [StringComparison]::Ordinal)) {
            return $true
        }
    }
    return $false
}

$packageFullPath = (Resolve-Path -LiteralPath $PackagePath).Path
$repositoryPath = (Resolve-Path -LiteralPath $RepositoryRoot).Path
if (-not (Test-Path -LiteralPath (Join-Path $repositoryPath 'Framework.sln') -PathType Leaf)) {
    throw "RepositoryRoot is not a LostArk physical folder: $repositoryPath"
}

$temporaryRoot = Join-Path ([IO.Path]::GetTempPath()) ("lostark-runtime-install-" + [guid]::NewGuid().ToString('N'))
$extractRoot = Join-Path $temporaryRoot 'extract'
$backupRoot = Join-Path $temporaryRoot 'backup'
$applied = [Collections.Generic.List[object]]::new()
try {
    New-Item -ItemType Directory -Path $extractRoot -Force | Out-Null
    New-Item -ItemType Directory -Path $backupRoot -Force | Out-Null
    Expand-Archive -LiteralPath $packageFullPath -DestinationPath $extractRoot

    $manifestPath = Join-Path $extractRoot 'runtime-delivery.json'
    if (-not (Test-Path -LiteralPath $manifestPath -PathType Leaf)) {
        throw 'runtime-delivery.json is missing from the package'
    }
    $manifest = Get-Content -Raw -LiteralPath $manifestPath | ConvertFrom-Json
    if ($manifest.schema -ne 'lostark.runtime-delivery' -or [int]$manifest.formatVersion -ne 1) {
        throw 'Unsupported runtime delivery manifest'
    }

    $seen = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $verified = @()
    foreach ($entry in @($manifest.files)) {
        $relativePath = [string]$entry.path
        if (-not (Test-AllowedRelativePath $relativePath)) {
            throw "Package contains a forbidden path: $relativePath"
        }
        if (-not $seen.Add($relativePath)) {
            throw "Package contains a duplicate path: $relativePath"
        }
        $sourcePath = Join-Path $extractRoot ($relativePath.Replace('/', [IO.Path]::DirectorySeparatorChar))
        if (-not (Test-Path -LiteralPath $sourcePath -PathType Leaf)) {
            throw "Manifest file is missing from the package: $relativePath"
        }
        $sourceItem = Get-Item -LiteralPath $sourcePath
        if ([long]$sourceItem.Length -ne [long]$entry.bytes) {
            throw "Size mismatch for $relativePath"
        }
        if ((Get-Sha256 $sourcePath) -ne ([string]$entry.sha256).ToLowerInvariant()) {
            throw "SHA256 mismatch for $relativePath"
        }
        $targetPath = [IO.Path]::GetFullPath((Join-Path $repositoryPath $relativePath))
        $expectedPrefix = $repositoryPath.TrimEnd('\') + '\'
        if (-not $targetPath.StartsWith($expectedPrefix, [StringComparison]::OrdinalIgnoreCase)) {
            throw "Resolved package path escapes RepositoryRoot: $relativePath"
        }
        $verified += [pscustomobject]@{
            RelativePath = $relativePath
            SourcePath = $sourcePath
            TargetPath = $targetPath
        }
    }

    if (-not $PSCmdlet.ShouldProcess($repositoryPath, "Install $($verified.Count) verified runtime files")) {
        Write-Output "Runtime delivery verified only: Files=$($verified.Count) ResourcesTouched=0"
        return
    }

    foreach ($file in $verified) {
        $targetParent = Split-Path -Parent $file.TargetPath
        New-Item -ItemType Directory -Path $targetParent -Force | Out-Null
        $hadOriginal = Test-Path -LiteralPath $file.TargetPath -PathType Leaf
        $backupPath = $null
        if ($hadOriginal) {
            $backupPath = Join-Path $backupRoot ($file.RelativePath.Replace('/', [IO.Path]::DirectorySeparatorChar))
            New-Item -ItemType Directory -Path (Split-Path -Parent $backupPath) -Force | Out-Null
            Copy-Item -LiteralPath $file.TargetPath -Destination $backupPath
        }
        Copy-Item -LiteralPath $file.SourcePath -Destination $file.TargetPath -Force
        $applied.Add([pscustomobject]@{
            TargetPath = $file.TargetPath
            BackupPath = $backupPath
            HadOriginal = $hadOriginal
        })
    }

    Write-Output "Runtime delivery installed: Files=$($verified.Count) ResourcesTouched=0"
    Write-Output 'The team Drive Client/Bin/Resources tree must already exist at the same relative paths.'
}
catch {
    for ($index = $applied.Count - 1; $index -ge 0; --$index) {
        $entry = $applied[$index]
        if ($entry.HadOriginal) {
            Copy-Item -LiteralPath $entry.BackupPath -Destination $entry.TargetPath -Force
        }
        elseif (Test-Path -LiteralPath $entry.TargetPath -PathType Leaf) {
            Remove-Item -LiteralPath $entry.TargetPath -Force
        }
    }
    throw
}
finally {
    if (Test-Path -LiteralPath $temporaryRoot) {
        Remove-Item -LiteralPath $temporaryRoot -Recurse -Force
    }
}
