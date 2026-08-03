param(
	[ValidateSet('Debug', 'Release')]
	[string]$Configuration = 'Debug',

	[ValidateSet('All', 'Bern', 'Valtan', 'Training')]
	[string]$Scenario = 'All',

	[ValidateRange(10000, 300000)]
	[int]$TimeoutMs = 90000
)

$ErrorActionPreference = 'Stop'
$repositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$clientPath = Join-Path $repositoryRoot "Client\Bin\$Configuration\Client.exe"
$clientWorkingDirectory = Join-Path $repositoryRoot 'Client\Default'
if (-not (Test-Path -LiteralPath $clientPath -PathType Leaf)) {
	throw "Client executable is missing: $clientPath"
}

$selectedScenarios = switch ($Scenario) {
	'Bern' { @(@{ Id = 'world.bern'; Name = 'bern' }) }
	'Valtan' { @(@{ Id = 'raid.valtan.arena'; Name = 'valtan' }) }
	'Training' { @(@{ Id = 'dev.training.ground'; Name = 'training' }) }
	default {
		@(
			@{ Id = 'world.bern'; Name = 'bern' },
			@{ Id = 'raid.valtan.arena'; Name = 'valtan' },
			@{ Id = 'dev.training.ground'; Name = 'training' }
		)
	}
}

$reportRoot = Join-Path ([System.IO.Path]::GetTempPath()) (
	'lostark-offline-smoke-' + [guid]::NewGuid().ToString('N'))
[System.IO.Directory]::CreateDirectory($reportRoot) | Out-Null

foreach ($entry in $selectedScenarios) {
	$listeners = @(Get-NetTCPConnection `
		-LocalPort 7777 `
		-State Listen `
		-ErrorAction SilentlyContinue)
	if ($listeners.Count -ne 0) {
		throw "Offline smoke requires no listener on port 7777. PID(s): $($listeners.OwningProcess -join ', ')"
	}

	$reportPath = Join-Path $reportRoot "$($entry.Name).json"
	$arguments = @(
		"--scenario=$($entry.Id)",
		'--smoke',
		'--entry-mode=local',
		'--character-class=lance-master',
		"--timeout-ms=$TimeoutMs",
		"--report=$reportPath"
	)
	$process = Start-Process `
		-FilePath $clientPath `
		-ArgumentList $arguments `
		-WorkingDirectory $clientWorkingDirectory `
		-WindowStyle Hidden `
		-PassThru
	if (-not $process.WaitForExit($TimeoutMs + 15000)) {
		Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
		throw "Offline Client smoke timed out: $($entry.Id)"
	}
	if (0 -ne $process.ExitCode) {
		throw "Offline Client smoke exited with code $($process.ExitCode): $($entry.Id)"
	}
	if (-not (Test-Path -LiteralPath $reportPath -PathType Leaf)) {
		throw "Offline Client smoke did not write a report: $($entry.Id)"
	}

	$report = Get-Content -LiteralPath $reportPath -Raw | ConvertFrom-Json
	if ($report.status -ne 'passed' -or
		$report.reason -ne 'offline-stage-local-player-and-camera-ready' -or
		$report.networkConnected -ne $false -or
		$report.networkCommandSinkLiveCount -ne 0 -or
		$null -eq $report.selectedCharacterClassId -or
		[string]::IsNullOrEmpty($report.localPreviewPlacementId) -or
		$report.cameraFollowReady -ne $true) {
		throw "Offline Client smoke failed: $($entry.Id) status=$($report.status) reason=$($report.reason)"
	}

	$listenersAfter = @(Get-NetTCPConnection `
		-LocalPort 7777 `
		-State Listen `
		-ErrorAction SilentlyContinue)
	if ($listenersAfter.Count -ne 0) {
		throw "Offline smoke created or observed an unexpected 7777 listener."
	}

	Write-Host "[PASS] offline.$($entry.Name) elapsedMs=$($report.elapsedMs) report=$reportPath"
}

Write-Host "Offline Client smoke passed: configuration=$Configuration scenarios=$($selectedScenarios.Count)"
