param(
    [string]$RepositoryRoot = ''
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
}
else {
    $RepositoryRoot = (Resolve-Path -LiteralPath $RepositoryRoot).Path
}

$validator = Join-Path $PSScriptRoot 'validate_effect_sources.py'
& python $validator --repository-root $RepositoryRoot
if ($LASTEXITCODE -ne 0) {
    throw "Effect source validation failed with exit code $LASTEXITCODE"
}
