param(
    [string]$RepositoryRoot = '',
    [string]$ResourceRoot = '',
    [switch]$AllowLocalResources
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
}
else {
    $RepositoryRoot = (Resolve-Path -LiteralPath $RepositoryRoot).Path
}

$validator = Join-Path $PSScriptRoot 'validate_effect_sources.py'
$validatorArguments = @($validator, '--repository-root', $RepositoryRoot)
if (-not [string]::IsNullOrWhiteSpace($ResourceRoot)) {
    $ResourceRoot = (Resolve-Path -LiteralPath $ResourceRoot).Path
    $validatorArguments += @('--resource-root', $ResourceRoot)
}
if ($AllowLocalResources) {
    $validatorArguments += '--allow-local-resources'
}
& python @validatorArguments
if ($LASTEXITCODE -ne 0) {
    throw "Effect source validation failed with exit code $LASTEXITCODE"
}
