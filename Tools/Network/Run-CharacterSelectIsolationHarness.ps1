[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [ValidateRange(1000, 4000)]
    [int]$HarnessTimeoutMilliseconds = 4000,
    [ValidateRange(1000, 60000)]
    [int]$ServerStartupTimeoutMilliseconds = 10000,
    [ValidateRange(1, 65535)]
    [int]$HarnessPort = 17777,
    [switch]$G02IdentityFast,
    [ValidateSet('All', 'Core', 'Party2', 'Party4')]
    [string]$Scenario = 'All',
    [ValidateSet('Valtan', 'KoukuSaydon')]
    [string]$PartyTarget = 'Valtan',
    [string]$ServerDataRoot = ''
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$serverExe = Join-Path $repoRoot "Server\Bin\$Configuration\Server.exe"
$harnessExe = Join-Path $repoRoot `
    "Tools\CharacterSelectIsolationHarness\Bin\$Configuration\CharacterSelectIsolationHarness.exe"
$logRoot = [IO.Path]::GetFullPath((Join-Path $repoRoot `
    ".codex_tmp\character-select-isolation-harness\$Configuration"))
$generatedRoot = [IO.Path]::GetFullPath((Join-Path $repoRoot '.codex_tmp'))

if (-not $logRoot.StartsWith(
        $generatedRoot.TrimEnd('\') + '\',
        [StringComparison]::OrdinalIgnoreCase)) {
    throw "Harness log path escaped the generated root: $logRoot"
}
if (-not (Test-Path -LiteralPath $serverExe -PathType Leaf)) {
    throw "Server executable is missing: $serverExe"
}
if (-not (Test-Path -LiteralPath $harnessExe -PathType Leaf)) {
    throw "Character Select isolation harness executable is missing: $harnessExe"
}

if ($PartyTarget -ne 'KoukuSaydon' -and $ServerStartupTimeoutMilliseconds -gt 15000) {
    throw 'Existing scenarios keep the 15000 ms startup limit.'
}
if ($PartyTarget -eq 'KoukuSaydon' -and
    ($Scenario -ne 'Party4' -or $Configuration -ne 'Debug' -or $G02IdentityFast -or $HarnessTimeoutMilliseconds -ne 4000)) {
    throw 'KoukuSaydon requires Debug / Party4 / 4000 ms and uses explicit Debug combat setup.'
}
if ($PartyTarget -eq 'KoukuSaydon' -and [string]::IsNullOrWhiteSpace($ServerDataRoot)) {
    throw 'KoukuSaydon requires an isolated ServerDataRoot; the shared published root is not used.'
}
if (-not [string]::IsNullOrWhiteSpace($ServerDataRoot)) {
    $ServerDataRoot = [IO.Path]::GetFullPath($ServerDataRoot)
    $isolationRoot = [IO.Path]::GetFullPath((Join-Path $repoRoot 'out'))
    if (-not $ServerDataRoot.StartsWith($isolationRoot.TrimEnd('\') + '\',
            [StringComparison]::OrdinalIgnoreCase) -or
        -not (Test-Path -LiteralPath (Join-Path $ServerDataRoot 'Gameplay\Gameplay.bootstrap') -PathType Leaf)) {
        throw 'ServerDataRoot must be an existing published-data copy below repository out.'
    }
}
if ($G02IdentityFast -and $Scenario -notin @('All', 'Core')) {
    throw 'G02IdentityFast cannot be combined with a party-transfer scenario.'
}
if ($Scenario -eq 'All' -and -not $G02IdentityFast) {
    # Each guide approach takes real server travel time. Keep every case in
    # its own bounded process instead of extending the server's 60s limit.
    foreach ($caseName in @('Core', 'Party2', 'Party4')) {
        $caseArguments = @{
            Configuration = $Configuration
            HarnessTimeoutMilliseconds = $HarnessTimeoutMilliseconds
            ServerStartupTimeoutMilliseconds = $ServerStartupTimeoutMilliseconds
            HarnessPort = $HarnessPort
            Scenario = $caseName
            ServerDataRoot = $ServerDataRoot
        }
        & $PSCommandPath @caseArguments
    }
    Write-Host "Character Select isolation and 2/4-party live harnesses passed: $Configuration"
    return
}

function Get-HarnessPortListeners {
    return @(Get-NetTCPConnection -LocalPort $HarnessPort -State Listen `
        -ErrorAction SilentlyContinue)
}

function Write-CapturedLog {
    param(
        [string]$Path,
        [switch]$AsError
    )
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        return
    }
    foreach ($line in Get-Content -LiteralPath $Path) {
        if ($AsError) {
            [Console]::Error.WriteLine($line)
        }
        else {
            [Console]::Out.WriteLine($line)
        }
    }
}

$preexisting = Get-HarnessPortListeners
if ($preexisting.Count -ne 0) {
    $owners = @($preexisting | ForEach-Object { $_.OwningProcess } |
        Sort-Object -Unique)
    throw "TCP $HarnessPort is already listening; refusing to touch PID(s): $($owners -join ', ')"
}

New-Item -ItemType Directory -Path $logRoot -Force | Out-Null
$runToken = [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfff') + '.' + $Scenario + '.' + $PartyTarget
$serverStdout = Join-Path $logRoot "$runToken.server.stdout.log"
$serverStderr = Join-Path $logRoot "$runToken.server.stderr.log"
$harnessStdout = Join-Path $logRoot "$runToken.harness.stdout.log"
$harnessStderr = Join-Path $logRoot "$runToken.harness.stderr.log"
$serverProcess = $null
$serverStarted = $false
$harnessStarted = $false
$harnessProcess = $null
$harnessExternalTimeoutMilliseconds =
    $HarnessTimeoutMilliseconds * 13 + 3000
if ($PartyTarget -eq 'KoukuSaydon') {
    $harnessExternalTimeoutMilliseconds = 120000
}
$serverLifetimeMilliseconds = [Math]::Min(
    60000,
    [Math]::Max(
        15000,
        $harnessExternalTimeoutMilliseconds + 3000))

try {
    # 자식 Server에만 격리 데이터 루트를 전달. 팀 LAN 환경과 기존 프로세스는 보존.
    $serverStartInfo = [Diagnostics.ProcessStartInfo]::new()
    $serverStartInfo.FileName = $serverExe
    $serverStartInfo.WorkingDirectory = Split-Path -Parent $serverExe
    $serverStartInfo.UseShellExecute = $false
    $serverStartInfo.CreateNoWindow = $true
    $serverStartInfo.RedirectStandardOutput = $true
    $serverStartInfo.RedirectStandardError = $true
    $serverStartInfo.Arguments = "--bind-address 127.0.0.1 --port $HarnessPort"
    if ($PartyTarget -eq 'KoukuSaydon') {
        $serverStartInfo.Arguments += ' --headless'
    }
    else {
        $serverStartInfo.Arguments += " --smoke-timeout-ms $serverLifetimeMilliseconds"
    }
    if (-not [string]::IsNullOrWhiteSpace($ServerDataRoot)) {
        $serverStartInfo.EnvironmentVariables['LOSTARK_SERVER_DATA_ROOT'] = $ServerDataRoot
    }
    $serverProcess = [Diagnostics.Process]::new()
    $serverProcess.StartInfo = $serverStartInfo
    if (-not $serverProcess.Start()) {
        throw 'Owned Server failed to start.'
    }
    $serverStarted = $true
    $serverStdoutTask = $serverProcess.StandardOutput.ReadToEndAsync()
    $serverStderrTask = $serverProcess.StandardError.ReadToEndAsync()

    $startupDeadline = [DateTime]::UtcNow.AddMilliseconds(
        $ServerStartupTimeoutMilliseconds)
    $listenerReady = $false
    while ([DateTime]::UtcNow -lt $startupDeadline) {
        $serverProcess.Refresh()
        if ($serverProcess.HasExited) {
            throw "Owned Server exited during startup with code $($serverProcess.ExitCode)."
        }

        $listeners = Get-HarnessPortListeners
        if ($listeners.Count -ne 0) {
            $foreign = @($listeners | Where-Object {
                $_.OwningProcess -ne $serverProcess.Id
            })
            if ($foreign.Count -ne 0) {
                throw "TCP $HarnessPort ownership changed during harness startup."
            }
            $owned = @($listeners | Where-Object {
                $_.OwningProcess -eq $serverProcess.Id -and
                $_.LocalAddress -eq '127.0.0.1' -and
                $_.LocalPort -eq $HarnessPort
            })
            if ($owned.Count -eq 1) {
                $listenerReady = $true
                break
            }
        }
        Start-Sleep -Milliseconds 25
    }
    if (-not $listenerReady) {
        throw "Owned Server did not listen on 127.0.0.1:$HarnessPort within $ServerStartupTimeoutMilliseconds ms."
    }

    $harnessStartInfo = [Diagnostics.ProcessStartInfo]::new()
    $harnessStartInfo.FileName = $harnessExe
    $harnessStartInfo.WorkingDirectory = Split-Path -Parent $harnessExe
    $harnessStartInfo.UseShellExecute = $false
    $harnessStartInfo.CreateNoWindow = $true
    $harnessStartInfo.RedirectStandardOutput = $true
    $harnessStartInfo.RedirectStandardError = $true
    $harnessArguments =
        "--host 127.0.0.1 --port $HarnessPort --timeout-ms $HarnessTimeoutMilliseconds"
    if ($G02IdentityFast) {
        $harnessArguments += ' --g02-identity-fast'
    }
    elseif ($Scenario -eq 'Party2') {
        $harnessArguments += ' --bern-party-size 2'
    }
    elseif ($Scenario -eq 'Party4') {
        $harnessArguments += ' --bern-party-size 4'
    }
    if ($Scenario -in @('Party2', 'Party4')) {
        $harnessArguments += ' --party-target ' + $PartyTarget.ToLowerInvariant()
    }
    if ($PartyTarget -eq 'KoukuSaydon') {
        $revisionRows = @(Get-Content -LiteralPath (Join-Path $ServerDataRoot 'Gameplay\Gameplay.bootstrap') |
            Where-Object { $_ -match '^KOUKUSAYDONPRODUCTREVISION\s+ENCOUNTER_KOUKUSAYDON_G1\s+' })
        if ($revisionRows.Count -ne 1) {
            throw 'Isolated bootstrap must have exactly one canonical Kouku Product revision.'
        }
        $revisionFields = $revisionRows[0] -split '\s+'
        [uint32]$sourceRevision = 0
        if ($revisionFields.Count -ne 4 -or -not [uint32]::TryParse($revisionFields[3], [ref]$sourceRevision) -or $sourceRevision -eq 0) {
            throw 'Invalid Kouku Product revision in isolated bootstrap.'
        }
        $harnessArguments += " --kouku-source-revision $sourceRevision"
    }
    $harnessStartInfo.Arguments = $harnessArguments
    $harnessProcess = [Diagnostics.Process]::new()
    $harnessProcess.StartInfo = $harnessStartInfo
    if (-not $harnessProcess.Start()) {
        throw 'Character Select isolation harness failed to start.'
    }
    $harnessStarted = $true
    $harnessStdoutTask = $harnessProcess.StandardOutput.ReadToEndAsync()
    $harnessStderrTask = $harnessProcess.StandardError.ReadToEndAsync()
    $harnessTimedOut = -not $harnessProcess.WaitForExit(
        $harnessExternalTimeoutMilliseconds)
    if ($harnessTimedOut) {
        # This owned console harness creates no child processes. The no-arg
        # overload also works in Windows PowerShell's .NET Framework runtime.
        $harnessProcess.Kill()
        if (-not $harnessProcess.WaitForExit(5000)) {
            throw 'Character Select isolation harness did not stop after its timeout.'
        }
    }
    $harnessProcess.WaitForExit()
    $harnessExitCode = $harnessProcess.ExitCode
    Set-Content -LiteralPath $harnessStdout `
        -Value $harnessStdoutTask.GetAwaiter().GetResult() `
        -Encoding UTF8
    Set-Content -LiteralPath $harnessStderr `
        -Value $harnessStderrTask.GetAwaiter().GetResult() `
        -Encoding UTF8
    Write-CapturedLog -Path $harnessStdout
    Write-CapturedLog -Path $harnessStderr -AsError
    if ($harnessTimedOut) {
        throw "Character Select isolation harness exceeded its external timeout: $Scenario"
    }
    if (0 -ne $harnessExitCode) {
        throw "Character Select isolation harness failed with code $harnessExitCode."
    }

    Write-Host "Character Select isolation live harness passed: $Configuration / $Scenario / $PartyTarget"
}
catch {
    Write-CapturedLog -Path $harnessStdout
    Write-CapturedLog -Path $harnessStderr -AsError
    Write-CapturedLog -Path $serverStdout
    Write-CapturedLog -Path $serverStderr -AsError
    throw
}
finally {
    if ($harnessStarted) {
        $harnessProcess.Refresh()
        if (-not $harnessProcess.HasExited) {
            $harnessProcess.Kill()
            $null = $harnessProcess.WaitForExit(5000)
        }
        $harnessProcess.Dispose()
    }
    if ($serverStarted) {
        $serverProcess.Refresh()
        if (-not $serverProcess.HasExited) {
            Stop-Process -Id $serverProcess.Id -ErrorAction SilentlyContinue
            if (-not $serverProcess.WaitForExit(5000)) {
                throw 'Owned Server did not stop within the cleanup deadline.'
            }
        }
        if ($null -ne $serverStdoutTask) {
            Set-Content -LiteralPath $serverStdout -Value $serverStdoutTask.GetAwaiter().GetResult() -Encoding UTF8
            Set-Content -LiteralPath $serverStderr -Value $serverStderrTask.GetAwaiter().GetResult() -Encoding UTF8
            Write-CapturedLog -Path $serverStdout
            Write-CapturedLog -Path $serverStderr -AsError
        }
        $serverProcess.Dispose()
    }

    $cleanupDeadline = [DateTime]::UtcNow.AddSeconds(5)
    do {
        $residue = Get-HarnessPortListeners
        if ($residue.Count -eq 0) {
            break
        }
        Start-Sleep -Milliseconds 50
    } while ([DateTime]::UtcNow -lt $cleanupDeadline)

    if ($residue.Count -ne 0) {
        $owners = @($residue | ForEach-Object { $_.OwningProcess } |
            Sort-Object -Unique)
        throw "TCP $HarnessPort listener residue remains after owned-process cleanup: $($owners -join ', ')"
    }
}
