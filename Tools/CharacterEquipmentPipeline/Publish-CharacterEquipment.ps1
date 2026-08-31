[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateSet('Validate', 'Publish')]
    [string]$Mode,

    [Parameter(Mandatory = $true)]
    [string]$AdmissionRoot,

    [Parameter(Mandatory = $true)]
    [string]$ResourceRoot,

    [string]$PythonPath
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$scriptPath = [System.IO.Path]::Combine(
    $PSScriptRoot, 'publish_character_equipment.py')
if (-not (Test-Path -LiteralPath $scriptPath -PathType Leaf)) {
    throw "Character equipment publisher is missing: $scriptPath"
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

$admissionRootFull = [System.IO.Path]::GetFullPath($AdmissionRoot)
$resourceRootFull = [System.IO.Path]::GetFullPath($ResourceRoot)
$arguments = @(
    $pythonArguments
    $scriptPath
    '--mode'
    $Mode
    '--admission-root'
    $admissionRootFull
    '--resource-root'
    $resourceRootFull
)

& $pythonPathFull @arguments
if ($LASTEXITCODE -ne 0) {
    throw "Character equipment publisher failed with exit code $LASTEXITCODE."
}
