[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [switch]$SkipBuild,
    [switch]$SkipNetworkSmoke,
    [switch]$SkipDevelopmentSmoke,
    [switch]$DeepAssetHash,
    [ValidateRange(1000, 45000)]
    [int]$ClientTimeoutMs = 45000
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$reportParent = [IO.Path]::GetFullPath((Join-Path $repoRoot '.codex_tmp\regression'))
$reportRoot = [IO.Path]::GetFullPath((Join-Path $reportParent $Configuration))
$clientExe = Join-Path $repoRoot "Client\Bin\$Configuration\Client.exe"
$clientWorkingDirectory = Join-Path $repoRoot 'Client\Default'
$serverExe = Join-Path $repoRoot "Server\Bin\$Configuration\Server.exe"
$serverWorkingDirectory = Join-Path $repoRoot 'Server\Default'

function Resolve-MSBuild {
    $command = Get-Command msbuild.exe -ErrorAction SilentlyContinue
    if ($null -ne $command) {
        return $command.Source
    }

    $candidate = 'C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe'
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

    & $MSBuild $Project /m /nodeReuse:false /t:Build "/p:Configuration=$Configuration" /p:Platform=x64 /v:minimal
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed: $Project"
    }
}

function Assert-ClientRuntimeLayout {
    $required = @(
        $clientExe,
        (Join-Path $repoRoot 'Client\Bin\ShaderFiles\Shader_Deferred.hlsl'),
        (Join-Path $repoRoot 'Client\Bin\ShaderFiles\Shader_VtxTex.hlsl'),
        (Join-Path $repoRoot 'Client\Bin\Resources\Fonts')
    )
    $missing = @($required | Where-Object { -not (Test-Path -LiteralPath $_) })
    if ($missing.Count -ne 0) {
        throw "Client runtime layout is incomplete: $($missing -join ', ')"
    }
}

function Invoke-ClientSmoke {
    param(
        [string]$ScenarioId,
        [switch]$RequiresServer,
        [int]$TimeoutMs = $ClientTimeoutMs
    )

    $safeName = $ScenarioId.Replace('.', '-')
    $reportPath = Join-Path $reportRoot "$safeName.json"
    if (Test-Path -LiteralPath $reportPath -PathType Leaf) {
        Remove-Item -LiteralPath $reportPath -Force
    }

    $serverProcess = $null
	$clientProcess = $null
    try {
        if ($RequiresServer) {
			$existingListeners = @(Get-NetTCPConnection `
				-LocalPort 7777 `
				-State Listen `
				-ErrorAction SilentlyContinue)
			if ($existingListeners.Count -ne 0) {
				throw "Port 7777 is already owned by PID(s): $($existingListeners.OwningProcess -join ', ')"
			}

            $serverProcess = Start-Process `
                -FilePath $serverExe `
                -ArgumentList @('--smoke-timeout-ms', '60000') `
                -WorkingDirectory $serverWorkingDirectory `
                -WindowStyle Hidden `
                -PassThru

            $isListening = $false
            for ($attempt = 0; $attempt -lt 100; ++$attempt) {
				if ($serverProcess.HasExited) {
					throw "Server exited before opening port 7777. ExitCode=$($serverProcess.ExitCode)"
				}
				$ownedListener = @(Get-NetTCPConnection `
					-LocalPort 7777 `
					-State Listen `
					-ErrorAction SilentlyContinue |
					Where-Object OwningProcess -eq $serverProcess.Id)
                if ($ownedListener.Count -ne 0) {
                    $isListening = $true
                    break
                }
                Start-Sleep -Milliseconds 100
            }
            if (-not $isListening) {
                throw 'Server did not listen on port 7777 within 10 seconds.'
            }
        }

        $arguments = @(
            '--smoke',
            "--scenario=$ScenarioId",
            "--timeout-ms=$TimeoutMs",
            "--report=$reportPath"
        )
        $clientProcess = Start-Process `
            -FilePath $clientExe `
            -ArgumentList $arguments `
            -WorkingDirectory $clientWorkingDirectory `
            -WindowStyle Hidden `
            -PassThru

        if (-not $clientProcess.WaitForExit($TimeoutMs + 15000)) {
            Stop-Process -Id $clientProcess.Id -Force
            throw "Client process exceeded its smoke deadline: $ScenarioId"
        }
        if (-not (Test-Path -LiteralPath $reportPath -PathType Leaf)) {
            throw "Client did not write a smoke report: $ScenarioId"
        }
		if ($clientProcess.ExitCode -ne 0) {
			throw "Client exited with code $($clientProcess.ExitCode): $ScenarioId"
		}

        $report = Get-Content -LiteralPath $reportPath -Raw | ConvertFrom-Json
        if ($report.status -ne 'passed') {
            throw "Smoke failed: $ScenarioId / $($report.reason) / $($report.loadingStage)"
        }
        Write-Host "[PASS] $ScenarioId ($($report.elapsedMs) ms)"
    }
    finally {
		if ($null -ne $clientProcess -and -not $clientProcess.HasExited) {
			Stop-Process -Id $clientProcess.Id -Force
		}
        if ($null -ne $serverProcess -and -not $serverProcess.HasExited) {
            Stop-Process -Id $serverProcess.Id -Force
        }
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
        Invoke-MSBuildProject $msbuild 'Tools\NetworkProtocolHarness\Default\NetworkProtocolHarness.vcxproj'
        Invoke-MSBuildProject $msbuild 'Server\Default\Server.vcxproj'
        Invoke-MSBuildProject $msbuild 'Client\Default\Client.vcxproj'
    }

    Assert-ClientRuntimeLayout

    $protocolHarness = Join-Path $repoRoot "Tools\NetworkProtocolHarness\Bin\$Configuration\NetworkProtocolHarness.exe"
    & $protocolHarness
    if ($LASTEXITCODE -ne 0) {
        throw 'NetworkProtocolHarness failed.'
    }
	& $serverExe --contract-test
	if ($LASTEXITCODE -ne 0) {
		throw 'Server gameplay contract tests failed.'
	}

    Invoke-ClientSmoke -ScenarioId 'front.lobby'
    if (-not $SkipNetworkSmoke) {
        Invoke-ClientSmoke -ScenarioId 'world.bern' -RequiresServer
        Invoke-ClientSmoke -ScenarioId 'raid.valtan.arena' -RequiresServer
    }
    if (-not $SkipDevelopmentSmoke -and $Configuration -eq 'Debug') {
        Invoke-ClientSmoke -ScenarioId 'dev.map.active' -TimeoutMs ([Math]::Max($ClientTimeoutMs, 120000))
        Invoke-ClientSmoke -ScenarioId 'asset.character.lance-master'
        Invoke-ClientSmoke -ScenarioId 'render.hdr-readback'
        Invoke-ClientSmoke -ScenarioId 'effect.preview'
        Invoke-ClientSmoke -ScenarioId 'ui.hud.layout'
    }
	elseif (-not $SkipDevelopmentSmoke) {
		Write-Host '[SKIP] Release developer ImGui tools are intentionally not shipped; ProjectAudit validates their data contracts.'
	}

    $auditArguments = @{
        ReportPath = (Join-Path $reportRoot 'ProjectAudit.json')
    }
    if ($DeepAssetHash) {
        $auditArguments.DeepAssetHash = $true
    }
    & (Join-Path $repoRoot 'Tools\ProjectAudit\Invoke-ProjectAudit.ps1') @auditArguments
    if ($LASTEXITCODE -ne 0) {
        throw 'Project audit failed.'
    }

    Write-Host "Regression completed: $Configuration"
}
finally {
    Pop-Location
}
