[CmdletBinding()]
param(
    [string]$RepositoryRoot = ''
)

$ErrorActionPreference = 'Stop'
$repoRoot = if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    [IO.Path]::GetFullPath($RepositoryRoot)
}
$test = Join-Path $PSScriptRoot 'test_valtan_pattern_master_v2.py'
& python $test --repository-root $repoRoot
if ($LASTEXITCODE -ne 0) {
    throw "Valtan tuning runtime-set harness failed with exit code $LASTEXITCODE."
}
