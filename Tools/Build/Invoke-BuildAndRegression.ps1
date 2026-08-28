[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [switch]$SkipBuild,
    [string]$ResourceRoot = '',
    [switch]$AllowLocalEffectResources
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$clientExe = Join-Path $repoRoot "Client\Bin\$Configuration\Client.exe"
$serverExe = Join-Path $repoRoot "Server\Bin\$Configuration\Server.exe"
$valtanHarnessExe = Join-Path $repoRoot `
    "Tools\ValtanFourPlayerHarness\Bin\$Configuration\ValtanFourPlayerHarness.exe"
$characterSelectIsolationHarnessExe = Join-Path $repoRoot `
    "Tools\CharacterSelectIsolationHarness\Bin\$Configuration\CharacterSelectIsolationHarness.exe"
$actionPresentationTimelineHarnessExe = Join-Path $repoRoot `
    "Tools\ActionPresentationTimelineHarness\Bin\$Configuration\ActionPresentationTimelineHarness.exe"
$valtanAuditionServiceHarnessExe = Join-Path $repoRoot `
    "Tools\ValtanPatternAuditionServiceHarness\Bin\$Configuration\ValtanPatternAuditionServiceHarness.exe"
$effectRenderHarnessExe = Join-Path $repoRoot `
    "Tools\EffectRenderContractHarness\Bin\$Configuration\EffectRenderContractHarness.exe"
$pointLightFalloffHarnessExe = Join-Path $repoRoot `
    "Tools\PointLightFalloffContractHarness\Bin\$Configuration\PointLightFalloffContractHarness.exe"
$mapFrustumHarnessExe = Join-Path $repoRoot `
    "Tools\MapFrustumContractHarness\Bin\$Configuration\MapFrustumContractHarness.exe"
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
    if ($global:LASTEXITCODE -ne 0) {
        throw "Build failed: $Project"
    }
}

function Assert-RuntimeLayout {
    $required = @(
        $clientExe,
        $serverExe,
        $valtanHarnessExe,
        $characterSelectIsolationHarnessExe,
        $actionPresentationTimelineHarnessExe,
        $valtanAuditionServiceHarnessExe,
        $effectRenderHarnessExe,
        $pointLightFalloffHarnessExe,
        $mapFrustumHarnessExe,
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
        # Native commands update the global automatic variable. Assigning the
        # unqualified name here creates a function-local shadow in Windows
        # PowerShell, which made every failed Python gate look successful.
        $global:LASTEXITCODE = 0
        & $python -B @Arguments
        $pythonExitCode = $global:LASTEXITCODE
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
        if ($global:LASTEXITCODE -ne 0) {
            throw 'UpdateLib.bat failed.'
        }
        Invoke-MSBuildProject $msbuild 'Shared\Default\Shared.vcxproj'
        Invoke-MSBuildProject $msbuild `
            'Tools\NetworkProtocolHarness\Default\NetworkProtocolHarness.vcxproj'
        Invoke-MSBuildProject $msbuild `
            'Tools\ValtanPatternAuditionServiceHarness\Default\ValtanPatternAuditionServiceHarness.vcxproj'
        Invoke-MSBuildProject $msbuild `
            'Tools\ValtanFourPlayerHarness\Default\ValtanFourPlayerHarness.vcxproj'
        Invoke-MSBuildProject $msbuild `
            'Tools\CharacterSelectIsolationHarness\Default\CharacterSelectIsolationHarness.vcxproj'
        Invoke-MSBuildProject $msbuild `
            'Tools\ActionPresentationTimelineHarness\Default\ActionPresentationTimelineHarness.vcxproj'
        Invoke-MSBuildProject $msbuild 'Server\Default\Server.vcxproj'
        Invoke-MSBuildProject $msbuild 'Client\Default\Client.vcxproj'
        Invoke-MSBuildProject $msbuild `
            'Tools\EffectRenderContractHarness\Default\EffectRenderContractHarness.vcxproj'
        Invoke-MSBuildProject $msbuild `
            'Tools\PointLightFalloffContractHarness\Default\PointLightFalloffContractHarness.vcxproj'
        Invoke-MSBuildProject $msbuild `
            'Tools\MapFrustumContractHarness\Default\MapFrustumContractHarness.vcxproj'
    }

    $global:LASTEXITCODE = 0
    & '.\Tools\Build\Test-CompiledShaderClosure.ps1' `
        -Configuration $Configuration `
        -RepositoryRoot $repoRoot
    if ($global:LASTEXITCODE -ne 0) {
        throw 'Compiled shader closure validation failed.'
    }

    Assert-RuntimeLayout

    $global:LASTEXITCODE = 0
    & '.\Tools\ProjectAudit\Test-BernFrustumCullingContract.ps1'
    if ($global:LASTEXITCODE -ne 0) {
        throw 'Bern frustum source contract validation failed.'
    }

    $global:LASTEXITCODE = 0
    & '.\Tools\MapFrustumContractHarness\Run-MapFrustumContractHarness.ps1' `
        -Configuration $Configuration
    if ($global:LASTEXITCODE -ne 0) {
        throw 'MapFrustumContractHarness failed.'
    }
    Invoke-PythonGate `
        'Map surface geometry and depth diagnostic unit gate' `
        @('Tools/MapPipeline/test_map_surface_depth_contract.py')

    & (Join-Path $repoRoot 'Tools\Build\Test-NativeHarnessExitPropagation.ps1') `
        -Configuration $Configuration

	$global:LASTEXITCODE = 0
	& '.\Tools\ValtanPipeline\Project-ValtanPatternMaster.ps1' `
		-Mode ValidateV2
	if ($global:LASTEXITCODE -ne 0) {
		throw 'Valtan split pattern master validation failed.'
	}
	$global:LASTEXITCODE = 0
	& '.\Tools\ValtanPipeline\Test-ValtanPatternMaster.ps1'
	if ($global:LASTEXITCODE -ne 0) {
		throw 'Valtan split pattern master focused harness failed.'
	}
	Invoke-PythonGate `
		'Valtan Animation Tool master timeline gate' `
		@('Tools/ValtanPipeline/test_animation_tool_valtan_pattern_master.py')
	Invoke-PythonGate `
		'Valtan Effect Tool master tree gate' `
		@('Tools/EffectPipeline/test_effect_tool_valtan_saved_rows.py')

    $global:LASTEXITCODE = 0
    & '.\Tools\GameplayPipeline\Publish-BalanceRuntimeSet.ps1' `
        -Mode Validate
    if ($global:LASTEXITCODE -ne 0) {
        throw 'Gameplay balance validation failed.'
    }

    $global:LASTEXITCODE = 0
    & '.\Tools\NavigationPipeline\Publish-ServerNavigation.ps1' `
        -Mode Validate
    if ($global:LASTEXITCODE -ne 0) {
        throw 'Server navigation validation failed.'
    }

    $global:LASTEXITCODE = 0
    & '.\Tools\WorldPipeline\Split-ValtanIndependentWallGroups.ps1' `
        -Mode CheckNavigation
    if ($global:LASTEXITCODE -ne 0) {
        throw 'Valtan wall navigation footprint validation failed.'
    }
    $global:LASTEXITCODE = 0
    & '.\Tools\WorldPipeline\Publish-ValtanWorldDestruction.ps1' `
        -Mode Validate
    if ($global:LASTEXITCODE -ne 0) {
        throw 'Valtan world destruction validation failed.'
    }

    $global:LASTEXITCODE = 0
    & '.\Tools\EffectPipeline\Validate-EffectSources.ps1' `
        -RepositoryRoot $repoRoot -AllowLocalResources:$AllowLocalEffectResources
    if ($global:LASTEXITCODE -ne 0) {
        throw 'Effect source validation failed.'
    }

    $global:LASTEXITCODE = 0
    & '.\Tools\EffectPipeline\Sync-EffectDataProject.ps1' -Check
    if ($global:LASTEXITCODE -ne 0) {
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
        'Valtan body collision and model composition gate' `
        @('Tools/EffectPipeline/test_valtan_model_view_composition.py')
    Invoke-PythonGate `
        'Ground-target preview prototype scope gate' `
        @('Tools/GameplayPipeline/test_ground_target_preview_prototype_scope.py')

    $global:LASTEXITCODE = 0
    & '.\Tools\RenderingPipeline\Publish-RenderingProfiles.ps1' `
        -Mode Validate
    if ($global:LASTEXITCODE -ne 0) {
        throw 'Rendering profile validation failed.'
    }
    Invoke-PythonGate `
        'Rendering authored/runtime identity and float32 boundary gate' `
        @('Tools/RenderingPipeline/test_publish_rendering_profiles.py')

    $protocolHarness = Join-Path $repoRoot `
        "Tools\NetworkProtocolHarness\Bin\$Configuration\NetworkProtocolHarness.exe"
    & $protocolHarness
    if ($global:LASTEXITCODE -ne 0) {
        throw 'NetworkProtocolHarness failed.'
    }

    & $valtanAuditionServiceHarnessExe
    if ($global:LASTEXITCODE -ne 0) {
        throw 'ValtanPatternAuditionServiceHarness failed.'
    }

    & $serverExe --contract-test
    if ($global:LASTEXITCODE -ne 0) {
        throw 'Server gameplay contract tests failed.'
    }

    & (Join-Path $repoRoot `
        'Tools\Network\Run-ValtanFourPlayerHarness.ps1') `
        -Configuration $Configuration

	& (Join-Path $repoRoot `
		'Tools\Network\Run-CharacterSelectIsolationHarness.ps1') `
		-Configuration $Configuration

    & (Join-Path $repoRoot `
        'Tools\ActionPresentationTimelineHarness\Run-ActionPresentationTimelineHarness.ps1') `
        -Configuration $Configuration

    & (Join-Path $repoRoot `
        'Tools\EffectRenderContractHarness\Test-EffectRenderResourceRoot.ps1') `
        -Configuration $Configuration -ResourceRoot $runtimeResourceRoot

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
