param(
    [Parameter(Mandatory = $true)]
    [string]$ManifestPath,

    [string]$OutputDirectory = (Split-Path -Parent $ManifestPath),
    [string]$PackageListPath,
    [switch]$CopyMatchedOutputs,
    [string]$CopyTo = (Join-Path $OutputDirectory "matched_effect_action_outputs")
)

$ProgressPreference = "SilentlyContinue"

if (-not (Test-Path -LiteralPath $ManifestPath)) {
    throw "Manifest path not found: $ManifestPath"
}

# Windows PowerShell 5.1 has no -Depth on ConvertFrom-Json and no depth limit.
$manifest = Get-Content -Raw -LiteralPath $ManifestPath | ConvertFrom-Json
if (-not $manifest.packages) {
    throw "No package list in manifest."
}

$matchedPackages = @($manifest.packages | Where-Object { [int]$_.matchedCount -gt 0 })

# 5.1 Measure-Object takes a property name, not a script block; passing a script
# block binds it as a string and silently yields an empty sum.
$totalMatched = ($matchedPackages | Measure-Object -Property matchedCount -Sum).Sum
if ($null -eq $totalMatched) { $totalMatched = 0 }

$summary = [ordered]@{
    schemaVersion = 1
    sourceManifest = $ManifestPath
    sourceRoot = $manifest.sourceRoot
    generatedAt = (Get-Date).ToString("s")
    matchedPackageCount = $matchedPackages.Count
    totalMatchedInstances = $totalMatched
    packages = $matchedPackages
}

# Set-Content -Encoding UTF8 emits a BOM on 5.1, which corrupts the first entry
# for any UTF-8 reader downstream, so write both outputs without one.
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)

$matchedManifestPath = Join-Path $OutputDirectory "effect_action_manifest_matched.json"
[System.IO.File]::WriteAllText(
    $matchedManifestPath,
    ($summary | ConvertTo-Json -Depth 20),
    $utf8NoBom
)

if (-not $PackageListPath) {
    $PackageListPath = Join-Path $OutputDirectory "effect_action_matched_package_list.txt"
}
[System.IO.File]::WriteAllLines(
    $PackageListPath,
    [string[]]@($matchedPackages | ForEach-Object { $_.package }),
    $utf8NoBom
)

if ($CopyMatchedOutputs) {
    New-Item -ItemType Directory -Path $CopyTo -Force | Out-Null
    foreach ($pkg in $matchedPackages) {
        if (-not $pkg.outputFile) { continue }

        $outputBase = Split-Path -Parent (Resolve-Path -LiteralPath $ManifestPath).Path
        $sourceFile = Join-Path $outputBase $pkg.outputFile
        if (Test-Path -LiteralPath $sourceFile) {
            Copy-Item -LiteralPath $sourceFile -Destination (Join-Path $CopyTo (Split-Path -Leaf $sourceFile)) -Force
        }
    }
}

Write-Host "Total matched packages: $($matchedPackages.Count)"
Write-Host "Matched manifest: $matchedManifestPath"
Write-Host "Package list   : $PackageListPath"
if ($CopyMatchedOutputs) { Write-Host "Matched outputs copied to: $CopyTo" }
