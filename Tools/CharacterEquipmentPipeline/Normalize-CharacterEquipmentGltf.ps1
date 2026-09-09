[CmdletBinding()]
param(
    [ValidateSet('Normalize', 'Cook', 'Install')]
    [string]$Mode = 'Normalize',

    [Parameter(Mandatory = $true)]
    [string]$SourceRoot,

    [Parameter(Mandatory = $true)]
    [string]$OutfitInventory,

    [Parameter(Mandatory = $true)]
    [string]$WorkRoot,

    [string]$ResourceRoot,
    [string]$ConverterPath,
    [string]$ReportPath,
    [string[]]$Class,
    [double]$InverseBindTolerance,
    [string]$PythonPath
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$repoRoot = [System.IO.Path]::GetFullPath(
    [System.IO.Path]::Combine($PSScriptRoot, '..', '..'))
if ([string]::IsNullOrWhiteSpace($ResourceRoot)) {
    $ResourceRoot = [System.IO.Path]::Combine($repoRoot, 'Client', 'Bin', 'Resources')
}

$scriptPath = [System.IO.Path]::Combine(
    $PSScriptRoot, 'normalize_character_equipment_gltf.py')
if (-not (Test-Path -LiteralPath $scriptPath -PathType Leaf)) {
    throw "Character equipment glTF normalizer is missing: $scriptPath"
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
    '--mode'
    $Mode.ToLowerInvariant()
    '--source-root'
    ([System.IO.Path]::GetFullPath($SourceRoot))
    '--outfits'
    ([System.IO.Path]::GetFullPath($OutfitInventory))
    '--resource-root'
    ([System.IO.Path]::GetFullPath($ResourceRoot))
    '--work-root'
    ([System.IO.Path]::GetFullPath($WorkRoot))
)
if (-not [string]::IsNullOrWhiteSpace($ConverterPath)) {
    $arguments += '--converter'
    $arguments += [System.IO.Path]::GetFullPath($ConverterPath)
}
if (-not [string]::IsNullOrWhiteSpace($ReportPath)) {
    $arguments += '--report'
    $arguments += [System.IO.Path]::GetFullPath($ReportPath)
}
if ($null -ne $Class -and $Class.Count -gt 0) {
    $arguments += '--classes'
    $arguments += $Class
}
if ($PSBoundParameters.ContainsKey('InverseBindTolerance')) {
    $arguments += '--inverse-bind-tolerance'
    $arguments += $InverseBindTolerance.ToString([System.Globalization.CultureInfo]::InvariantCulture)
}

& $pythonPathFull @arguments
if ($LASTEXITCODE -ne 0) {
    throw "Character equipment glTF normalization failed with exit code $LASTEXITCODE."
}
