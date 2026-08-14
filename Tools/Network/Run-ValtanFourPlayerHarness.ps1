[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [ValidateRange(1000, 30000)]
    [int]$HarnessTimeoutMilliseconds = 10000,
    [ValidateRange(1000, 15000)]
    [int]$ServerStartupTimeoutMilliseconds = 10000,
    [ValidateRange(1, 65535)]
    [int]$HarnessPort = 17778
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$serverExe = Join-Path $repoRoot "Server\Bin\$Configuration\Server.exe"
$harnessExe = Join-Path $repoRoot `
    "Tools\ValtanFourPlayerHarness\Bin\$Configuration\ValtanFourPlayerHarness.exe"
$logRoot = [IO.Path]::GetFullPath(
    (Join-Path $repoRoot ".codex_tmp\valtan-four-player-harness\$Configuration"))
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
    throw "Valtan four-player harness executable is missing: $harnessExe"
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
$runToken = [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfff')
$serverStdout = Join-Path $logRoot "$runToken.server.stdout.log"
$serverStderr = Join-Path $logRoot "$runToken.server.stderr.log"
$harnessStdout = Join-Path $logRoot "$runToken.harness.stdout.log"
$harnessStderr = Join-Path $logRoot "$runToken.harness.stderr.log"
$serverProcess = $null
$harnessProcess = $null
$serverLifetimeMilliseconds = [Math]::Min(
    60000,
    [Math]::Max(
        15000,
        $ServerStartupTimeoutMilliseconds + $HarnessTimeoutMilliseconds + 10000))

try {
    $serverProcess = Start-Process `
        -FilePath $serverExe `
        -ArgumentList @(
            '--bind-address', '127.0.0.1',
            '--port', $HarnessPort,
            '--smoke-timeout-ms', $serverLifetimeMilliseconds) `
        -WorkingDirectory (Split-Path -Parent $serverExe) `
        -WindowStyle Hidden `
        -RedirectStandardOutput $serverStdout `
        -RedirectStandardError $serverStderr `
        -PassThru

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
    $harnessStartInfo.Arguments =
        "--host 127.0.0.1 --port $HarnessPort --timeout-ms $HarnessTimeoutMilliseconds"
    $harnessProcess = [Diagnostics.Process]::new()
    $harnessProcess.StartInfo = $harnessStartInfo
    if (-not $harnessProcess.Start()) {
        throw 'Valtan four-player harness failed to start.'
    }
    $harnessStdoutTask = $harnessProcess.StandardOutput.ReadToEndAsync()
    $harnessStderrTask = $harnessProcess.StandardError.ReadToEndAsync()
    if (-not $harnessProcess.WaitForExit($HarnessTimeoutMilliseconds + 5000)) {
        $harnessProcess.Kill($true)
        throw 'Valtan four-player harness exceeded its external timeout.'
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
    if (0 -ne $harnessExitCode) {
        throw "Valtan four-player harness failed with code $harnessExitCode."
    }

    Write-Host "Valtan four-player live harness passed: $Configuration"
}
catch {
    Write-CapturedLog -Path $harnessStdout
    Write-CapturedLog -Path $harnessStderr -AsError
    Write-CapturedLog -Path $serverStdout
    Write-CapturedLog -Path $serverStderr -AsError
    throw
}
finally {
    if ($null -ne $harnessProcess) {
        $harnessProcess.Refresh()
        if (-not $harnessProcess.HasExited) {
            $harnessProcess.Kill($true)
            $null = $harnessProcess.WaitForExit(5000)
        }
        $harnessProcess.Dispose()
    }
    if ($null -ne $serverProcess) {
        $serverProcess.Refresh()
        if (-not $serverProcess.HasExited) {
            Stop-Process -Id $serverProcess.Id -ErrorAction SilentlyContinue
            $null = $serverProcess.WaitForExit(5000)
        }
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
