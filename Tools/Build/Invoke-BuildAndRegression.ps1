[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [switch]$SkipBuild,
    [string]$ResourceRoot = ''
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$clientExe = Join-Path $repoRoot "Client\Bin\$Configuration\Client.exe"
$serverExe = Join-Path $repoRoot "Server\Bin\$Configuration\Server.exe"
$valtanHarnessExe = Join-Path $repoRoot `
    "Tools\ValtanFourPlayerHarness\Bin\$Configuration\ValtanFourPlayerHarness.exe"
$characterSelectIsolationHarnessExe = Join-Path $repoRoot `
    "Tools\CharacterSelectIsolationHarness\Bin\$Configuration\CharacterSelectIsolationHarness.exe"
$effectRenderHarnessExe = Join-Path $repoRoot `
    "Tools\EffectRenderContractHarness\Bin\$Configuration\EffectRenderContractHarness.exe"
$pointLightFalloffHarnessExe = Join-Path $repoRoot `
    "Tools\PointLightFalloffContractHarness\Bin\$Configuration\PointLightFalloffContractHarness.exe"
$runtimeResourceRoot = if ([string]::IsNullOrWhiteSpace($ResourceRoot)) {
    Join-Path $repoRoot 'Client\Bin\Resources'
}
else {
    [IO.Path]::GetFullPath($ResourceRoot)
}

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
        $valtanHarnessExe,
        $characterSelectIsolationHarnessExe,
        $effectRenderHarnessExe,
        $pointLightFalloffHarnessExe,
        (Join-Path $repoRoot 'Client\Bin\ShaderFiles\Shader_Deferred.hlsl'),
        (Join-Path $repoRoot 'Client\Bin\ShaderFiles\Shader_VtxTex.hlsl'),
        (Join-Path $runtimeResourceRoot 'Fonts')
    )
    $missing = @($required | Where-Object {
        -not (Test-Path -LiteralPath $_)
    })
    if ($missing.Count -ne 0) {
        throw "Runtime layout is incomplete: $($missing -join ', ')"
    }
}

function Invoke-PythonGate {
    param(
        [string]$Description,
        [string[]]$Arguments
    )

    $python = (Get-Command python -ErrorAction Stop).Source
    $pythonExitCode = -1
    $previousErrorActionPreference = $ErrorActionPreference
    try {
        # unittest writes successful progress to stderr.  Windows PowerShell
        # surfaces that stream as NativeCommandError when the script-wide
        # preference is Stop, so preserve the process exit code explicitly.
        $ErrorActionPreference = 'Continue'
        $LASTEXITCODE = 0
        & $python -B @Arguments
        $pythonExitCode = $LASTEXITCODE
    }
    finally {
        $ErrorActionPreference = $previousErrorActionPreference
    }
    if ($pythonExitCode -ne 0) {
        throw "$Description failed."
    }
}

Push-Location $repoRoot
$previousResourceRoot = [Environment]::GetEnvironmentVariable(
    'LOSTARK_RESOURCE_ROOT', 'Process')
try {
    [Environment]::SetEnvironmentVariable(
        'LOSTARK_RESOURCE_ROOT', $runtimeResourceRoot, 'Process')

    if (-not $SkipBuild) {
        $msbuild = Resolve-MSBuild
        Invoke-MSBuildProject $msbuild 'Engine\Default\Engine.vcxproj'
        & cmd /c ".\UpdateLib.bat $Configuration"
        if ($LASTEXITCODE -ne 0) {
            throw 'UpdateLib.bat failed.'
        }
        Invoke-MSBuildProject $msbuild 'Shared\Default\Shared.vcxproj'
        Invoke-MSBuildProject $msbuild `
            'Tools\NetworkProtocolHarness\Default\NetworkProtocolHarness.vcxproj'
        Invoke-MSBuildProject $msbuild `
            'Tools\ValtanFourPlayerHarness\Default\ValtanFourPlayerHarness.vcxproj'
        Invoke-MSBuildProject $msbuild `
            'Tools\CharacterSelectIsolationHarness\Default\CharacterSelectIsolationHarness.vcxproj'
        Invoke-MSBuildProject $msbuild 'Server\Default\Server.vcxproj'
        Invoke-MSBuildProject $msbuild 'Client\Default\Client.vcxproj'
        Invoke-MSBuildProject $msbuild `
            'Tools\EffectRenderContractHarness\Default\EffectRenderContractHarness.vcxproj'
        Invoke-MSBuildProject $msbuild `
            'Tools\PointLightFalloffContractHarness\Default\PointLightFalloffContractHarness.vcxproj'
    }

    $LASTEXITCODE = 0
    & '.\Tools\Build\Test-CompiledShaderClosure.ps1' `
        -Configuration $Configuration `
        -RepositoryRoot $repoRoot
    if ($LASTEXITCODE -ne 0) {
        throw 'Compiled shader closure validation failed.'
    }

    Assert-RuntimeLayout

	$LASTEXITCODE = 0
	& '.\Tools\ValtanPipeline\Project-ValtanPatternMaster.ps1' `
		-Mode ValidateV2
	if ($LASTEXITCODE -ne 0) {
		throw 'Valtan split pattern master validation failed.'
	}
	$LASTEXITCODE = 0
	& '.\Tools\ValtanPipeline\Test-ValtanPatternMaster.ps1'
	if ($LASTEXITCODE -ne 0) {
		throw 'Valtan split pattern master focused harness failed.'
	}
	Invoke-PythonGate `
		'Valtan Animation Tool master timeline gate' `
		@('Tools/ValtanPipeline/test_animation_tool_valtan_pattern_master.py')
	Invoke-PythonGate `
		'Valtan Effect Tool master tree gate' `
		@('Tools/EffectPipeline/test_effect_tool_valtan_saved_rows.py')

    $LASTEXITCODE = 0
    & '.\Tools\GameplayPipeline\Publish-BalanceRuntimeSet.ps1' `
        -Mode Validate
    if ($LASTEXITCODE -ne 0) {
        throw 'Gameplay balance validation failed.'
    }

    $LASTEXITCODE = 0
    & '.\Tools\NavigationPipeline\Publish-ServerNavigation.ps1' `
        -Mode Validate
    if ($LASTEXITCODE -ne 0) {
        throw 'Server navigation validation failed.'
    }

    $LASTEXITCODE = 0
    & '.\Tools\EffectPipeline\Validate-EffectSources.ps1' `
        -RepositoryRoot $repoRoot
    if ($LASTEXITCODE -ne 0) {
        throw 'Effect source validation failed.'
    }

    $LASTEXITCODE = 0
    & '.\Tools\EffectPipeline\Sync-EffectDataProject.ps1' -Check
    if ($LASTEXITCODE -ne 0) {
        throw 'Effect project registration validation failed.'
    }

    Invoke-PythonGate `
        'Artist 31470 material runtime-oracle unit gate' `
        @('Tools/LevelPlacementExtractor/test_build_artist_31470_material_runtime_oracle.py')
    Invoke-PythonGate `
        'Artist 31470 material runtime-oracle receipt gate' `
        @('Tools/LevelPlacementExtractor/build_artist_31470_material_runtime_oracle.py',
          '--shallow-check')
    Invoke-PythonGate `
        'Artist 31470 material runtime-oracle HLSL/WARP gate' `
        @('Tools/LevelPlacementExtractor/verify_artist_31470_material_runtime_oracle_hlsl.py')
    Invoke-PythonGate `
        'Valtan floor crack emissive runtime contract gate' `
        @('Tools/LevelPlacementExtractor/test_valtan_floor_emissive_contract.py')
    Invoke-PythonGate `
        'Ground-target preview prototype scope gate' `
        @('Tools/GameplayPipeline/test_ground_target_preview_prototype_scope.py')

    $LASTEXITCODE = 0
    & '.\Tools\RenderingPipeline\Publish-RenderingProfiles.ps1' `
        -Mode Validate
    if ($LASTEXITCODE -ne 0) {
        throw 'Rendering profile validation failed.'
    }
    Invoke-PythonGate `
        'Rendering authored/runtime identity and float32 boundary gate' `
        @('Tools/RenderingPipeline/test_publish_rendering_profiles.py')

    $protocolHarness = Join-Path $repoRoot `
        "Tools\NetworkProtocolHarness\Bin\$Configuration\NetworkProtocolHarness.exe"
    & $protocolHarness
    if ($LASTEXITCODE -ne 0) {
        throw 'NetworkProtocolHarness failed.'
    }

    & $serverExe --contract-test
    if ($LASTEXITCODE -ne 0) {
        throw 'Server gameplay contract tests failed.'
    }

    & (Join-Path $repoRoot `
        'Tools\Network\Run-ValtanFourPlayerHarness.ps1') `
        -Configuration $Configuration

	& (Join-Path $repoRoot `
		'Tools\Network\Run-CharacterSelectIsolationHarness.ps1') `
		-Configuration $Configuration

    & (Join-Path $repoRoot `
        'Tools\EffectRenderContractHarness\Run-EffectRenderContractHarness.ps1') `
        -Configuration $Configuration

    & (Join-Path $repoRoot `
        'Tools\PointLightFalloffContractHarness\Run-PointLightFalloffContractHarness.ps1') `
        -Configuration $Configuration

	Write-Host "Regression completed: $Configuration"
    Write-Host 'Runtime level validation uses Framework.slnLaunch (Server + Client).'
}
finally {
    [Environment]::SetEnvironmentVariable(
        'LOSTARK_RESOURCE_ROOT', $previousResourceRoot, 'Process')
    Pop-Location
}
