param()

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$publishSource = Join-Path $repoRoot `
    'Tools\EffectPipeline\publish_four_class_authored_rollout.py'
$publisherTests = Join-Path $repoRoot `
    'Tools\EffectPipeline\test_publish_four_class_authored_rollout.py'
$componentBuilder = Join-Path $repoRoot `
    'Tools\LevelPlacementExtractor\build_effect_components.py'
$componentBuilderTests = Join-Path $repoRoot `
    'Tools\LevelPlacementExtractor\test_build_effect_components.py'
$effectPublisher = Join-Path $repoRoot `
    'Tools\EffectPipeline\Publish-Effects.ps1'
$projectSync = Join-Path $repoRoot `
    'Tools\EffectPipeline\Sync-EffectDataProject.ps1'
$receiptRoot = Join-Path $repoRoot `
    'Data\Effects\AuthoredCorrections\Generated\ComponentBuild'

function Invoke-PythonJson([string[]]$Arguments) {
    $output = @(& python @Arguments 2>&1)
    if ($LASTEXITCODE -ne 0) {
        throw ($output -join [Environment]::NewLine)
    }
    if ($output.Count -eq 0) {
        throw 'Python contract command returned no result.'
    }
    try {
        return ($output[-1] | ConvertFrom-Json)
    }
    catch {
        throw "Python contract command returned invalid JSON: $($output -join ' ')"
    }
}

function Invoke-PythonTest([string]$TestPath, [string]$FailureMessage) {
    # Python unittest writes normal progress to stderr. PowerShell 5.1 promotes
    # native stderr to ErrorRecord, so a caller using ErrorActionPreference=Stop
    # would otherwise abort this audit even when the test process exits 0.
    $previousErrorActionPreference = $ErrorActionPreference
    try {
        $ErrorActionPreference = 'Continue'
        $output = @(& python $TestPath 2>&1)
        $exitCode = $LASTEXITCODE
    }
    finally {
        $ErrorActionPreference = $previousErrorActionPreference
    }
    if ($exitCode -ne 0) {
        throw ("{0}`n{1}" -f $FailureMessage, ($output -join [Environment]::NewLine))
    }
    if ($output.Count -gt 0) {
        Write-Host ($output -join [Environment]::NewLine)
    }
}

Invoke-PythonTest $publisherTests `
    'Four-class Authored rollout publisher unit tests failed.'
Invoke-PythonTest $componentBuilderTests `
    'Effect Component builder unit tests failed.'

$rollout = Invoke-PythonJson @(
    $publishSource,
    '--check',
    '--check-runtime'
)
if (
    [int]$rollout.skillCount -ne 51 -or
    [int]$rollout.stageCount -ne 74 -or
    [int]$rollout.clipOccurrenceCount -ne 113 -or
    [int]$rollout.effectBearingStageCount -ne 73 -or
    [int]$rollout.sourceIntentionallySilentStageCount -ne 1 -or
    [int]$rollout.blockedStageCount -ne 0 -or
    [int]$rollout.generatedApproximationStageCount -ne 71 -or
    [int]$rollout.preservedExistingStageCount -ne 1 -or
    [int]$rollout.reusedProductTargetStageCount -ne 1 -or
    [int]$rollout.visualClipOccurrenceCount -ne 102 -or
    [int]$rollout.silentClipOccurrenceCount -ne 11 -or
    [int]$rollout.derivedClipTargetOccurrenceCount -ne 48 -or
    [int]$rollout.retainedStageTargetOccurrenceCount -ne 54 -or
    [int]$rollout.derivedClipTargetCount -ne 48 -or
    [int]$rollout.retainedStageTargetCount -ne 53 -or
    [int]$rollout.trimmedElementCount -ne 0 -or
    [int]$rollout.unmappedElementCount -ne 0 -or
    [int]$rollout.productTargetCount -ne 101 -or
    [int]$rollout.productCueCount -ne 101 -or
    [int]$rollout.runtimeEffectCount -ne 101
) {
    throw "Four-class Authored rollout count mismatch: $($rollout | ConvertTo-Json -Compress)"
}

$componentResult = Invoke-PythonJson @(
    $componentBuilder,
    '--all-product-classes',
    '--receipt-root', $receiptRoot,
    '--verify-existing'
)
if (-not [bool]$componentResult.compileIdentityComplete) {
    throw 'Four-class Effect Component compile identity is incomplete.'
}
$componentCount = [int]$componentResult.componentCount
$emitterCount = [int]$componentResult.emitterCount

& powershell -ExecutionPolicy Bypass -File $effectPublisher -Mode Validate
if ($LASTEXITCODE -ne 0) {
    throw 'Publish-Effects Validate failed.'
}
& powershell -ExecutionPolicy Bypass -File $projectSync -Check
if ($LASTEXITCODE -ne 0) {
    throw 'Effect data project registration is stale.'
}

if ($componentCount -ne [int]$rollout.runtimeComponentCount) {
    throw "Runtime/component build count mismatch: $($rollout.runtimeComponentCount) != $componentCount"
}

Write-Output (
    'PASS: Four-class Authored rollout ' +
    "skills=51 stages=74 clips=113 effectBearing=73 silent=1 visualClips=102 " +
    "derived=48 retained=53 targets=101 cues=101 Particle=0 " +
    "components=$componentCount emitters=$emitterCount"
)
