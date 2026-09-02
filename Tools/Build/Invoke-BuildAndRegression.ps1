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
if ($SkipBuild -and $Profile -ne 'Product') {
    throw '-SkipBuild is supported only for Product. Core/FullDiagnostic require freshly built harness executables and PDBs.'
}
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$includeCore = $Profile -in @('Core', 'FullDiagnostic')
$includeFullDiagnostic = $Profile -eq 'FullDiagnostic'
$clientExe = Join-Path $repoRoot "Client\Bin\$Configuration\Client.exe"
$serverExe = Join-Path $repoRoot "Server\Bin\$Configuration\Server.exe"
$protocolHarnessExe = Join-Path $repoRoot `
    "Tools\NetworkProtocolHarness\Bin\$Configuration\NetworkProtocolHarness.exe"
$characterSelectIsolationHarnessExe = Join-Path $repoRoot `
    "Tools\CharacterSelectIsolationHarness\Bin\$Configuration\CharacterSelectIsolationHarness.exe"
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
$domainManifestPath = Join-Path $PSScriptRoot 'BuildDomains.json'
$domainPipelinePath = Join-Path $PSScriptRoot 'BuildDomainPipeline.psm1'
$productOutputGuardPath = Join-Path $PSScriptRoot 'ProductOutputGuard.psm1'
$buildReceiptRoot = Join-Path $repoRoot 'out\BuildPipeline\receipts'
$buildEvidenceRoot = Join-Path $repoRoot 'out\BuildPipeline\runs'
Import-Module $domainPipelinePath -Force
Import-Module $productOutputGuardPath -Force
$buildDomainManifest = Read-BuildDomainManifest $domainManifestPath
$script:buildStepRecords = [Collections.Generic.List[object]]::new()
$script:buildDomainResults = [Collections.Generic.List[object]]::new()
$script:buildStartedUtc = [DateTime]::UtcNow.ToString('o')
$script:buildRunTimer = [Diagnostics.Stopwatch]::StartNew()
$script:buildStartGitIdentity = $null
$script:buildStartProductSourceInputSha256 = ''

function Add-BuildStepRecord {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][string]$Result,
        [Parameter(Mandatory = $true)][long]$ElapsedMilliseconds
    )

    $script:buildStepRecords.Add([pscustomobject][ordered]@{
        name = $Name
        result = $Result
        elapsedMs = $ElapsedMilliseconds
    }) | Out-Null
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

    $timer = [Diagnostics.Stopwatch]::StartNew()
    $result = 'FAIL'
    try {
        & $MSBuild $Project /m /nodeReuse:false /t:Build `
            "/p:Configuration=$Configuration" /p:Platform=x64 `
            /p:BuildProjectReferences=false /v:minimal
        if ($global:LASTEXITCODE -ne 0) {
            throw "Build failed: $Project"
        }
        $result = 'PASS'
    }
    finally {
        $timer.Stop()
        Add-BuildStepRecord "msbuild:$Project" $result $timer.ElapsedMilliseconds
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
    $timer = [Diagnostics.Stopwatch]::StartNew()
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
        $timer.Stop()
        $result = if ($pythonExitCode -eq 0) { 'PASS' } else { 'FAIL' }
        Add-BuildStepRecord "python:$Description" $result $timer.ElapsedMilliseconds
    }
    if ($pythonExitCode -ne 0) {
        throw "$Description failed."
    }
}

function Assert-ProductOutputsUnlocked {
    $timer = [Diagnostics.Stopwatch]::StartNew()
    $result = 'FAIL'
    try {
        Assert-StandardProductOutputsNotRunning -RepositoryRoot $repoRoot
        $result = 'PASS'
    }
    finally {
        $timer.Stop()
        Add-BuildStepRecord 'product:output-lock-preflight' $result `
            $timer.ElapsedMilliseconds
    }
}

function Assert-SkipBuildProductFreshness {
    $timer = [Diagnostics.Stopwatch]::StartNew()
    $result = 'FAIL'
    try {
        $freshness = Test-BuildProductReceipt $repoRoot $buildDomainManifest `
            $Configuration $buildReceiptRoot
        if (-not $freshness.Fresh) {
            throw "-SkipBuild rejected stale product output: $($freshness.Reason). Run without -SkipBuild first."
        }
        $result = 'PASS'
    }
    finally {
        $timer.Stop()
        Add-BuildStepRecord 'product:freshness' $result $timer.ElapsedMilliseconds
    }
}

function Invoke-SelectedBuildDomains {
    foreach ($domain in @(Get-BuildDomainsForProfile $buildDomainManifest $Profile)) {
        $timer = [Diagnostics.Stopwatch]::StartNew()
        $result = 'FAIL'
        try {
            $domainResult = Invoke-BuildDomain $repoRoot $domain `
                $runtimeResourceRoot $buildReceiptRoot
            $script:buildDomainResults.Add($domainResult) | Out-Null
            $result = if ($domainResult.reused) { 'REUSED' } else { 'PASS' }
            Write-Host "Build domain $($domainResult.domainId): $result"
        }
        finally {
            $timer.Stop()
            Add-BuildStepRecord "domain:$($domain.id)" $result `
                $timer.ElapsedMilliseconds
        }
    }
}

function Write-CurrentProductReceipt {
    $timer = [Diagnostics.Stopwatch]::StartNew()
    $result = 'FAIL'
    try {
        $path = Write-BuildProductReceipt $repoRoot $buildDomainManifest `
            $Configuration $buildReceiptRoot
        $result = 'PASS'
        Write-Host "Product build receipt: $path"
    }
    finally {
        $timer.Stop()
        Add-BuildStepRecord 'product:receipt' $result $timer.ElapsedMilliseconds
    }
}

function Capture-BuildStartIdentity {
    $script:buildStartGitIdentity = Get-BuildGitIdentity $repoRoot
    $source = Get-BuildProductSourceFingerprint $repoRoot `
        $buildDomainManifest $Configuration
    $script:buildStartProductSourceInputSha256 =
        [string]$source.sourceInputSha256
}

function Write-CurrentBuildEvidence {
    $stability = Assert-BuildRunStability $repoRoot $buildDomainManifest `
        $Configuration $runtimeResourceRoot $buildReceiptRoot `
        $script:buildStartGitIdentity `
        $script:buildStartProductSourceInputSha256 `
        @($script:buildDomainResults)
    $script:buildRunTimer.Stop()
    $path = Write-BuildRunEvidence $repoRoot $Configuration $Profile `
        ([bool]$SkipBuild) @($script:buildStepRecords) `
        @($script:buildDomainResults) $buildEvidenceRoot $stability `
        $script:buildStartedUtc $script:buildRunTimer.ElapsedMilliseconds
    Write-Host "Build evidence: $path"
}

$productRunLockPath = Join-Path (Join-Path $buildReceiptRoot 'locks') `
    "product.$($Configuration.ToLowerInvariant()).lock"
$productRunLock = Enter-BuildExclusiveLock $productRunLockPath 300000 `
    "$Configuration product build/run"
Push-Location $repoRoot
$previousResourceRoot = [Environment]::GetEnvironmentVariable(
    'LOSTARK_RESOURCE_ROOT', 'Process')
try {
    [Environment]::SetEnvironmentVariable(
        'LOSTARK_RESOURCE_ROOT', $runtimeResourceRoot, 'Process')

    # This is deliberately unconditional.  Build-domain publishers mutate shared
    # Data even for -SkipBuild, and either Debug or Release may still be running.
    Assert-ProductOutputsUnlocked

    Invoke-PythonGate `
        'Release client surface visibility and product input gate' `
        @('Tools/Build/test_release_client_surface_contract.py')
    Invoke-PythonGate `
        'Product-first build profile and project surface gate' `
        @('Tools/Build/test_build_profile_contract.py')
    Invoke-PythonGate `
        'Build-domain fingerprint, receipt, and freshness gate' `
        @('Tools/Build/test_build_domain_pipeline_receipts.py')
    Invoke-PythonGate `
        'DimensionMaster glass/water Tool audition canary gates' `
        @(
            '-m',
            'unittest',
            'Tools.EffectPipeline.test_dimensionmaster_2050230_mirror_particle_tool_canary',
            'Tools.EffectPipeline.test_dimensionmaster_2050230_single_glass_product_canary',
            'Tools.EffectPipeline.test_dimensionmaster_2050230_glass_water_visual_canary'
        )

    if ($includeCore) {
        Invoke-PythonGate `
            'Action Presentation Workbench joined-domain gate' `
            @('Tools/ValtanPipeline/test_action_presentation_workbench_contract.py')
        Invoke-PythonGate `
            'Bern entrance camera authored runtime gate' `
            @('Tools/ValtanPipeline/test_bern_entrance_camera_contract.py')
        Invoke-PythonGate `
            'KoukuSaton resource intake and world admission gate' `
            @('Tools/KakulSaydonPipeline/test_kakul_world_admission.py')
        Invoke-PythonGate `
            'KoukuSaton Client product level contract gate' `
            @('Tools/KakulSaydonPipeline/test_kakul_client_product_level_contract.py')
        Invoke-PythonGate `
            'Valtan eight-player raid capacity gate' `
            @('Tools/Network/test_valtan_raid_capacity_contract.py')
        Invoke-PythonGate `
            'Effect Tool V2 authored document gate' `
            @('Tools/EffectToolV2/test_validate_effect_v2.py')
        Invoke-PythonGate `
            'Effect Tool V2 binding schema, migration, and read-set gate' `
            @('Tools/EffectToolV2/test_effect_v2_binding_pipeline.py')
        Invoke-PythonGate `
            'Effect Tool V2 Product transaction gate' `
            @('Tools/EffectToolV2/test_effect_v2_product_contract.py')
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

    Invoke-SelectedBuildDomains
    Capture-BuildStartIdentity

    if ($SkipBuild) {
        Assert-SkipBuildProductFreshness
    }

    if (-not $SkipBuild) {
        $msbuild = Resolve-MSBuild
        Invoke-MSBuildProject $msbuild 'Engine\Default\Engine.vcxproj'
        Invoke-MSBuildProject $msbuild 'Shared\Default\Shared.vcxproj'
        if ($includeCore) {
            Invoke-MSBuildProject $msbuild `
                'Tools\NetworkProtocolHarness\Default\NetworkProtocolHarness.vcxproj'
            Invoke-MSBuildProject $msbuild `
                'Tools\CharacterSelectIsolationHarness\Default\CharacterSelectIsolationHarness.vcxproj'
            Invoke-MSBuildProject $msbuild `
                'Tools\ValtanPatternAuditionServiceHarness\Default\ValtanPatternAuditionServiceHarness.vcxproj'
        }
        Invoke-MSBuildProject $msbuild 'Server\Default\Server.vcxproj'
        Invoke-MSBuildProject $msbuild 'Client\Default\Client.vcxproj'
        if ($includeFullDiagnostic) {
            Invoke-MSBuildProject $msbuild `
                'Tools\PointLightFalloffContractHarness\Default\PointLightFalloffContractHarness.vcxproj'
            Invoke-MSBuildProject $msbuild `
                'Tools\PhysicsContractHarness\Default\PhysicsContractHarness.vcxproj'
            Invoke-MSBuildProject $msbuild `
                'Tools\WModelGeometryContractHarness\Default\WModelGeometryContractHarness.vcxproj'
        }
    }

    $shaderTimer = [Diagnostics.Stopwatch]::StartNew()
    $shaderResult = 'FAIL'
    try {
        $global:LASTEXITCODE = 0
        & '.\Tools\Build\Test-CompiledShaderClosure.ps1' `
            -Configuration $Configuration `
            -RepositoryRoot $repoRoot `
            -Modules Product
        if ($global:LASTEXITCODE -ne 0) {
            throw 'Compiled shader closure validation failed.'
        }
        $shaderResult = 'PASS'
    }
    finally {
        $shaderTimer.Stop()
        Add-BuildStepRecord 'product:compiled-shader-closure' $shaderResult `
            $shaderTimer.ElapsedMilliseconds
    }

    Assert-RuntimeLayout
    if (-not $SkipBuild) {
        Write-CurrentProductReceipt
    }

    if (-not $includeCore) {
        Write-CurrentBuildEvidence
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
			'Valtan pattern sound cue exact-join gate' `
			@('-m', 'unittest',
			  'Tools.ValtanPipeline.test_valtan_pattern_sound_cue_contract')
		Invoke-PythonGate `
			'Valtan Effect Tool master tree gate' `
			@('Tools/EffectPipeline/test_effect_tool_valtan_saved_rows.py')
		Invoke-PythonGate `
			'Valtan CROSS Product cue and fixed-step rock wave gate' `
			@('Tools/EffectPipeline/test_valtan_cross_rock_wave_effect.py')
		Invoke-PythonGate `
			'Valtan combat-object hit Effect presentation gate' `
			@('Tools/ValtanPipeline/test_valtan_combat_object_hit_effect_presentation_contract.py')
	}

    $global:LASTEXITCODE = 0
    & '.\Tools\WorldPipeline\Split-ValtanIndependentWallGroups.ps1' `
        -Mode CheckNavigation
    if ($global:LASTEXITCODE -ne 0) {
        throw 'Valtan wall navigation footprint validation failed.'
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

    & $valtanAuditionServiceHarnessExe
    if ($global:LASTEXITCODE -ne 0) {
        throw 'ValtanPatternAuditionServiceHarness failed.'
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
            'Tools\PointLightFalloffContractHarness\Run-PointLightFalloffContractHarness.ps1') `
            -Configuration $Configuration

        & (Join-Path $repoRoot `
            'Tools\PhysicsContractHarness\Run-PhysicsContractHarness.ps1') `
            -Configuration $Configuration

        & (Join-Path $repoRoot `
            'Tools\WModelGeometryContractHarness\Run-WModelGeometryContractHarness.ps1') `
            -Configuration $Configuration
    }

    Write-CurrentBuildEvidence
	Write-Host "Build and regression completed: $Configuration / $Profile"
    Write-Host 'Runtime level validation uses Framework.slnLaunch (Server + Client).'
}
finally {
    [Environment]::SetEnvironmentVariable(
        'LOSTARK_RESOURCE_ROOT', $previousResourceRoot, 'Process')
    Pop-Location
    $script:buildRunTimer.Stop()
    $productRunLock.Dispose()
}
