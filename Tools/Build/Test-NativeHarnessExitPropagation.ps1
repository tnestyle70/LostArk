[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug'
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$guardedScripts = @(
    'Tools/Build/Invoke-BuildAndRegression.ps1',
    'Tools/ActionPresentationTimelineHarness/Run-ActionPresentationTimelineHarness.ps1',
    'Tools/EffectRenderContractHarness/Run-EffectRenderContractHarness.ps1',
    'Tools/PointLightFalloffContractHarness/Run-PointLightFalloffContractHarness.ps1',
    'Tools/ValtanPipeline/Test-ValtanPatternMaster.ps1',
    'Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1'
)
foreach ($relativePath in $guardedScripts) {
    $parseErrors = $null
    $tokens = $null
    $ast = [Management.Automation.Language.Parser]::ParseFile(
        (Join-Path $repoRoot $relativePath), [ref]$tokens, [ref]$parseErrors)
    if ($parseErrors.Count -ne 0) {
        throw "PowerShell parse failed: $relativePath"
    }
    $scopedExitReferences = @($ast.FindAll({
        param($node)
        $node -is [Management.Automation.Language.VariableExpressionAst] -and
            $node.VariablePath.UserPath -ieq 'LASTEXITCODE'
    }, $true))
    if ($scopedExitReferences.Count -ne 0) {
        throw "Native exit status can be shadowed in $relativePath"
    }
}

$previousRoot = [Environment]::GetEnvironmentVariable('LOSTARK_RESOURCE_ROOT', 'Process')
$missingRoot = Join-Path $repoRoot ('.codex_tmp\missing-native-exit-probe-' + [Guid]::NewGuid().ToString('N'))
function Invoke-NativeFailureWithExitShadow {
    # Deliberately reproduce callers that keep a local success sentinel.
    $LASTEXITCODE = 0
    $failure = $null
    try {
        & (Join-Path $repoRoot 'Tools/EffectRenderContractHarness/Run-EffectRenderContractHarness.ps1') `
            -Configuration $Configuration 2>&1 | Out-Null
    }
    catch {
        $failure = $_
    }
    if ($null -eq $failure) {
        throw 'A real native failure was masked by the caller exit-code sentinel.'
    }
    if ($failure.Exception.Message -notmatch
        'harness resource root is unavailable:|EffectRenderContractHarness failed with exit code 2') {
        throw "Unexpected native-failure probe error: $($failure.Exception.Message)"
    }
}

try {
    [Environment]::SetEnvironmentVariable('LOSTARK_RESOURCE_ROOT', $missingRoot, 'Process')
    Invoke-NativeFailureWithExitShadow
    $global:LASTEXITCODE = 0
    Write-Output "Native harness exit propagation: PASS ($Configuration, six script guards and a real rejection)"
}
finally {
    $restoreValue = if ($null -eq $previousRoot) {
        [Management.Automation.Language.NullString]::Value
    }
    else { $previousRoot }
    [Environment]::SetEnvironmentVariable('LOSTARK_RESOURCE_ROOT', $restoreValue, 'Process')
}
