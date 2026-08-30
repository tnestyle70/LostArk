[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [Parameter(Mandatory = $true)]
    [string]$ResourceRoot
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
$executable = Join-Path $PSScriptRoot "Bin\$Configuration\EffectRenderContractHarness.exe"
$externalRoot = (Resolve-Path -LiteralPath $ResourceRoot).Path
$defaultRoot = Join-Path $repoRoot 'Client\Bin\Resources'
$missingRoot = Join-Path $repoRoot ('.codex_tmp\missing-resource-root-' + [Guid]::NewGuid().ToString('N'))
if (-not (Test-Path -LiteralPath $executable -PathType Leaf) -or
    -not (Test-Path -LiteralPath $externalRoot -PathType Container) -or
    -not (Test-Path -LiteralPath $defaultRoot -PathType Container)) {
    throw 'Built native harness and valid external/default directories are required.'
}

$previousRoot = [Environment]::GetEnvironmentVariable('LOSTARK_RESOURCE_ROOT', 'Process')
$previousShared = [Environment]::GetEnvironmentVariable('LOSTARK_SHARED_ASSET_ROOT', 'Process')
function Set-RootEnvironment {
    param([string]$Name, $Value)
    # PowerShell converts ordinary $null to an empty .NET string. NullString
    # preserves deletion, including when restoring a previously absent value.
    if ($null -eq $Value) {
        $Value = [System.Management.Automation.Language.NullString]::Value
    }
    [Environment]::SetEnvironmentVariable($Name, $Value, 'Process')
}
function Invoke-RootCase {
    # Preserve null so .NET 9 removes the variable instead of setting an
    # existing empty value. Empty strings remain explicit regression inputs.
    param([string]$Name, $Primary, $Shared, [string]$Expected, [int]$ExpectedExit = 0)
    Set-RootEnvironment 'LOSTARK_RESOURCE_ROOT' $Primary
    Set-RootEnvironment 'LOSTARK_SHARED_ASSET_ROOT' $Shared
    $previousNativeErrorAction = $ErrorActionPreference
    try {
        # Windows PowerShell wraps expected native stderr in an ErrorRecord.
        # Capture it so the negative cases can assert the real exit and reason.
        $ErrorActionPreference = 'Continue'
        $global:LASTEXITCODE = -1
        $lines = @(& $executable $repoRoot $Configuration '--validate-resource-root' 2>&1)
        $nativeExit = $global:LASTEXITCODE
    }
    finally {
        $ErrorActionPreference = $previousNativeErrorAction
    }
    if ($nativeExit -ne $ExpectedExit) {
        throw "$Name returned $nativeExit, expected ${ExpectedExit}: $($lines -join ' ')"
    }
    if ($ExpectedExit -eq 0) {
        $result = ($lines -join [Environment]::NewLine) | ConvertFrom-Json
        $selected = [IO.Path]::GetFullPath($result.resourceRoot).TrimEnd('\', '/')
        $wanted = [IO.Path]::GetFullPath($Expected).TrimEnd('\', '/')
        if (-not [StringComparer]::OrdinalIgnoreCase.Equals($selected, $wanted) -or
            -not $result.assetPathBoundaryValidated) {
            throw "$Name selected '$selected', expected '$wanted', or lost the asset path boundary."
        }
    }
    elseif (($lines -join ' ') -notmatch 'harness resource root is unavailable:') {
        throw "$Name did not preserve the invalid configured-root failure reason."
    }
    Write-Output "PASS $Name"
}

try {
    Invoke-RootCase 'external primary' $externalRoot $null $externalRoot
    Invoke-RootCase 'primary takes precedence' $externalRoot $missingRoot $externalRoot
    Invoke-RootCase 'shared root fallback' $null $externalRoot $externalRoot
    Invoke-RootCase 'repository default' $null $null $defaultRoot
    Invoke-RootCase 'empty roots use repository default' '' '' $defaultRoot
    Invoke-RootCase 'empty primary permits shared fallback' '' $externalRoot $externalRoot
    Invoke-RootCase 'invalid primary cannot fall back' $missingRoot $externalRoot $null 2
    Invoke-RootCase 'invalid shared cannot fall back' $null $missingRoot $null 2
    Write-Output "EffectRenderResourceRoot: PASS ($Configuration, 8 native cases)"
    # The last native command intentionally returned 2. Do not leak that
    # expected rejection as the aggregate gate's process status.
    $global:LASTEXITCODE = 0
}
finally {
    Set-RootEnvironment 'LOSTARK_RESOURCE_ROOT' $previousRoot
    Set-RootEnvironment 'LOSTARK_SHARED_ASSET_ROOT' $previousShared
}
