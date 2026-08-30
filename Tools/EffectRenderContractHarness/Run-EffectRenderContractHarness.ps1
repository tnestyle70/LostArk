param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [string]$ResourceRoot = '',
    [switch]$AllowLocalEffectResources,
    [switch]$ValidateResourceRootOnly
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
if ([string]::IsNullOrWhiteSpace($ResourceRoot)) {
    $ResourceRoot = Join-Path $repoRoot 'Client\Bin\Resources'
}
$ResourceRoot = [IO.Path]::GetFullPath($ResourceRoot)
$executable = Join-Path $PSScriptRoot `
    "Bin\$Configuration\EffectRenderContractHarness.exe"
if (-not (Test-Path -LiteralPath $executable -PathType Leaf)) {
    throw "EffectRenderContractHarness was not built: $executable"
}

function Invoke-RequiredNativeValidation {
    param(
        [Parameter(Mandatory = $true)][string]$Executable,
        [Parameter(Mandatory = $true)][string[]]$Arguments,
        [Parameter(Mandatory = $true)][string]$FailureLabel
    )

    $previousErrorAction = $ErrorActionPreference
    $exitCode = -1
    try {
        $ErrorActionPreference = 'Continue'
        $global:LASTEXITCODE = -1
        & $Executable @Arguments
        $exitCode = $global:LASTEXITCODE
    }
    finally {
        $ErrorActionPreference = $previousErrorAction
    }
    if ($exitCode -ne 0) {
        throw "$FailureLabel failed with exit code $exitCode"
    }
}

if (-not $ValidateResourceRootOnly) {
    $v1Arguments = @(
        '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File',
        (Join-Path $repoRoot 'Tools\EffectPipeline\Validate-EffectSources.ps1'),
        '-RepositoryRoot', $repoRoot, '-ResourceRoot', $ResourceRoot)
    if ($AllowLocalEffectResources) {
        $v1Arguments += '-AllowLocalResources'
    }
    Invoke-RequiredNativeValidation -Executable 'powershell.exe' `
        -Arguments $v1Arguments `
        -FailureLabel 'Effect V1 contract validation'

    Invoke-RequiredNativeValidation -Executable 'python' `
        -Arguments @((Join-Path $repoRoot 'Tools\EffectToolV2\validate_effect_v2.py'),
            '--repository-root', $repoRoot, '--resource-root', $ResourceRoot) `
        -FailureLabel 'Effect V2 contract validation'

    foreach ($shaderName in @(
        'Shader_VtxEffectMeshPreview.cso',
        'Shader_EffectMeshV2.cso')) {
        $shader = Join-Path $repoRoot "Client\Bin\$Configuration\$shaderName"
        if (-not (Test-Path -LiteralPath $shader -PathType Leaf)) {
            throw "Product Client shader is missing: $shader"
        }
    }
}

$nativeArguments = @($repoRoot, $Configuration)
if ($ValidateResourceRootOnly) {
    $nativeArguments += '--validate-resource-root'
}
$previousNativeErrorAction = $ErrorActionPreference
$nativeExitCode = -1
try {
    # Native stderr carries fail-closed rejection reasons. Preserve the real
    # process exit code instead of converting it into a PowerShell exception.
    $ErrorActionPreference = 'Continue'
    $global:LASTEXITCODE = -1
    & $executable @nativeArguments
    $nativeExitCode = $global:LASTEXITCODE
}
finally {
    $ErrorActionPreference = $previousNativeErrorAction
}
if ($nativeExitCode -ne 0) {
    throw "EffectRenderContractHarness failed with exit code $nativeExitCode"
}
