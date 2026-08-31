Set-StrictMode -Version Latest

function ConvertTo-ProductOutputFullPath {
    param([Parameter(Mandatory = $true)][string]$Path)

    return [IO.Path]::GetFullPath($Path)
}

function Get-StandardProductOutputPaths {
    [CmdletBinding()]
    param([Parameter(Mandatory = $true)][string]$RepositoryRoot)

    $root = ConvertTo-ProductOutputFullPath $RepositoryRoot
    $outputs = [Collections.Generic.List[object]]::new()
    foreach ($configuration in @('Debug', 'Release')) {
        foreach ($role in @('Client', 'Server')) {
            $outputs.Add([pscustomobject][ordered]@{
                Configuration = $configuration
                Role = $role
                Path = ConvertTo-ProductOutputFullPath (Join-Path $root `
                    "$role\Bin\$configuration\$role.exe")
            }) | Out-Null
        }
    }
    return @($outputs)
}

function Get-RunningStandardProductOutputs {
    [CmdletBinding()]
    param([Parameter(Mandatory = $true)][string]$RepositoryRoot)

    $targets = [Collections.Generic.Dictionary[string,object]]::new(
        [StringComparer]::OrdinalIgnoreCase)
    foreach ($output in @(Get-StandardProductOutputPaths $RepositoryRoot)) {
        $targets[[string]$output.Path] = $output
    }

    $running = [Collections.Generic.List[object]]::new()
    # Standard outputs have fixed executable names; filtering first avoids
    # opening Path on every unrelated system process while the exact-path
    # comparison below remains the authority.
    foreach ($process in @(Get-Process -Name @('Client', 'Server') `
            -ErrorAction SilentlyContinue)) {
        $processPath = $null
        try { $processPath = $process.Path } catch { continue }
        if ([string]::IsNullOrWhiteSpace([string]$processPath)) { continue }

        try {
            $fullProcessPath = ConvertTo-ProductOutputFullPath $processPath
        }
        catch {
            continue
        }
        if (-not $targets.ContainsKey($fullProcessPath)) { continue }

        $target = $targets[$fullProcessPath]
        $running.Add([pscustomobject][ordered]@{
            Configuration = [string]$target.Configuration
            Role = [string]$target.Role
            ProcessName = [string]$process.ProcessName
            ProcessId = [int]$process.Id
            Path = $fullProcessPath
        }) | Out-Null
    }
    return @($running | Sort-Object Path, ProcessId)
}

function Assert-StandardProductOutputsNotRunning {
    [CmdletBinding()]
    param([Parameter(Mandatory = $true)][string]$RepositoryRoot)

    $running = @(Get-RunningStandardProductOutputs $RepositoryRoot)
    if ($running.Count -eq 0) { return }

    $details = @($running | ForEach-Object {
        $processName = if ($_.ProcessName.EndsWith(
                '.exe', [StringComparison]::OrdinalIgnoreCase)) {
            $_.ProcessName
        }
        else {
            "$($_.ProcessName).exe"
        }
        "$processName PID $($_.ProcessId) [$($_.Configuration)/$($_.Role)] ($($_.Path))"
    })
    throw ('Standard Product output is in use. Close every listed Debug/Release ' +
        'Server/Client process before any publisher, shared Data mutation, or ' +
        'compile step so an old EXE cannot run against newly projected Data: ' +
        ($details -join '; '))
}

Export-ModuleMember -Function @(
    'Get-StandardProductOutputPaths',
    'Get-RunningStandardProductOutputs',
    'Assert-StandardProductOutputsNotRunning'
)
