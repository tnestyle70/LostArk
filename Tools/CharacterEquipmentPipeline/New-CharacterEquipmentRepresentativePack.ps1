[CmdletBinding()]
param(
    [string]$SelectionPath,
    [string]$OutputDirectory,
    [string]$PythonPath,
    [switch]$Overwrite
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$repoRoot = [System.IO.Path]::GetFullPath(
    [System.IO.Path]::Combine($PSScriptRoot, '..', '..'))
if ([string]::IsNullOrWhiteSpace($SelectionPath)) {
    $SelectionPath = [System.IO.Path]::Combine(
        $PSScriptRoot, 'RepresentativeCharacterEquipment.json')
}
if ([string]::IsNullOrWhiteSpace($OutputDirectory)) {
    $OutputDirectory = [System.IO.Path]::Combine(
        $repoRoot, 'out', 'CharacterEquipmentExtraction', 'Admission',
        'representative-20260901-curated')
}

$scriptPath = [System.IO.Path]::Combine(
    $PSScriptRoot, 'new_character_equipment_representative_pack.py')
if (-not (Test-Path -LiteralPath $scriptPath -PathType Leaf)) {
    throw "Representative pack builder is missing: $scriptPath"
}

$pythonArguments = @()
if ([string]::IsNullOrWhiteSpace($PythonPath)) {
    $launcher = Get-Command 'py.exe' -ErrorAction SilentlyContinue
    if ($null -eq $launcher) {
        $launcher = Get-Command 'py' -ErrorAction SilentlyContinue
    }
    if ($null -ne $launcher) {
        $PythonPath = $launcher.Source
        $pythonArguments += '-3'
    }
    else {
        $python = Get-Command 'python.exe' -ErrorAction SilentlyContinue
        if ($null -eq $python) {
            $python = Get-Command 'python' -ErrorAction SilentlyContinue
        }
        if ($null -eq $python) {
            throw 'Python 3 was not found. Pass -PythonPath explicitly.'
        }
        $PythonPath = $python.Source
    }
}

$pythonPathFull = [System.IO.Path]::GetFullPath($PythonPath)
if (-not (Test-Path -LiteralPath $pythonPathFull -PathType Leaf)) {
    throw "Python executable does not exist: $pythonPathFull"
}

$arguments = @(
    $pythonArguments
    $scriptPath
    '--repo-root'
    $repoRoot
    '--selection'
    ([System.IO.Path]::GetFullPath($SelectionPath))
    '--output'
    ([System.IO.Path]::GetFullPath($OutputDirectory))
)
if ($Overwrite) {
    $arguments += '--overwrite'
}

& $pythonPathFull @arguments
if ($LASTEXITCODE -ne 0) {
    throw "Representative character-equipment pack failed with exit code $LASTEXITCODE."
}

Write-Output "RepresentativePack=$([System.IO.Path]::GetFullPath($OutputDirectory))"
