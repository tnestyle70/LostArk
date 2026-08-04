[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [switch]$SkipBuild,
    [switch]$DeepAssetHash
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$reportParent = [IO.Path]::GetFullPath(
    (Join-Path $repoRoot '.codex_tmp\regression'))
$reportRoot = [IO.Path]::GetFullPath(
    (Join-Path $reportParent $Configuration))
$clientExe = Join-Path $repoRoot "Client\Bin\$Configuration\Client.exe"
$serverExe = Join-Path $repoRoot "Server\Bin\$Configuration\Server.exe"

function Resolve-MSBuild {
    $command = Get-Command msbuild.exe -ErrorAction SilentlyContinue
    if ($null -ne $command) {
        return $command.Source
    }

    $candidate =
        'C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe'
    if (Test-Path -LiteralPath $candidate -PathType Leaf) {
        return $candidate
    }
    throw 'MSBuild.exe was not found.'
}

function Invoke-MSBuildProject {
    param(
        [string]$MSBuild,
        [string]$Project
    )

    & $MSBuild $Project /m /nodeReuse:false /t:Build `
        "/p:Configuration=$Configuration" /p:Platform=x64 /v:minimal
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed: $Project"
    }
}

function Assert-RuntimeLayout {
    $required = @(
        $clientExe,
        $serverExe,
        (Join-Path $repoRoot 'Client\Bin\ShaderFiles\Shader_Deferred.hlsl'),
        (Join-Path $repoRoot 'Client\Bin\ShaderFiles\Shader_VtxTex.hlsl'),
        (Join-Path $repoRoot 'Client\Bin\Resources\Fonts')
    )
    $missing = @($required | Where-Object {
        -not (Test-Path -LiteralPath $_)
    })
    if ($missing.Count -ne 0) {
        throw "Runtime layout is incomplete: $($missing -join ', ')"
    }
}

Push-Location $repoRoot
try {
    if (-not $reportRoot.StartsWith(
            $reportParent.TrimEnd('\') + '\',
            [StringComparison]::OrdinalIgnoreCase)) {
        throw "Regression report path escaped its generated root: $reportRoot"
    }
    if ([IO.Directory]::Exists($reportRoot)) {
        Remove-Item -LiteralPath $reportRoot -Recurse -Force
    }
    New-Item -ItemType Directory -Path $reportRoot -Force | Out-Null

    if (-not $SkipBuild) {
        $msbuild = Resolve-MSBuild
        Invoke-MSBuildProject $msbuild 'Engine\Default\Engine.vcxproj'
        & cmd /c "UpdateLib.bat $Configuration"
        if ($LASTEXITCODE -ne 0) {
            throw 'UpdateLib.bat failed.'
        }
        Invoke-MSBuildProject $msbuild 'Shared\Default\Shared.vcxproj'
        Invoke-MSBuildProject $msbuild `
            'Tools\NetworkProtocolHarness\Default\NetworkProtocolHarness.vcxproj'
        Invoke-MSBuildProject $msbuild `
            'Tools\ClientFrontendHarness\Default\ClientFrontendHarness.vcxproj'
        Invoke-MSBuildProject $msbuild 'Server\Default\Server.vcxproj'
        Invoke-MSBuildProject $msbuild 'Client\Default\Client.vcxproj'
    }

    Assert-RuntimeLayout

    $protocolHarness = Join-Path $repoRoot `
        "Tools\NetworkProtocolHarness\Bin\$Configuration\NetworkProtocolHarness.exe"
    & $protocolHarness
    if ($LASTEXITCODE -ne 0) {
        throw 'NetworkProtocolHarness failed.'
    }

    $frontendHarness = Join-Path $repoRoot `
        "Tools\ClientFrontendHarness\Bin\$Configuration\ClientFrontendHarness.exe"
    & $frontendHarness
    if ($LASTEXITCODE -ne 0) {
        throw 'ClientFrontendHarness failed.'
    }

    & $serverExe --contract-test
    if ($LASTEXITCODE -ne 0) {
        throw 'Server gameplay contract tests failed.'
    }

    $auditArguments = @{
        ReportPath = (Join-Path $reportRoot 'ProjectAudit.json')
    }
    if ($DeepAssetHash) {
        $auditArguments.DeepAssetHash = $true
    }
    & (Join-Path $repoRoot `
        'Tools\ProjectAudit\Invoke-ProjectAudit.ps1') @auditArguments
    if ($LASTEXITCODE -ne 0) {
        throw 'Project audit failed.'
    }

    Write-Host "Regression completed: $Configuration"
    Write-Host 'Runtime level validation uses Framework.slnLaunch (Server + Client).'
}
finally {
    Pop-Location
}
