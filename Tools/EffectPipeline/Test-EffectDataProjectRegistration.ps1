$ErrorActionPreference = 'Stop'
$repositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$syncScript = Join-Path $PSScriptRoot 'Sync-EffectDataProject.ps1'
$projectPath = Join-Path $repositoryRoot 'Client\Default\Client.vcxproj'
$filtersPath = Join-Path $repositoryRoot 'Client\Default\Client.vcxproj.filters'

& $syncScript -RepositoryRoot $repositoryRoot -Check
if ($LASTEXITCODE -ne 0) {
    throw "Effect data registration check exited with code $LASTEXITCODE."
}

Push-Location $repositoryRoot
try {
    $effectFiles = @(& git ls-files --cached --others --exclude-standard -- Data/Effects |
        Where-Object { $_ -notmatch '(?i)\.tmp(?:\.|$)' } |
        Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } |
        ForEach-Object { '..\..\' + $_.Replace('/', '\') })
}
finally {
    Pop-Location
}
$effectFiles = [string[]]@([Collections.Generic.HashSet[string]]::new(
    [string[]]$effectFiles,
    [StringComparer]::Ordinal))
[Array]::Sort($effectFiles, [StringComparer]::Ordinal)

[xml]$project = Get-Content -LiteralPath $projectPath -Raw -Encoding utf8
[xml]$filters = Get-Content -LiteralPath $filtersPath -Raw -Encoding utf8
$projectManager = [Xml.XmlNamespaceManager]::new($project.NameTable)
$projectManager.AddNamespace('m', 'http://schemas.microsoft.com/developer/msbuild/2003')
$filterManager = [Xml.XmlNamespaceManager]::new($filters.NameTable)
$filterManager.AddNamespace('m', 'http://schemas.microsoft.com/developer/msbuild/2003')

$projectIncludes = @($project.SelectNodes('//m:None', $projectManager) |
    ForEach-Object { [string]$_.Include } |
    Where-Object { $_ -like '..\..\Data\Effects\*' })
$projectIncludes = [string[]]$projectIncludes
[Array]::Sort($projectIncludes, [StringComparer]::Ordinal)
$filterItems = @($filters.SelectNodes('//m:None', $filterManager) |
    Where-Object { ([string]$_.Include) -like '..\..\Data\Effects\*' })
$definedFilters = @($filters.SelectNodes('//m:Filter[@Include]', $filterManager) |
    ForEach-Object { [string]$_.Include })

if (($projectIncludes -join "`n") -cne ($effectFiles -join "`n")) {
    throw "Client.vcxproj Effect data entries do not exactly match Git-managed files: expected=$($effectFiles.Count) actual=$($projectIncludes.Count)"
}
if ($filterItems.Count -ne $effectFiles.Count) {
    throw "Client.vcxproj.filters Effect data entry count mismatch: expected=$($effectFiles.Count) actual=$($filterItems.Count)"
}
if (@($projectIncludes | Group-Object | Where-Object Count -ne 1).Count -ne 0) {
    throw 'Client.vcxproj contains duplicate Effect data entries.'
}
if (@($projectIncludes | Where-Object { $_ -match '(?i)\.tmp(?:\.|$)' }).Count -ne 0) {
    throw 'Client.vcxproj contains a temporary Effect authoring file.'
}

foreach ($item in $filterItems) {
    $include = [string]$item.Include
    $relative = $include.Substring('..\..\'.Length)
    $directory = [IO.Path]::GetDirectoryName($relative)
    $suffix = $directory.Substring('Data\Effects'.Length).TrimStart('\')
    $expectedFilter = if ([string]::IsNullOrWhiteSpace($suffix)) {
        '96.DataFiles\Effects'
    }
    else {
        '96.DataFiles\Effects\' + $suffix
    }
    $actualFilter = [string]$item.Filter
    if ($actualFilter -cne $expectedFilter) {
        throw "Wrong Effect filter for ${include}: expected='$expectedFilter' actual='$actualFilter'"
    }
    if ($actualFilter -notin $definedFilters) {
        throw "Undefined Effect filter '$actualFilter' for $include"
    }
}

Write-Host "Effect data project registration harness PASS: files=$($effectFiles.Count)"
