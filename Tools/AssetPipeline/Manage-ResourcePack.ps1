[CmdletBinding()]
param(
    [ValidateSet('Snapshot', 'Verify', 'Publish', 'Hydrate')]
    [string]$Mode = 'Verify',

    [string]$ResourceRoot = 'Client/Bin/Resources',
    [string]$LockFile = 'Data/AssetPacks.lock.json',
    [string]$ManifestDirectory = 'Data/AssetManifests',
    [string]$PackId = 'lostark-resources',
    [string]$Version = '2026.08.03.2',
    [string]$PackRoot = $env:LOSTARK_PACK_ROOT,

    [switch]$FailureAfterManifest
)

$ErrorActionPreference = 'Stop'
$AllowedTopLevel = @('Character', 'Deploy', 'Effect', 'Fonts', 'Map', 'UI')
$ForbiddenTopLevel = @('LostArk', 'Models', 'Textures', 'SourceData', 'Sound')
$ForbiddenExtensions = @('.exe', '.dll', '.pdb', '.cfg')

function Resolve-ProjectPath {
    param([string]$Value)

    if ([IO.Path]::IsPathRooted($Value)) {
        return [IO.Path]::GetFullPath($Value)
    }
    return [IO.Path]::GetFullPath((Join-Path (Get-Location) $Value))
}

function Assert-ChildPath {
    param(
        [string]$Root,
        [string]$Candidate
    )

    $resolvedRoot = [IO.Path]::GetFullPath($Root).TrimEnd('\')
    $resolvedCandidate = [IO.Path]::GetFullPath($Candidate)
    if (-not $resolvedCandidate.StartsWith(
            $resolvedRoot + '\',
            [StringComparison]::OrdinalIgnoreCase)) {
        throw "Path escaped root '$resolvedRoot': $resolvedCandidate"
    }
    return $resolvedCandidate
}

function Get-RelativeResourcePath {
    param(
        [string]$Root,
        [string]$FullName
    )

    $rootUri = [Uri]([IO.Path]::GetFullPath($Root).TrimEnd('\') + '\')
    $fileUri = [Uri][IO.Path]::GetFullPath($FullName)
    return [Uri]::UnescapeDataString(
        $rootUri.MakeRelativeUri($fileUri).ToString()).Replace('\', '/')
}

function Assert-ResourceContract {
    param([string]$Root)

    $resolvedRoot = Resolve-ProjectPath $Root
    if (-not [IO.Directory]::Exists($resolvedRoot)) {
        throw "Resource root does not exist: $resolvedRoot"
    }

    $actualTopLevel = @(
        Get-ChildItem -LiteralPath $resolvedRoot -Force |
            ForEach-Object Name |
            Sort-Object)
    $expectedTopLevel = @($AllowedTopLevel | Sort-Object)
    if (($actualTopLevel -join "`n") -ne ($expectedTopLevel -join "`n")) {
        throw "Resource root mismatch. Expected [$($expectedTopLevel -join ', ')], actual [$($actualTopLevel -join ', ')]"
    }

    foreach ($forbidden in $ForbiddenTopLevel) {
        if (Test-Path -LiteralPath (Join-Path $resolvedRoot $forbidden)) {
            throw "Forbidden top-level resource exists: $forbidden"
        }
    }

    $forbiddenFiles = @(
        Get-ChildItem -LiteralPath $resolvedRoot -Recurse -File -Force |
            Where-Object { $ForbiddenExtensions -contains $_.Extension.ToLowerInvariant() })
    if ($forbiddenFiles.Count -gt 0) {
        throw "Forbidden runtime file: $($forbiddenFiles[0].FullName)"
    }
    return $resolvedRoot
}

function Get-ContentHash {
    param([object[]]$Entries)

    $builder = [Text.StringBuilder]::new()
    foreach ($entry in $Entries) {
        [void]$builder.Append($entry.path)
        [void]$builder.Append("`0")
        [void]$builder.Append([string]$entry.size)
        [void]$builder.Append("`0")
        [void]$builder.Append($entry.sha256)
        [void]$builder.Append("`n")
    }
    $bytes = [Text.Encoding]::UTF8.GetBytes($builder.ToString())
    $sha = [Security.Cryptography.SHA256]::Create()
    try {
        return ([BitConverter]::ToString($sha.ComputeHash($bytes))).Replace('-', '').ToLowerInvariant()
    }
    finally {
        $sha.Dispose()
    }
}

function New-ResourceManifest {
    param(
        [string]$Root,
        [string]$Id,
        [string]$PackVersion
    )

    $entries = [Collections.Generic.List[object]]::new()
    $files = @(Get-ChildItem -LiteralPath $Root -Recurse -File -Force |
        Sort-Object FullName)
    $index = 0
    foreach ($file in $files) {
        $index++
        if (0 -eq ($index % 500)) {
            Write-Progress -Activity 'Hashing resource pack' -Status "$index / $($files.Count)" -PercentComplete (($index * 100) / $files.Count)
        }
        $entries.Add([ordered]@{
            path = Get-RelativeResourcePath -Root $Root -FullName $file.FullName
            size = [long]$file.Length
            sha256 = (Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
        })
    }
    Write-Progress -Activity 'Hashing resource pack' -Completed

    return [ordered]@{
        schema = 'lostark.asset-pack-manifest'
        formatVersion = 1
        packId = $Id
        version = $PackVersion
        generatedUtc = [DateTime]::UtcNow.ToString('o')
        allowedTopLevel = $AllowedTopLevel
        contentSha256 = Get-ContentHash -Entries $entries
        fileCount = $entries.Count
        totalBytes = [long](($entries | ForEach-Object { $_.size } |
            Measure-Object -Sum).Sum)
        files = $entries
    }
}

function Write-JsonAtomic {
    param(
        [object]$Value,
        [string]$Path,
        [int]$Depth = 12
    )

    $resolvedPath = Resolve-ProjectPath $Path
    [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($resolvedPath)) | Out-Null
    $temporaryPath = $resolvedPath + '.tmp'
    $backupPath = $resolvedPath + '.replace-backup'
    $wasReplaced = $false
    try {
        $json = $Value | ConvertTo-Json -Depth $Depth
        [IO.File]::WriteAllText(
            $temporaryPath,
            $json + [Environment]::NewLine,
            [Text.UTF8Encoding]::new($false))
        if ([IO.File]::Exists($backupPath)) {
            throw "Atomic JSON backup already exists: $backupPath"
        }
        [IO.File]::Replace($temporaryPath, $resolvedPath, $backupPath, $true)
        $wasReplaced = $true
    }
    finally {
        if ([IO.File]::Exists($temporaryPath)) {
            [IO.File]::Delete($temporaryPath)
        }
        if ($wasReplaced -and [IO.File]::Exists($backupPath)) {
            [IO.File]::Delete($backupPath)
        }
    }
}

function Write-NewJsonAtomic {
    param(
        [object]$Value,
        [string]$Path,
        [int]$Depth = 12
    )

    $resolvedPath = Resolve-ProjectPath $Path
    [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($resolvedPath)) | Out-Null
    $temporaryPath = $resolvedPath + '.tmp'
    try {
        $json = $Value | ConvertTo-Json -Depth $Depth
        [IO.File]::WriteAllText(
            $temporaryPath,
            $json + [Environment]::NewLine,
            [Text.UTF8Encoding]::new($false))
        Move-Item -LiteralPath $temporaryPath -Destination $resolvedPath
    }
    finally {
        if ([IO.File]::Exists($temporaryPath)) {
            [IO.File]::Delete($temporaryPath)
        }
    }
}

function Get-Lock {
    param([string]$Path)

    $resolvedPath = Resolve-ProjectPath $Path
    if (-not [IO.File]::Exists($resolvedPath)) {
        throw "Asset pack lock does not exist: $resolvedPath"
    }
    $lock = Get-Content -LiteralPath $resolvedPath -Raw | ConvertFrom-Json
    if ($lock.schema -ne 'lostark.asset-pack-lock' -or $lock.formatVersion -ne 1) {
        throw 'Asset pack lock schema mismatch.'
    }
    return $lock
}

function Get-LockedPack {
    param(
        [object]$Lock,
        [string]$Id
    )

    $matches = @($Lock.packs | Where-Object packId -eq $Id)
    if ($matches.Count -ne 1) {
        throw "Expected one locked pack '$Id', found $($matches.Count)."
    }
    return $matches[0]
}

function Test-Payload {
    param(
        [string]$Root,
        [object]$Manifest
    )

    $actualFiles = @(
        Get-ChildItem -LiteralPath $Root -Recurse -File -Force |
            ForEach-Object { Get-RelativeResourcePath -Root $Root -FullName $_.FullName } |
            Sort-Object)
    $expectedFiles = @($Manifest.files | ForEach-Object path | Sort-Object)
    if (($actualFiles -join "`n") -ne ($expectedFiles -join "`n")) {
        throw 'Resource file set differs from the locked manifest.'
    }

    $index = 0
    foreach ($entry in $Manifest.files) {
        $index++
        $filePath = Assert-ChildPath -Root $Root -Candidate (Join-Path $Root $entry.path)
        $item = Get-Item -LiteralPath $filePath
        if ([long]$item.Length -ne [long]$entry.size) {
            throw "Resource size mismatch: $($entry.path)"
        }
        $actualHash = (Get-FileHash -LiteralPath $filePath -Algorithm SHA256).Hash.ToLowerInvariant()
        if ($actualHash -ne $entry.sha256) {
            throw "Resource hash mismatch: $($entry.path)"
        }
    }
}

$resolvedResourceRoot = Resolve-ProjectPath $ResourceRoot
$resolvedLockFile = Resolve-ProjectPath $LockFile
$resolvedManifestDirectory = Resolve-ProjectPath $ManifestDirectory

switch ($Mode) {
    'Snapshot' {
        $resolvedResourceRoot = Assert-ResourceContract $resolvedResourceRoot
        $manifest = New-ResourceManifest -Root $resolvedResourceRoot -Id $PackId -PackVersion $Version
        [IO.Directory]::CreateDirectory($resolvedManifestDirectory) | Out-Null
        $manifestName = "$PackId-$Version.manifest.json"
        $manifestPath = Join-Path $resolvedManifestDirectory $manifestName
        if ([IO.File]::Exists($manifestPath)) {
            throw "Manifest version already exists and is immutable: $manifestPath"
        }
        $lock = Get-Lock $resolvedLockFile
        $manifestCreated = $false
        $lockCommitted = $false
        try {
            Write-NewJsonAtomic -Value $manifest -Path $manifestPath -Depth 10
            $manifestCreated = $true
            if ($FailureAfterManifest) {
                throw 'Injected snapshot failure after manifest creation.'
            }
            $manifestHash = (Get-FileHash -LiteralPath $manifestPath -Algorithm SHA256).Hash.ToLowerInvariant()
            $otherPacks = @($lock.packs | Where-Object packId -ne $PackId)
            $lock.packs = @($otherPacks) + @([ordered]@{
                packId = $PackId
                version = $Version
                manifest = "Data/AssetManifests/$manifestName"
                manifestSha256 = $manifestHash
                contentSha256 = $manifest.contentSha256
                fileCount = $manifest.fileCount
                totalBytes = $manifest.totalBytes
                dependencies = @()
                target = 'Client/Bin/Resources'
            })
            Write-JsonAtomic -Value $lock -Path $resolvedLockFile -Depth 10
            $lockCommitted = $true
        }
        catch {
            $snapshotFailure = $_
            if ($manifestCreated -and -not $lockCommitted -and
                [IO.File]::Exists($manifestPath)) {
                [IO.File]::Delete($manifestPath)
            }
            throw $snapshotFailure
        }
        Write-Output "Snapshot locked: $PackId@$Version, $($manifest.fileCount) files, $($manifest.totalBytes) bytes"
    }

    'Verify' {
        $resolvedResourceRoot = Assert-ResourceContract $resolvedResourceRoot
        $lock = Get-Lock $resolvedLockFile
        $lockedPack = Get-LockedPack -Lock $lock -Id $PackId
        $manifestPath = Resolve-ProjectPath $lockedPack.manifest
        $manifestHash = (Get-FileHash -LiteralPath $manifestPath -Algorithm SHA256).Hash.ToLowerInvariant()
        if ($manifestHash -ne $lockedPack.manifestSha256) {
            throw 'Manifest hash differs from AssetPacks.lock.json.'
        }
        $manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
        if ($manifest.packId -ne $lockedPack.packId -or $manifest.version -ne $lockedPack.version -or
            $manifest.contentSha256 -ne $lockedPack.contentSha256) {
            throw 'Manifest identity differs from the lock.'
        }
        Test-Payload -Root $resolvedResourceRoot -Manifest $manifest
        Write-Output "Resource pack verified: $($lockedPack.packId)@$($lockedPack.version)"
    }

    'Publish' {
        if ([string]::IsNullOrWhiteSpace($PackRoot)) {
            throw 'PackRoot or LOSTARK_PACK_ROOT is required for Publish.'
        }
        & $PSCommandPath -Mode Verify -ResourceRoot $resolvedResourceRoot -LockFile $resolvedLockFile -ManifestDirectory $resolvedManifestDirectory -PackId $PackId
        $lock = Get-Lock $resolvedLockFile
        $lockedPack = Get-LockedPack -Lock $lock -Id $PackId
        $resolvedPackRoot = Resolve-ProjectPath $PackRoot
        [IO.Directory]::CreateDirectory($resolvedPackRoot) | Out-Null
        $leasePath = Join-Path $resolvedPackRoot "$PackId.publisher.lock"
        $lease = [IO.File]::Open($leasePath, [IO.FileMode]::CreateNew, [IO.FileAccess]::Write, [IO.FileShare]::None)
		$stagingPath = $null
        try {
            $finalPath = Join-Path $resolvedPackRoot "$PackId\$($lockedPack.version)"
            if (Test-Path -LiteralPath $finalPath) {
                throw "Pack version already exists and cannot be overwritten: $finalPath"
            }
            # Keep the staging prefix short. Curated map asset IDs can already be
            # close to MAX_PATH and the published destination is much shorter
            # than a packId-version-GUID staging name.
            $stagingId = [Guid]::NewGuid().ToString('N').Substring(0, 8)
            $stagingPath = Join-Path $resolvedPackRoot ".staging\$stagingId"
            $payloadPath = Join-Path $stagingPath 'payload'
            [IO.Directory]::CreateDirectory($payloadPath) | Out-Null
            foreach ($domain in $AllowedTopLevel) {
                Copy-Item -LiteralPath (Join-Path $resolvedResourceRoot $domain) -Destination (Join-Path $payloadPath $domain) -Recurse
            }
            Copy-Item -LiteralPath (Resolve-ProjectPath $lockedPack.manifest) -Destination (Join-Path $stagingPath 'manifest.json')
            Test-Payload -Root $payloadPath -Manifest (Get-Content -LiteralPath (Join-Path $stagingPath 'manifest.json') -Raw | ConvertFrom-Json)
            [IO.File]::WriteAllText((Join-Path $stagingPath 'READY'), $lockedPack.manifestSha256 + [Environment]::NewLine, [Text.UTF8Encoding]::new($false))
            [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($finalPath)) | Out-Null
            Move-Item -LiteralPath $stagingPath -Destination $finalPath
            Write-Output "Published immutable pack: $finalPath"
        }
        finally {
			if ($null -ne $stagingPath -and
				[IO.Directory]::Exists($stagingPath)) {
				Remove-Item -LiteralPath $stagingPath -Recurse -Force
			}
            $lease.Dispose()
            [IO.File]::Delete($leasePath)
        }
    }

    'Hydrate' {
        if ([string]::IsNullOrWhiteSpace($PackRoot)) {
            throw 'PackRoot or LOSTARK_PACK_ROOT is required for Hydrate.'
        }
        $lock = Get-Lock $resolvedLockFile
        $lockedPack = Get-LockedPack -Lock $lock -Id $PackId
        $sourcePath = Join-Path (Resolve-ProjectPath $PackRoot) "$PackId\$($lockedPack.version)"
        $readyPath = Join-Path $sourcePath 'READY'
        if (-not [IO.File]::Exists($readyPath) -or
            ([IO.File]::ReadAllText($readyPath).Trim() -ne $lockedPack.manifestSha256)) {
            throw 'Pack READY marker is missing or does not match the lock.'
        }
        $sourceManifest = Join-Path $sourcePath 'manifest.json'
        if ((Get-FileHash -LiteralPath $sourceManifest -Algorithm SHA256).Hash.ToLowerInvariant() -ne $lockedPack.manifestSha256) {
            throw 'Published manifest hash mismatch.'
        }

        $parent = [IO.Path]::GetDirectoryName($resolvedResourceRoot)
        $stagingPath = Join-Path $parent "Resources.staging.$([Guid]::NewGuid().ToString('N'))"
        try {
			Copy-Item -LiteralPath (Join-Path $sourcePath 'payload') -Destination $stagingPath -Recurse
			$manifest = Get-Content -LiteralPath $sourceManifest -Raw | ConvertFrom-Json
			Assert-ResourceContract $stagingPath | Out-Null
			Test-Payload -Root $stagingPath -Manifest $manifest

			$backupPath = Join-Path $parent ("Resources.previous." + [DateTime]::UtcNow.ToString('yyyyMMddHHmmss'))
			$hadPreviousResources = [IO.Directory]::Exists($resolvedResourceRoot)
			if ($hadPreviousResources -and [IO.Directory]::Exists($backupPath)) {
				throw "Hydrate backup path already exists: $backupPath"
			}

			try {
				if ($hadPreviousResources) {
					Move-Item -LiteralPath $resolvedResourceRoot -Destination $backupPath
				}
				Move-Item -LiteralPath $stagingPath -Destination $resolvedResourceRoot
			}
			catch {
				$hydrateFailure = $_
				if ($hadPreviousResources -and
					-not [IO.Directory]::Exists($resolvedResourceRoot) -and
					[IO.Directory]::Exists($backupPath)) {
					Move-Item -LiteralPath $backupPath -Destination $resolvedResourceRoot
				}
				throw $hydrateFailure
            }
			Write-Output "Hydrated $($lockedPack.packId)@$($lockedPack.version). Previous resources: $backupPath"
        }
		finally {
			if ([IO.Directory]::Exists($stagingPath)) {
				Remove-Item -LiteralPath $stagingPath -Recurse -Force
			}
		}
    }
}
