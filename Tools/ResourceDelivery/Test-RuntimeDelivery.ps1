[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$temporaryParent = (Resolve-Path -LiteralPath ([IO.Path]::GetTempPath())).Path.TrimEnd('\')
$testRoot = [IO.Path]::GetFullPath((Join-Path $temporaryParent ("lostark-runtime-delivery-test-" + [guid]::NewGuid().ToString('N'))))
$sourceRoot = Join-Path $testRoot 'source'
$targetRoot = Join-Path $testRoot 'target'
$zipPath = Join-Path $testRoot 'runtime.zip'

function Assert-PackageFiles([string]$Archive, [string]$InspectionRoot, [string[]]$ExpectedPaths) {
    Expand-Archive -LiteralPath $Archive -DestinationPath $InspectionRoot
    $manifest = Get-Content -Raw -LiteralPath (Join-Path $InspectionRoot 'runtime-delivery.json') | ConvertFrom-Json
    $actualPaths = @($manifest.files | ForEach-Object { [string]$_.path })
    if (@(Compare-Object -ReferenceObject ($ExpectedPaths | Sort-Object) -DifferenceObject ($actualPaths | Sort-Object)).Count -ne 0) {
        throw "Unexpected package manifest files: $($actualPaths -join ', ')"
    }
    $inspectionPrefix = $InspectionRoot.TrimEnd('\') + '\'
    $archivePaths = @(Get-ChildItem -LiteralPath $InspectionRoot -File -Recurse | ForEach-Object {
        $_.FullName.Substring($inspectionPrefix.Length).Replace('\', '/')
    })
    $expectedArchivePaths = @($ExpectedPaths) + @('runtime-delivery.json')
    if (@(Compare-Object -ReferenceObject ($expectedArchivePaths | Sort-Object) -DifferenceObject ($archivePaths | Sort-Object)).Count -ne 0) {
        throw 'Archive files do not match its runtime-only manifest.'
    }
}

try {
    foreach ($root in @($sourceRoot, $targetRoot)) {
        New-Item -ItemType Directory -Path $root -Force | Out-Null
        Set-Content -LiteralPath (Join-Path $root 'Framework.sln') -Value 'fixture' -Encoding UTF8
    }
    $sourceFiles = [ordered]@{
        'Client/Bin/Debug/Client.exe' = 'new-client'
        'Client/Bin/Debug/Engine.dll' = 'new-engine'
        'Client/Bin/Debug/assimp-vc143-mtd.dll' = 'runtime-assimp'
        'Client/Bin/Debug/fmod.dll' = 'runtime-fmod'
        'Client/Bin/Debug/PhysX_64.dll' = 'runtime-physx'
        'Client/Bin/Debug/PhysXCommon_64.dll' = 'runtime-physx-common'
        'Client/Bin/Debug/PhysXFoundation_64.dll' = 'runtime-physx-foundation'
        'Client/Bin/Debug/Shader_Viewport.cso' = 'runtime-shader'
        'Client/Bin/Debug/Client.pdb' = 'client-symbols'
        'Client/Bin/Debug/Engine.pdb' = 'engine-symbols'
        'Client/Bin/Debug/ClientEffectVerify.exe' = 'exclude-verification-exe'
        'Client/Bin/Debug/ClientEffectVerify.pdb' = 'exclude-verification-symbols'
        'Client/Bin/Debug/ClientProbe.dll' = 'exclude-probe-dll'
        'Client/Bin/Debug/ClientProbe.pdb' = 'exclude-probe-symbols'
        'Client/Bin/Debug/Shared.pdb' = 'exclude-static-library-symbols'
        'Client/Bin/Debug/vc143.pdb' = 'exclude-compiler-symbols'
        'Client/Bin/Debug/Client.ilk' = 'exclude-ilk'
        'Client/Bin/Debug/Client.lib' = 'exclude-lib'
        'Client/Bin/Debug/Client.idb' = 'exclude-idb'
        'Client/Bin/Debug/Client.exp' = 'exclude-exp'
        'Client/Bin/Debug/Client.obj' = 'exclude-obj'
        'Client/Bin/Debug/Diagnostics/465.jsonl' = 'exclude-diagnostics'
        'Client/Bin/Debug/Nested/Engine.dll' = 'exclude-nested-module'
        'Client/Bin/Debug/Nested/Shader.cso' = 'exclude-nested-shader'
        'Client/Bin/Release/Client.exe' = 'release-client'
        'Client/Bin/Release/Engine.dll' = 'release-engine'
        'Client/Bin/Release/assimp-vc143-mt.dll' = 'release-assimp'
        'Client/Bin/Release/assimp-vc143-mtd.dll' = 'exclude-debug-assimp'
        'Server/Bin/Release/Server.exe' = 'release-server'
        'Client/Bin/DataFiles/World/test.json' = '{"ok":true}'
        'Client/Bin/DataFiles/Map/Nested/test.mapplacements' = 'nested-runtime-map'
        'Client/Bin/DataFiles/.staging/World/test.json' = 'exclude-staging'
        'Client/Bin/DataFiles/Map/.map-publish.staging.area.transaction/test.json' = 'exclude-map-staging'
        'Client/Bin/DataFiles/Map/test.mapplacements.rollback.transaction' = 'exclude-map-rollback'
        'Client/Bin/DataFiles/Composition/.composition-publish.journal.json' = 'exclude-transaction-journal'
        'Server/Bin/Debug/Server.exe' = 'new-server'
        'Server/Bin/Debug/Server.pdb' = 'server-symbols'
        'Server/Bin/Debug/KoukuPatternServerContract.exe' = 'exclude-server-test'
        'Server/Bin/Debug/KoukuPatternServerContract.pdb' = 'exclude-server-test-symbols'
        'Server/Bin/DataFiles/World/test.bootstrap' = 'world'
        'Server/Bin/DataFiles/World/.staging.transaction/test.bootstrap' = 'exclude-world-staging'
        'Server/Bin/DataFiles/Items/test.bootstrap.staging.transaction' = 'exclude-item-staging'
        'Server/Bin/DataFiles/.balance-runtime-set.staging.transaction/World/test.bootstrap' = 'exclude-balance-staging'
        'Data/Worlds/authoring.json' = 'git-owned-source'
        'Client/Bin/Resources/Effect/local.dds' = 'drive-source'
    }
    foreach ($entry in $sourceFiles.GetEnumerator()) {
        $path = Join-Path $sourceRoot $entry.Key
        New-Item -ItemType Directory -Path (Split-Path -Parent $path) -Force | Out-Null
        Set-Content -LiteralPath $path -Value $entry.Value -Encoding UTF8
    }
    $targetResource = Join-Path $targetRoot 'Client/Bin/Resources/Effect/local.dds'
    New-Item -ItemType Directory -Path (Split-Path -Parent $targetResource) -Force | Out-Null
    Set-Content -LiteralPath $targetResource -Value 'drive-target-must-survive' -Encoding UTF8

    & (Join-Path $PSScriptRoot 'New-LostArkRuntimeDelivery.ps1') `
        -Configuration Debug `
        -RepositoryRoot $sourceRoot `
        -OutputZip $zipPath
    $expectedFiles = @(
        'Client/Bin/Debug/Client.exe', 'Client/Bin/Debug/Engine.dll',
        'Client/Bin/Debug/assimp-vc143-mtd.dll', 'Client/Bin/Debug/fmod.dll',
        'Client/Bin/Debug/PhysX_64.dll', 'Client/Bin/Debug/PhysXCommon_64.dll',
        'Client/Bin/Debug/PhysXFoundation_64.dll', 'Client/Bin/Debug/Shader_Viewport.cso',
        'Client/Bin/DataFiles/World/test.json', 'Client/Bin/DataFiles/Map/Nested/test.mapplacements',
        'Server/Bin/Debug/Server.exe', 'Server/Bin/DataFiles/World/test.bootstrap'
    )
    Assert-PackageFiles $zipPath (Join-Path $testRoot 'inspect') $expectedFiles
    $symbolZipPath = Join-Path $testRoot 'runtime-symbols.zip'
    & (Join-Path $PSScriptRoot 'New-LostArkRuntimeDelivery.ps1') `
        -Configuration Debug -RepositoryRoot $sourceRoot -OutputZip $symbolZipPath -IncludePdb
    $expectedSymbolFiles = $expectedFiles + @(
        'Client/Bin/Debug/Client.pdb', 'Client/Bin/Debug/Engine.pdb', 'Server/Bin/Debug/Server.pdb'
    )
    Assert-PackageFiles $symbolZipPath (Join-Path $testRoot 'inspect-symbols') $expectedSymbolFiles
    $releaseZipPath = Join-Path $testRoot 'runtime-release.zip'
    & (Join-Path $PSScriptRoot 'New-LostArkRuntimeDelivery.ps1') `
        -Configuration Release -RepositoryRoot $sourceRoot -OutputZip $releaseZipPath
    $expectedReleaseFiles = @(
        'Client/Bin/Release/Client.exe', 'Client/Bin/Release/Engine.dll',
        'Client/Bin/Release/assimp-vc143-mt.dll', 'Server/Bin/Release/Server.exe',
        'Client/Bin/DataFiles/World/test.json', 'Client/Bin/DataFiles/Map/Nested/test.mapplacements',
        'Server/Bin/DataFiles/World/test.bootstrap'
    )
    Assert-PackageFiles $releaseZipPath (Join-Path $testRoot 'inspect-release') $expectedReleaseFiles
    & (Join-Path $PSScriptRoot 'Install-LostArkRuntimeDelivery.ps1') `
        -PackagePath $zipPath `
        -RepositoryRoot $targetRoot `
        -Confirm:$false

    $expectedClient = Get-Content -Raw -LiteralPath (Join-Path $sourceRoot 'Client/Bin/Debug/Client.exe')
    $actualClient = Get-Content -Raw -LiteralPath (Join-Path $targetRoot 'Client/Bin/Debug/Client.exe')
    if ($actualClient -ne $expectedClient) {
        throw 'Client.exe fixture was not installed.'
    }
    $resourceValue = (Get-Content -Raw -LiteralPath $targetResource).Trim()
    if ($resourceValue -ne 'drive-target-must-survive') {
        throw 'Drive-owned Resource was modified by runtime delivery.'
    }

    $maliciousRoot = Join-Path $testRoot 'malicious'
    $maliciousZip = Join-Path $testRoot 'malicious.zip'
    Expand-Archive -LiteralPath $zipPath -DestinationPath $maliciousRoot
    $maliciousManifestPath = Join-Path $maliciousRoot 'runtime-delivery.json'
    $maliciousManifest = Get-Content -Raw -LiteralPath $maliciousManifestPath | ConvertFrom-Json
    $maliciousManifest.files[0].path = 'Client/Bin/Resources/Effect/evil.dds'
    $maliciousManifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $maliciousManifestPath -Encoding UTF8
    Compress-Archive -Path (Join-Path $maliciousRoot '*') -DestinationPath $maliciousZip
    $maliciousRejected = $false
    try {
        & (Join-Path $PSScriptRoot 'Install-LostArkRuntimeDelivery.ps1') `
            -PackagePath $maliciousZip `
            -RepositoryRoot $targetRoot `
            -Confirm:$false
    }
    catch {
        $maliciousRejected = $_.Exception.Message -like '*forbidden path*'
    }
    if (-not $maliciousRejected) {
        throw 'Installer did not reject a Resource path in the manifest.'
    }

    $rollbackRoot = Join-Path $testRoot 'rollback-target'
    New-Item -ItemType Directory -Path (Join-Path $rollbackRoot 'Client/Bin/Debug') -Force | Out-Null
    Set-Content -LiteralPath (Join-Path $rollbackRoot 'Framework.sln') -Value 'fixture' -Encoding UTF8
    $rollbackClient = Join-Path $rollbackRoot 'Client/Bin/Debug/Client.exe'
    Set-Content -LiteralPath $rollbackClient -Value 'old-client-must-return' -Encoding UTF8
    New-Item -ItemType Directory -Path (Join-Path $rollbackRoot 'Server') -Force | Out-Null
    Set-Content -LiteralPath (Join-Path $rollbackRoot 'Server/Bin') -Value 'blocks-server-directory' -Encoding UTF8
    $rollbackTriggered = $false
    try {
        & (Join-Path $PSScriptRoot 'Install-LostArkRuntimeDelivery.ps1') `
            -PackagePath $zipPath `
            -RepositoryRoot $rollbackRoot `
            -Confirm:$false
    }
    catch {
        $rollbackTriggered = $true
    }
    if (-not $rollbackTriggered) {
        throw 'Rollback fixture did not trigger an install failure.'
    }
    if ((Get-Content -Raw -LiteralPath $rollbackClient).Trim() -ne 'old-client-must-return') {
        throw 'Installer did not restore a previously replaced runtime file.'
    }

    Write-Output 'Runtime delivery integration PASS: runtime modules, optional matching PDBs, staging exclusion, install, forbidden Resource rejection, and rollback.'
}
finally {
    if (Test-Path -LiteralPath $testRoot) {
        $cleanupPath = (Resolve-Path -LiteralPath $testRoot).Path
        if (-not $cleanupPath.Equals($testRoot, [StringComparison]::OrdinalIgnoreCase) -or
            -not ([IO.Path]::GetDirectoryName($cleanupPath)).Equals($temporaryParent, [StringComparison]::OrdinalIgnoreCase) -or
            ((Get-Item -LiteralPath $cleanupPath).Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
            throw "Refusing to remove an unexpected runtime delivery test directory: $cleanupPath"
        }
        Remove-Item -LiteralPath $cleanupPath -Recurse -Force
    }
}
