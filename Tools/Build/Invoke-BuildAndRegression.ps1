[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [ValidateSet('Product', 'Core', 'FullDiagnostic')]
    [string]$Profile = 'Core',
    [switch]$SkipBuild,
    [string]$ResourceRoot = '',
    [switch]$AllowLocalEffectResources
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$includeCore = $Profile -in @('Core', 'FullDiagnostic')
$includeFullDiagnostic = $Profile -eq 'FullDiagnostic'
$clientExe = Join-Path $repoRoot "Client\Bin\$Configuration\Client.exe"
$serverExe = Join-Path $repoRoot "Server\Bin\$Configuration\Server.exe"
$protocolHarnessExe = Join-Path $repoRoot `
    "Tools\NetworkProtocolHarness\Bin\$Configuration\NetworkProtocolHarness.exe"
$characterSelectIsolationHarnessExe = Join-Path $repoRoot `
    "Tools\CharacterSelectIsolationHarness\Bin\$Configuration\CharacterSelectIsolationHarness.exe"
$actionPresentationTimelineHarnessExe = Join-Path $repoRoot `
    "Tools\ActionPresentationTimelineHarness\Bin\$Configuration\ActionPresentationTimelineHarness.exe"
$valtanAuditionServiceHarnessExe = Join-Path $repoRoot `
    "Tools\ValtanPatternAuditionServiceHarness\Bin\$Configuration\ValtanPatternAuditionServiceHarness.exe"
$pointLightFalloffHarnessExe = Join-Path $repoRoot `
    "Tools\PointLightFalloffContractHarness\Bin\$Configuration\PointLightFalloffContractHarness.exe"
$physicsHarnessExe = Join-Path $repoRoot `
    "Tools\PhysicsContractHarness\Bin\$Configuration\PhysicsContractHarness.exe"
$wmodelHarnessExe = Join-Path $repoRoot `
    "Tools\WModelGeometryContractHarness\Bin\$Configuration\WModelGeometryContractHarness.exe"
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
        "/p:Configuration=$Configuration" /p:Platform=x64 `
        /p:BuildProjectReferences=false /v:minimal
    if ($global:LASTEXITCODE -ne 0) {
        throw "Build failed: $Project"
    }
}

function Assert-RuntimeLayout {
    $required = @(
        $clientExe,
        $serverExe,
        (Join-Path $repoRoot 'Client\Bin\ShaderFiles\Shader_Deferred.hlsl'),
        (Join-Path $repoRoot 'Client\Bin\ShaderFiles\Shader_VtxTex.hlsl')
    )
    if ($includeCore) {
        $required += @(
            $protocolHarnessExe,
            $characterSelectIsolationHarnessExe
        )
    }
    if ($includeFullDiagnostic) {
        $required += @(
            $actionPresentationTimelineHarnessExe,
            $valtanAuditionServiceHarnessExe,
            $pointLightFalloffHarnessExe,
            $physicsHarnessExe,
            $wmodelHarnessExe
        )
    }
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

    Invoke-PythonGate `
        'Release client surface visibility and product input gate' `
        @('Tools/Build/test_release_client_surface_contract.py')
    Invoke-PythonGate `
        'Product-first build profile and project surface gate' `
        @('Tools/Build/test_build_profile_contract.py')

    if ($includeCore) {
        Invoke-PythonGate `
            'Action Presentation Workbench joined-domain gate' `
            @('Tools/ValtanPipeline/test_action_presentation_workbench_contract.py')
        Invoke-PythonGate `
            'Bern entrance camera authored runtime gate' `
            @('Tools/ValtanPipeline/test_bern_entrance_camera_contract.py')
        Invoke-PythonGate `
            'KakulSaydon resource intake and world admission gate' `
            @('Tools/KakulSaydonPipeline/test_kakul_world_admission.py')
        Invoke-PythonGate `
            'KakulSaydon Client product level contract gate' `
            @('Tools/KakulSaydonPipeline/test_kakul_client_product_level_contract.py')
        Invoke-PythonGate `
            'Valtan eight-player raid capacity gate' `
            @('Tools/Network/test_valtan_raid_capacity_contract.py')
        Invoke-PythonGate `
            'Effect Tool V2 authored document gate' `
            @('Tools/EffectToolV2/test_validate_effect_v2.py')
        Invoke-PythonGate `
            'Team LAN endpoint contract gate' `
            @('Tools/Network/test_team_lan_endpoint_contract.py')

        $global:LASTEXITCODE = 0
        & '.\Tools\EffectPipeline\Validate-EffectSources.ps1' `
            -RepositoryRoot $repoRoot `
            -ResourceRoot $runtimeResourceRoot `
            -AllowLocalResources:$AllowLocalEffectResources
        if ($global:LASTEXITCODE -ne 0) {
            throw 'Effect source validation failed before compilation.'
        }
    }

    if (-not $SkipBuild) {
        $msbuild = Resolve-MSBuild
        Invoke-MSBuildProject $msbuild 'Engine\Default\Engine.vcxproj'
        & cmd /c ".\UpdateLib.bat $Configuration"
        if ($global:LASTEXITCODE -ne 0) {
            throw 'UpdateLib.bat failed.'
        }
        Invoke-MSBuildProject $msbuild 'Shared\Default\Shared.vcxproj'
        if ($includeCore) {
            Invoke-MSBuildProject $msbuild `
                'Tools\NetworkProtocolHarness\Default\NetworkProtocolHarness.vcxproj'
            Invoke-MSBuildProject $msbuild `
                'Tools\CharacterSelectIsolationHarness\Default\CharacterSelectIsolationHarness.vcxproj'
        }
        Invoke-MSBuildProject $msbuild 'Server\Default\Server.vcxproj'
        Invoke-MSBuildProject $msbuild 'Client\Default\Client.vcxproj'
        if ($includeFullDiagnostic) {
            Invoke-MSBuildProject $msbuild `
                'Tools\ValtanPatternAuditionServiceHarness\Default\ValtanPatternAuditionServiceHarness.vcxproj'
            Invoke-MSBuildProject $msbuild `
                'Tools\ActionPresentationTimelineHarness\Default\ActionPresentationTimelineHarness.vcxproj'
            Invoke-MSBuildProject $msbuild `
                'Tools\PointLightFalloffContractHarness\Default\PointLightFalloffContractHarness.vcxproj'
            Invoke-MSBuildProject $msbuild `
                'Tools\PhysicsContractHarness\Default\PhysicsContractHarness.vcxproj'
            Invoke-MSBuildProject $msbuild `
                'Tools\WModelGeometryContractHarness\Default\WModelGeometryContractHarness.vcxproj'
        }
    }

    $global:LASTEXITCODE = 0
    & '.\Tools\Build\Test-CompiledShaderClosure.ps1' `
        -Configuration $Configuration `
        -RepositoryRoot $repoRoot `
        -Modules Product
    if ($global:LASTEXITCODE -ne 0) {
        throw 'Compiled shader closure validation failed.'
    }

    Assert-RuntimeLayout

    if (-not $includeCore) {
        Write-Host "Build and validation completed: $Configuration / $Profile"
        Write-Host 'Runtime visual/audio validation remains user-operated.'
        return
    }

    if ($includeFullDiagnostic) {
        $global:LASTEXITCODE = 0
        & '.\Tools\MapPipeline\Test-MapWaterRenderContract.ps1'
        if ($global:LASTEXITCODE -ne 0) {
            throw 'Map water render contract validation failed.'
        }
        Invoke-PythonGate `
            'Map surface geometry and depth diagnostic unit gate' `
            @('Tools/MapPipeline/test_map_surface_depth_contract.py')
    }

	$global:LASTEXITCODE = 0
	& '.\Tools\ValtanPipeline\Project-ValtanPatternMaster.ps1' `
		-Mode ValidateV2
	if ($global:LASTEXITCODE -ne 0) {
		throw 'Valtan split pattern master validation failed.'
	}
	if ($includeFullDiagnostic) {
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
	}

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

    if ($includeFullDiagnostic) {
        Invoke-PythonGate `
            'Valtan floor crack emissive runtime contract gate' `
            @('Tools/LevelPlacementExtractor/test_valtan_floor_emissive_contract.py')
        Invoke-PythonGate `
            'Valtan body collision and model composition gate' `
            @('Tools/EffectPipeline/test_valtan_model_view_composition.py')
        Invoke-PythonGate `
            'Ground-target preview prototype scope gate' `
            @('Tools/GameplayPipeline/test_ground_target_preview_prototype_scope.py')
    }

    $global:LASTEXITCODE = 0
    & '.\Tools\RenderingPipeline\Publish-RenderingProfiles.ps1' `
        -Mode Validate
    if ($global:LASTEXITCODE -ne 0) {
        throw 'Rendering profile validation failed.'
    }
    if ($includeFullDiagnostic) {
        Invoke-PythonGate `
            'Rendering authored/runtime identity and float32 boundary gate' `
            @('Tools/RenderingPipeline/test_publish_rendering_profiles.py')
    }

    & $protocolHarnessExe
    if ($global:LASTEXITCODE -ne 0) {
        throw 'NetworkProtocolHarness failed.'
    }

    if ($includeFullDiagnostic) {
        & $valtanAuditionServiceHarnessExe
        if ($global:LASTEXITCODE -ne 0) {
            throw 'ValtanPatternAuditionServiceHarness failed.'
        }
    }

    if ($includeFullDiagnostic) {
        & $serverExe --contract-test
        if ($global:LASTEXITCODE -ne 0) {
            throw 'Server gameplay contract tests failed.'
        }
    }

	& (Join-Path $repoRoot `
		'Tools\Network\Run-CharacterSelectIsolationHarness.ps1') `
		-Configuration $Configuration `
		-Scenario Core

    if ($includeFullDiagnostic) {
		foreach ($partyScenario in @('Party2', 'Party4')) {
			& (Join-Path $repoRoot `
				'Tools\Network\Run-CharacterSelectIsolationHarness.ps1') `
				-Configuration $Configuration `
				-Scenario $partyScenario
		}

        & (Join-Path $repoRoot `
            'Tools\ActionPresentationTimelineHarness\Run-ActionPresentationTimelineHarness.ps1') `
            -Configuration $Configuration

        & (Join-Path $repoRoot `
            'Tools\PointLightFalloffContractHarness\Run-PointLightFalloffContractHarness.ps1') `
            -Configuration $Configuration

        & (Join-Path $repoRoot `
            'Tools\PhysicsContractHarness\Run-PhysicsContractHarness.ps1') `
            -Configuration $Configuration

        & (Join-Path $repoRoot `
            'Tools\WModelGeometryContractHarness\Run-WModelGeometryContractHarness.ps1') `
            -Configuration $Configuration
    }

	Write-Host "Build and regression completed: $Configuration / $Profile"
    Write-Host 'Runtime level validation uses Framework.slnLaunch (Server + Client).'
}
finally {
    [Environment]::SetEnvironmentVariable(
        'LOSTARK_RESOURCE_ROOT', $previousResourceRoot, 'Process')
    Pop-Location
}
