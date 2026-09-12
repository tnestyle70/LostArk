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

$assimpRuntimeName = if ($Configuration -eq 'Debug') { 'assimp-vc143-mtd.dll' } else { 'assimp-vc143-mt.dll' }
# Keep this list aligned with Client.vcxproj's ClientRuntimeDependencies.
$moduleRoots = @(
    @{ Path = "Client/Bin/$Configuration"; Modules = @(
        'Client.exe', 'Engine.dll', $assimpRuntimeName, 'fmod.dll',
        'PhysX_64.dll', 'PhysXCommon_64.dll', 'PhysXFoundation_64.dll') },
    @{ Path = "Server/Bin/$Configuration"; Modules = @('Server.exe') }
)
$candidates = [Collections.Generic.List[IO.FileInfo]]::new()
foreach ($moduleRoot in $moduleRoots) {
    $physicalRoot = Join-Path $repositoryPath $moduleRoot.Path
    $moduleFiles = @(Get-ChildItem -LiteralPath $physicalRoot -File |
        Where-Object { $_.Name -in $moduleRoot.Modules })
    $symbolNames = @($moduleFiles | ForEach-Object { $_.BaseName + '.pdb' })
    foreach ($file in (Get-ChildItem -LiteralPath $physicalRoot -File)) {
        if ($file.Name -in $moduleRoot.Modules -or $file.Extension -ieq '.cso' -or
            ($IncludePdb -and $file.Name -in $symbolNames)) {
            $candidates.Add($file)
        }
    }
}

# Publisher transaction directories/files are not committed runtime data.
# Prune directories before walking them, including nested .staging directories.
foreach ($relativeRoot in @('Client/Bin/DataFiles', 'Server/Bin/DataFiles')) {
    $physicalRoot = Join-Path $repositoryPath $relativeRoot
    if (-not (Test-Path -LiteralPath $physicalRoot -PathType Container)) {
        continue
    }
    $pendingDirectories = [Collections.Generic.Stack[string]]::new()
    $pendingDirectories.Push($physicalRoot)
    while ($pendingDirectories.Count -gt 0) {
        foreach ($item in (Get-ChildItem -LiteralPath $pendingDirectories.Pop())) {
            if ($item.Name -match '(^|\.)(staging|rollback)(\.|$)' -or
                $item.Name -ieq '.composition-publish.journal.json') {
                continue
            }
            if ($item.PSIsContainer) {
                if (($item.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
                    throw "Runtime data directory must not redirect outside its tree: $($item.FullName)"
                }
                $pendingDirectories.Push($item.FullName)
            }
            elseif ($item.Extension -notin @('.log', '.tlog', '.lastbuildstate', '.pdb')) {
                $candidates.Add($item)
            }
        }
    }
}
$filesByRelativePath = [ordered]@{}
$repositoryPrefix = $repositoryPath.TrimEnd('\') + '\'
foreach ($file in $candidates) {
    if (-not $file.FullName.StartsWith($repositoryPrefix, [StringComparison]::OrdinalIgnoreCase)) {
        throw "Runtime output escaped RepositoryRoot: $($file.FullName)"
    }
    $relativePath = $file.FullName.Substring($repositoryPrefix.Length).Replace('\', '/')
    if ($relativePath.StartsWith('Client/Bin/Resources/', [StringComparison]::OrdinalIgnoreCase)) {
        throw "Resource packs must never enter a runtime delivery ZIP: $relativePath"
    }
    $filesByRelativePath[$relativePath] = $file.FullName
}

$temporaryParent = (Resolve-Path -LiteralPath ([IO.Path]::GetTempPath())).Path.TrimEnd('\')
$stagingRoot = [IO.Path]::GetFullPath((Join-Path $temporaryParent ("lostark-runtime-delivery-" + [guid]::NewGuid().ToString('N'))))
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
        $cleanupPath = (Resolve-Path -LiteralPath $stagingRoot).Path
        if (-not $cleanupPath.Equals($stagingRoot, [StringComparison]::OrdinalIgnoreCase) -or
            -not ([IO.Path]::GetDirectoryName($cleanupPath)).Equals($temporaryParent, [StringComparison]::OrdinalIgnoreCase) -or
            ((Get-Item -LiteralPath $cleanupPath).Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
            throw "Refusing to remove an unexpected runtime delivery staging directory: $cleanupPath"
        }
        Remove-Item -LiteralPath $cleanupPath -Recurse -Force
    }
}
