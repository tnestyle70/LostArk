[CmdletBinding()]
param(
    [ValidateSet('Validate', 'Publish')]
    [string]$Mode = 'Validate',
    [string]$SourcePath = '',
    [string]$DestinationPath = ''
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
if ([string]::IsNullOrWhiteSpace($SourcePath)) {
    $SourcePath = Join-Path $repoRoot 'Data\Rendering\Authored\LightResources.json'
}
if ([string]::IsNullOrWhiteSpace($DestinationPath)) {
    $DestinationPath = Join-Path $repoRoot 'Client\Bin\DataFiles\Rendering\LightResources.runtime.json'
}
$python = Get-Command python.exe -ErrorAction SilentlyContinue
if ($null -eq $python) { $python = Get-Command python -ErrorAction SilentlyContinue }
if ($null -eq $python) { throw 'Python is required by the light resource publisher.' }
$pipeline = Join-Path $PSScriptRoot 'light_resources_pipeline.py'
if (-not (Test-Path -LiteralPath $pipeline -PathType Leaf)) {
    throw "Light resource pipeline is missing: $pipeline"
}
$previousErrorActionPreference = $ErrorActionPreference
try {
    $ErrorActionPreference = 'Continue'
    $output = & $python.Source $pipeline --mode $Mode `
        --source ([IO.Path]::GetFullPath($SourcePath)) `
        --destination ([IO.Path]::GetFullPath($DestinationPath)) 2>&1
    $exitCode = $LASTEXITCODE
}
finally { $ErrorActionPreference = $previousErrorActionPreference }
if ($exitCode -ne 0) { throw "Light resource $Mode failed: $($output -join ' ')" }
$output | ForEach-Object { Write-Output $_ }
