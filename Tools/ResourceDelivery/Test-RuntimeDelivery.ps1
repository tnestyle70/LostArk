[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$testRoot = Join-Path ([IO.Path]::GetTempPath()) ("lostark-runtime-delivery-test-" + [guid]::NewGuid().ToString('N'))
$sourceRoot = Join-Path $testRoot 'source'
$targetRoot = Join-Path $testRoot 'target'
$zipPath = Join-Path $testRoot 'runtime.zip'

try {
    foreach ($root in @($sourceRoot, $targetRoot)) {
        New-Item -ItemType Directory -Path $root -Force | Out-Null
        Set-Content -LiteralPath (Join-Path $root 'Framework.sln') -Value 'fixture' -Encoding UTF8
    }
    $sourceFiles = [ordered]@{
        'Client/Bin/Debug/Client.exe' = 'new-client'
        'Client/Bin/Debug/Engine.dll' = 'new-engine'
        'Client/Bin/DataFiles/World/test.json' = '{"ok":true}'
        'Server/Bin/Debug/Server.exe' = 'new-server'
        'Server/Bin/DataFiles/World/test.bootstrap' = 'world'
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

    Write-Output 'Runtime delivery integration PASS: install, forbidden Resource rejection, and rollback.'
}
finally {
    if (Test-Path -LiteralPath $testRoot) {
        Remove-Item -LiteralPath $testRoot -Recurse -Force
    }
}
