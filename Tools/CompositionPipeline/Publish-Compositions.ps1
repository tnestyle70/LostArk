[CmdletBinding()]
param(
    [ValidateSet('Validate', 'SyncValtanShadow', 'Publish')]
    [string]$Mode = 'Validate',
    [string]$RepositoryRoot = '',
    [string]$OutputRoot = '',
    [ValidateSet('', 'after-stage', 'after-first-promote')]
    [string]$FailAt = ''
)

$ErrorActionPreference = 'Stop'
$repoRoot = if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    [IO.Path]::GetFullPath($RepositoryRoot)
}
$pipeline = Join-Path $PSScriptRoot 'composition_pipeline.py'
if (-not [IO.File]::Exists($pipeline)) {
    throw "Missing composition pipeline: $pipeline"
}

$arguments = @('-B', $pipeline, '--repository-root', $repoRoot)
if ($Mode -eq 'Validate') {
    $arguments += 'validate'
}
elseif ($Mode -eq 'SyncValtanShadow') {
    $arguments += 'sync-valtan-shadow'
}
else {
    $arguments += 'publish'
    if (-not [string]::IsNullOrWhiteSpace($OutputRoot)) {
        $resolvedOutput = if ([IO.Path]::IsPathRooted($OutputRoot)) {
            [IO.Path]::GetFullPath($OutputRoot)
        }
        else {
            [IO.Path]::GetFullPath((Join-Path $repoRoot $OutputRoot))
        }
        $arguments += @('--output-root', $resolvedOutput)
    }
    if (-not [string]::IsNullOrWhiteSpace($FailAt)) {
        $arguments += @('--fail-at', $FailAt)
    }
}

& python @arguments
if ($LASTEXITCODE -ne 0) {
    throw "Composition pipeline failed with exit code $LASTEXITCODE"
}
