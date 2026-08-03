[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$AssetId,

    [Parameter()]
    [string[]]$TexturePath = @(),

    [Parameter()]
    [string]$MeshPath = '',

    [Parameter()]
    [string]$OutputRoot = 'Data/Effects/Cooked',

    [Parameter()]
    [string]$Category = '',

    [Parameter()]
    [switch]$Force
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = 'Stop'

function Resolve-RepositoryPath {
    param(
        [Parameter(Mandatory = $true)]
        [string]$PathValue,

        [Parameter(Mandatory = $true)]
        [string]$RepositoryRoot,

        [Parameter()]
        [switch]$RequireFile
    )

    if ([string]::IsNullOrWhiteSpace($PathValue)) {
        throw 'A required path value is empty.'
    }

    $expandedPath = [Environment]::ExpandEnvironmentVariables($PathValue)
    if ([System.IO.Path]::IsPathRooted($expandedPath)) {
        $candidatePath = [System.IO.Path]::GetFullPath($expandedPath)
    }
    else {
        $candidatePath = [System.IO.Path]::GetFullPath(
            (Join-Path -Path $RepositoryRoot -ChildPath $expandedPath))
    }

    if ($RequireFile) {
        if (-not (Test-Path -LiteralPath $candidatePath -PathType Leaf)) {
            throw "File does not exist: $candidatePath"
        }

        return (Resolve-Path -LiteralPath $candidatePath).ProviderPath
    }

    return $candidatePath
}

function Get-Sha256 {
    param(
        [Parameter(Mandatory = $true)]
        [string]$FilePath
    )

    return (Get-FileHash -LiteralPath $FilePath -Algorithm SHA256).Hash.ToLowerInvariant()
}

if ($AssetId -notmatch '^[A-Za-z0-9][A-Za-z0-9._-]{0,127}$') {
    throw ('AssetId must start with an ASCII letter or digit and contain only ' +
        'ASCII letters, digits, dot, underscore, or hyphen (maximum 128 characters).')
}

if ($AssetId -match '^(?i:CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])(?:\.|$)') {
    throw "AssetId is a reserved Windows file name: $AssetId"
}

$repositoryRoot = (Resolve-Path -LiteralPath (
    Join-Path -Path $PSScriptRoot -ChildPath '..\..')).ProviderPath

if (-not [string]::IsNullOrWhiteSpace($MeshPath)) {
    throw ('MeshPath is not supported by the effect texture intake. ' +
        'Convert mesh sources with Tools/ModelAssetConverter to .wmodel, ' +
        'then admit the complete asset through the runtime resource pack.')
}

$resolvedOutputRoot = Resolve-RepositoryPath `
    -PathValue $OutputRoot `
    -RepositoryRoot $repositoryRoot

# Category groups staged assets by class folder, e.g. Glaivier.
# ASCII names only: the converter breaks on non-ASCII path segments.
if (-not [string]::IsNullOrWhiteSpace($Category)) {
    if ($Category -notmatch '^[A-Za-z0-9_-]+$') {
        throw "Category must be a single ASCII folder name: $Category"
    }
    $resolvedOutputRoot = Join-Path -Path $resolvedOutputRoot -ChildPath $Category
}

if (Test-Path -LiteralPath $resolvedOutputRoot -PathType Leaf) {
    throw "OutputRoot points to a file: $resolvedOutputRoot"
}

$resolvedTexturePaths = @()
$textureNames = @{}
if ($null -ne $TexturePath) {
    foreach ($textureArgument in $TexturePath) {
        $resolvedTexturePath = Resolve-RepositoryPath `
            -PathValue $textureArgument `
            -RepositoryRoot $repositoryRoot `
            -RequireFile

        $textureName = [System.IO.Path]::GetFileName($resolvedTexturePath)
        $textureNameKey = $textureName.ToLowerInvariant()
        if ($textureNames.ContainsKey($textureNameKey)) {
            throw "Texture file-name collision in the selected inputs: $textureName"
        }

        $textureNames[$textureNameKey] = $true
        $resolvedTexturePaths += $resolvedTexturePath
    }
}

if ($resolvedTexturePaths.Count -eq 0) {
    throw 'Select at least one TexturePath to cook.'
}

$targetAssetDirectory = Join-Path -Path $resolvedOutputRoot -ChildPath $AssetId
if (Test-Path -LiteralPath $targetAssetDirectory) {
    if (-not (Test-Path -LiteralPath $targetAssetDirectory -PathType Container)) {
        throw "The target asset path exists but is not a directory: $targetAssetDirectory"
    }

    if (-not $Force) {
        throw ("The cooked asset already exists: $targetAssetDirectory`n" +
            'Use -Force only when replacing that complete asset is intended.')
    }
}

[System.IO.Directory]::CreateDirectory($resolvedOutputRoot) | Out-Null

$operationId = [Guid]::NewGuid().ToString('N')
$stagingDirectory = Join-Path -Path $resolvedOutputRoot `
    -ChildPath ".cook-staging-$AssetId-$operationId"
$backupDirectory = Join-Path -Path $resolvedOutputRoot `
    -ChildPath ".cook-backup-$AssetId-$operationId"
$committed = $false

try {
    [System.IO.Directory]::CreateDirectory($stagingDirectory) | Out-Null

    $sourceRecords = @()
    $outputRecords = @()
    if ($resolvedTexturePaths.Count -gt 0) {
        $textureOutputDirectory = Join-Path -Path $stagingDirectory -ChildPath 'Textures'
        [System.IO.Directory]::CreateDirectory($textureOutputDirectory) | Out-Null

        foreach ($resolvedTexturePath in $resolvedTexturePaths) {
            $textureName = [System.IO.Path]::GetFileName($resolvedTexturePath)
            $textureOutputPath = Join-Path -Path $textureOutputDirectory `
                -ChildPath $textureName
            Copy-Item -LiteralPath $resolvedTexturePath -Destination $textureOutputPath

            $textureSourceHash = Get-Sha256 -FilePath $resolvedTexturePath
            $sourceRecords += [ordered]@{
                kind      = 'texture'
                path      = $resolvedTexturePath
                sha256    = $textureSourceHash
                extension = [System.IO.Path]::GetExtension(
                    $resolvedTexturePath).ToLowerInvariant()
            }
            $outputRecords += [ordered]@{
                kind         = 'texture'
                relativePath = ('Textures/' + $textureName)
                sha256       = Get-Sha256 -FilePath $textureOutputPath
                source       = $resolvedTexturePath
            }
        }
    }

    $manifest = [ordered]@{
        schemaVersion = 1
        assetId       = $AssetId
        createdAt     = [DateTime]::UtcNow.ToString('o')
        provenance    = [ordered]@{
            tool       = 'Tools/EffectResourceIntake/Cook-SelectedEffectAsset.ps1'
            repository = $repositoryRoot
        }
        source        = @($sourceRecords)
        outputs       = @($outputRecords)
    }

    $manifestPath = Join-Path -Path $stagingDirectory -ChildPath 'manifest.json'
    $manifestJson = $manifest | ConvertTo-Json -Depth 8
    $utf8NoBom = New-Object System.Text.UTF8Encoding -ArgumentList $false
    [System.IO.File]::WriteAllText(
        $manifestPath,
        $manifestJson + [Environment]::NewLine,
        $utf8NoBom)

    $hadPreviousAsset = Test-Path -LiteralPath $targetAssetDirectory -PathType Container
    if ($hadPreviousAsset) {
        Move-Item -LiteralPath $targetAssetDirectory -Destination $backupDirectory
    }

    try {
        Move-Item -LiteralPath $stagingDirectory -Destination $targetAssetDirectory
        $committed = $true
    }
    catch {
        $commitError = $_
        if ($hadPreviousAsset -and
            (Test-Path -LiteralPath $backupDirectory -PathType Container) -and
            -not (Test-Path -LiteralPath $targetAssetDirectory)) {
            try {
                Move-Item -LiteralPath $backupDirectory `
                    -Destination $targetAssetDirectory
            }
            catch {
                throw ("Cook commit failed and the previous asset could not be restored. " +
                    "The previous asset is preserved at '$backupDirectory'. " +
                    "Commit error: $($commitError.Exception.Message)")
            }
        }

        throw $commitError
    }

    if (Test-Path -LiteralPath $backupDirectory) {
        try {
            Remove-Item -LiteralPath $backupDirectory -Recurse -Force
        }
        catch {
            Write-Warning ("The new asset was committed, but the old backup could not be " +
                "removed: $backupDirectory")
        }
    }

    Write-Output $targetAssetDirectory
}
finally {
    if (-not $committed -and (Test-Path -LiteralPath $stagingDirectory)) {
        try {
            Remove-Item -LiteralPath $stagingDirectory -Recurse -Force
        }
        catch {
            Write-Warning "Failed to remove staging directory: $stagingDirectory"
        }
    }
}
