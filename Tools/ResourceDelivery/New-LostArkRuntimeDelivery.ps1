[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [string]$RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path,
    [Parameter(Mandatory = $true)]
    [string]$OutputZip,
    [switch]$IncludePdb
)

$ErrorActionPreference = 'Stop'

function Get-Sha256([string]$Path) {
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant()
}

$repositoryPath = (Resolve-Path -LiteralPath $RepositoryRoot).Path
if (-not (Test-Path -LiteralPath (Join-Path $repositoryPath 'Framework.sln') -PathType Leaf)) {
    throw "RepositoryRoot is not a LostArk physical folder: $repositoryPath"
}

$requiredFiles = @(
    "Client/Bin/$Configuration/Client.exe",
    "Server/Bin/$Configuration/Server.exe"
)
foreach ($relativePath in $requiredFiles) {
    if (-not (Test-Path -LiteralPath (Join-Path $repositoryPath $relativePath) -PathType Leaf)) {
        throw "Required runtime output is missing: $relativePath"
    }
}

$candidateRoots = @(
    "Client/Bin/$Configuration",
    'Client/Bin/DataFiles',
    "Server/Bin/$Configuration",
    'Server/Bin/DataFiles'
)
$filesByRelativePath = [ordered]@{}
$repositoryPrefix = $repositoryPath.TrimEnd('\') + '\'
foreach ($relativeRoot in $candidateRoots) {
    $physicalRoot = Join-Path $repositoryPath $relativeRoot
    if (-not (Test-Path -LiteralPath $physicalRoot -PathType Container)) {
        continue
    }
    Get-ChildItem -LiteralPath $physicalRoot -File -Recurse | ForEach-Object {
        if (-not $_.FullName.StartsWith($repositoryPrefix, [StringComparison]::OrdinalIgnoreCase)) {
            throw "Runtime output escaped RepositoryRoot: $($_.FullName)"
        }
        $relativePath = $_.FullName.Substring($repositoryPrefix.Length).Replace('\', '/')
        if ($relativePath.StartsWith('Client/Bin/Resources/', [StringComparison]::OrdinalIgnoreCase)) {
            throw "Resource packs must never enter a runtime delivery ZIP: $relativePath"
        }
        if (-not $IncludePdb -and $_.Extension -ieq '.pdb') {
            return
        }
        if ($_.Extension -in @('.log', '.tlog', '.lastbuildstate')) {
            return
        }
        $filesByRelativePath[$relativePath] = $_.FullName
    }
}

$stagingRoot = Join-Path ([IO.Path]::GetTempPath()) ("lostark-runtime-delivery-" + [guid]::NewGuid().ToString('N'))
try {
    New-Item -ItemType Directory -Path $stagingRoot | Out-Null
    $manifestFiles = @()
    foreach ($relativePath in ($filesByRelativePath.Keys | Sort-Object)) {
        $sourcePath = $filesByRelativePath[$relativePath]
        $stagedPath = Join-Path $stagingRoot ($relativePath.Replace('/', [IO.Path]::DirectorySeparatorChar))
        New-Item -ItemType Directory -Path (Split-Path -Parent $stagedPath) -Force | Out-Null
        Copy-Item -LiteralPath $sourcePath -Destination $stagedPath
        $item = Get-Item -LiteralPath $stagedPath
        $manifestFiles += [ordered]@{
            path = $relativePath
            bytes = [long]$item.Length
            sha256 = Get-Sha256 $stagedPath
        }
    }

    $manifest = [ordered]@{
        schema = 'lostark.runtime-delivery'
        formatVersion = 1
        configuration = $Configuration
        resourcePolicy = 'drive-owned-not-in-zip'
        files = $manifestFiles
    }
    $manifestPath = Join-Path $stagingRoot 'runtime-delivery.json'
    $manifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $manifestPath -Encoding UTF8

    $outputFullPath = [IO.Path]::GetFullPath($OutputZip)
    $outputParent = Split-Path -Parent $outputFullPath
    if ($outputParent) {
        New-Item -ItemType Directory -Path $outputParent -Force | Out-Null
    }
    if (Test-Path -LiteralPath $outputFullPath) {
        Remove-Item -LiteralPath $outputFullPath -Force
    }
    Compress-Archive -Path (Join-Path $stagingRoot '*') -DestinationPath $outputFullPath -CompressionLevel Optimal
    Write-Output "Runtime delivery created: $outputFullPath"
    Write-Output "Configuration=$Configuration Files=$($manifestFiles.Count) ResourcesIncluded=0"
}
finally {
    if (Test-Path -LiteralPath $stagingRoot) {
        Remove-Item -LiteralPath $stagingRoot -Recurse -Force
    }
}
