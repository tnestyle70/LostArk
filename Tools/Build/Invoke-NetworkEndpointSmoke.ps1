param(
	[ValidateSet('Debug', 'Release')]
	[string]$Configuration = 'Debug',

	[ValidateRange(30000, 180000)]
	[int]$ClientTimeoutMs = 90000
)

$ErrorActionPreference = 'Stop'
$repositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$clientPath = Join-Path $repositoryRoot "Client\Bin\$Configuration\Client.exe"
$serverPath = Join-Path $repositoryRoot "Server\Bin\$Configuration\Server.exe"
$clientWorkingDirectory = Join-Path $repositoryRoot 'Client\Default'
$serverWorkingDirectory = Join-Path $repositoryRoot 'Server\Default'

foreach ($requiredPath in @($clientPath, $serverPath)) {
	if (-not (Test-Path -LiteralPath $requiredPath -PathType Leaf)) {
		throw "Required executable is missing: $requiredPath"
	}
}

function Resolve-ActiveLanIPv4 {
	$defaultRoutes = @(Get-NetRoute `
		-AddressFamily IPv4 `
		-DestinationPrefix '0.0.0.0/0' `
		-ErrorAction SilentlyContinue |
		Sort-Object RouteMetric, InterfaceMetric)
	foreach ($route in $defaultRoutes) {
		$addresses = @(Get-NetIPAddress `
			-AddressFamily IPv4 `
			-InterfaceIndex $route.InterfaceIndex `
			-ErrorAction SilentlyContinue |
			Where-Object {
				$_.IPAddress -notlike '127.*' -and
				$_.IPAddress -notlike '169.254.*' -and
				-not $_.SkipAsSource
			})
		if ($addresses.Count -ne 0) {
			return $addresses[0].IPAddress
		}
	}
	throw 'No active LAN IPv4 address was found from the default routes.'
}

function Assert-PortIsClear {
	$listeners = @(Get-NetTCPConnection `
		-LocalPort 7777 `
		-State Listen `
		-ErrorAction SilentlyContinue)
	if ($listeners.Count -ne 0) {
		throw "Port 7777 is already owned by PID(s): $($listeners.OwningProcess -join ', ')"
	}
}

function Wait-ForOwnedAnyListener {
	param([Diagnostics.Process]$Process)

	for ($attempt = 0; $attempt -lt 100; ++$attempt) {
		if ($Process.HasExited) {
			throw "Server exited before opening its listener. ExitCode=$($Process.ExitCode)"
		}
		$listener = @(Get-NetTCPConnection `
			-LocalPort 7777 `
			-State Listen `
			-ErrorAction SilentlyContinue |
			Where-Object {
				$_.OwningProcess -eq $Process.Id -and
				$_.LocalAddress -eq '0.0.0.0'
			})
		if ($listener.Count -eq 1) {
			return
		}
		Start-Sleep -Milliseconds 100
	}
	throw "Server PID $($Process.Id) did not own exactly one 0.0.0.0:7777 listener."
}

function Stop-OwnedProcess {
	param([Diagnostics.Process]$Process)
	if ($null -ne $Process -and -not $Process.HasExited) {
		Stop-Process -Id $Process.Id -Force -ErrorAction SilentlyContinue
		$null = $Process.WaitForExit(5000)
	}
}

function Wait-ForPortCleanup {
	for ($attempt = 0; $attempt -lt 50; ++$attempt) {
		$listeners = @(Get-NetTCPConnection `
			-LocalPort 7777 `
			-State Listen `
			-ErrorAction SilentlyContinue)
		if ($listeners.Count -eq 0) {
			return
		}
		Start-Sleep -Milliseconds 100
	}
	throw 'Port 7777 listener remained after the network smoke cleanup.'
}

function Invoke-NetworkScenario {
	param(
		[string]$Name,
		[string]$ExpectedReason,
		[int]$ServerLifetimeMs,
		[switch]$ExpectDisconnectRecovery
	)

	Assert-PortIsClear
	$serverProcess = $null
	$clientProcess = $null
	$reportPath = Join-Path $reportRoot "$Name.json"
	try {
		$serverProcess = Start-Process `
			-FilePath $serverPath `
			-ArgumentList @(
				'--bind-address', '0.0.0.0',
				'--smoke-timeout-ms', "$ServerLifetimeMs") `
			-WorkingDirectory $serverWorkingDirectory `
			-WindowStyle Hidden `
			-PassThru
		Wait-ForOwnedAnyListener -Process $serverProcess

		$arguments = @(
			'--smoke',
			'--scenario=world.bern',
			'--entry-mode=multiplayer',
			"--server-host=$lanAddress",
			'--server-port=7777',
			'--character-class=lance-master',
			"--timeout-ms=$ClientTimeoutMs",
			"--report=$reportPath"
		)
		if ($ExpectDisconnectRecovery) {
			$arguments += '--expect-disconnect-recovery'
		}
		$clientProcess = Start-Process `
			-FilePath $clientPath `
			-ArgumentList $arguments `
			-WorkingDirectory $clientWorkingDirectory `
			-WindowStyle Hidden `
			-PassThru

		if (-not $clientProcess.WaitForExit($ClientTimeoutMs + 15000)) {
			throw "Client exceeded the network smoke deadline: $Name"
		}
		if (0 -ne $clientProcess.ExitCode) {
			throw "Client exited with code $($clientProcess.ExitCode): $Name"
		}
		if (-not (Test-Path -LiteralPath $reportPath -PathType Leaf)) {
			throw "Client did not write a network smoke report: $Name"
		}

		$report = Get-Content -LiteralPath $reportPath -Raw | ConvertFrom-Json
		if ($report.status -ne 'passed' -or $report.reason -ne $ExpectedReason) {
			throw "Network smoke failed: $Name status=$($report.status) reason=$($report.reason)"
		}
		if ($ExpectDisconnectRecovery -and
			($report.networkConnected -ne $false -or
			 $report.networkCommandSinkLiveCount -ne 0 -or
			 $report.requestedScenarioId -ne 'world.bern')) {
			throw "Disconnect recovery retained online state: connected=$($report.networkConnected) sinks=$($report.networkCommandSinkLiveCount)"
		}

		Write-Host "[PASS] $Name endpoint=$($lanAddress):7777 elapsedMs=$($report.elapsedMs) report=$reportPath"
	}
	finally {
		Stop-OwnedProcess -Process $clientProcess
		Stop-OwnedProcess -Process $serverProcess
		Wait-ForPortCleanup
	}
}

function Invoke-EnterApprovalTimeoutScenario {
	Assert-PortIsClear
	$listener = $null
	$acceptedClient = $null
	$clientProcess = $null
	$reportPath = Join-Path $reportRoot 'enter-approval-timeout.json'
	try {
		$listener = [Net.Sockets.TcpListener]::new(
			[Net.IPAddress]::Loopback,
			7777)
		$listener.Start()
		$acceptTask = $listener.AcceptTcpClientAsync()
		$clientProcess = Start-Process `
			-FilePath $clientPath `
			-ArgumentList @(
				'--smoke',
				'--scenario=world.bern',
				'--entry-mode=multiplayer',
				'--server-host=127.0.0.1',
				'--server-port=7777',
				'--character-class=lance-master',
				'--expect-enter-approval-timeout',
				'--timeout-ms=30000',
				"--report=$reportPath") `
			-WorkingDirectory $clientWorkingDirectory `
			-WindowStyle Hidden `
			-PassThru

		if (-not $acceptTask.Wait(5000)) {
			throw 'Silent test listener did not accept the Client connection.'
		}
		$acceptedClient = $acceptTask.Result
		if (-not $clientProcess.WaitForExit(45000)) {
			throw 'Client exceeded the enter-approval timeout smoke deadline.'
		}
		if (0 -ne $clientProcess.ExitCode) {
			throw "Client exited with code $($clientProcess.ExitCode): enter-approval-timeout"
		}
		if (-not (Test-Path -LiteralPath $reportPath -PathType Leaf)) {
			throw 'Client did not write the enter-approval timeout report.'
		}

		$report = Get-Content -LiteralPath $reportPath -Raw | ConvertFrom-Json
		if ($report.status -ne 'passed' -or
			$report.reason -ne 'network-enter-approval-timeout-returned-to-lobby' -or
			$report.networkConnected -ne $false -or
			$report.networkCommandSinkLiveCount -ne 0 -or
			$report.requestedScenarioId -ne 'world.bern') {
			throw "Enter approval timeout smoke failed: status=$($report.status) reason=$($report.reason)"
		}
		Write-Host "[PASS] enter-approval-timeout elapsedMs=$($report.elapsedMs) report=$reportPath"
	}
	finally {
		Stop-OwnedProcess -Process $clientProcess
		if ($null -ne $acceptedClient) {
			$acceptedClient.Dispose()
		}
		if ($null -ne $listener) {
			$listener.Stop()
		}
		Wait-ForPortCleanup
	}
}

$lanAddress = Resolve-ActiveLanIPv4
$reportRoot = Join-Path ([IO.Path]::GetTempPath()) (
	'lostark-network-endpoint-smoke-' + [guid]::NewGuid().ToString('N'))
[IO.Directory]::CreateDirectory($reportRoot) | Out-Null

Invoke-NetworkScenario `
	-Name 'lan-endpoint' `
	-ExpectedReason 'map-and-player-replication-ready' `
	-ServerLifetimeMs 60000
Invoke-NetworkScenario `
	-Name 'disconnect-recovery' `
	-ExpectedReason 'network-disconnect-returned-to-lobby' `
	-ServerLifetimeMs 30000 `
	-ExpectDisconnectRecovery
Invoke-EnterApprovalTimeoutScenario

Write-Host "Network endpoint smoke passed: configuration=$Configuration lanAddress=$lanAddress"
