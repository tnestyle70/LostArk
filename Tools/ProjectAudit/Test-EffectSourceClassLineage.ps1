param(
    [string]$CandidatePath = "Data/Effects/Imported/Artist/Candidates/effect.artist.skill.31470.native-v14.source-contract-candidate.effect.json",
    [string]$HarnessPath = "Tools/ClientFrontendHarness/Bin/Debug/ClientFrontendHarness.exe"
)

$ErrorActionPreference = "Stop"

$candidate = (Resolve-Path -LiteralPath $CandidatePath).Path
$harness = (Resolve-Path -LiteralPath $HarnessPath).Path
$tempRoot = Join-Path ([System.IO.Path]::GetTempPath()) (
    "artist-f-source-class-lineage-" + [guid]::NewGuid().ToString("N"))
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)

function Read-BoundDocument {
    $document = Get-Content -LiteralPath $candidate -Raw -Encoding UTF8 |
        ConvertFrom-Json
    foreach ($element in @($document.elements)) {
        $modules = @{}
        foreach ($module in @($element.sourceRecipe.modules)) {
            $modules[[string]$module.stableId] = $module
        }
        foreach ($coverage in @($element.sourceRecipe.moduleCoverage)) {
            $stableId = [string]$coverage.moduleStableId
            if (-not $modules.ContainsKey($stableId)) {
                throw "Module coverage has no source module: $stableId"
            }
            $coverage | Add-Member -NotePropertyName exactSourceClass `
                -NotePropertyValue ([string]$modules[$stableId].className) -Force
            $coverage | Add-Member -NotePropertyName aliasId `
                -NotePropertyValue "" -Force
        }
    }
    return $document
}

function Write-Document {
    param(
        [Parameter(Mandatory = $true)]$Document,
        [Parameter(Mandatory = $true)][string]$Name
    )
    $path = Join-Path $tempRoot $Name
    $json = $Document | ConvertTo-Json -Depth 100
    [System.IO.File]::WriteAllText($path, $json + "`n", $utf8NoBom)
    return $path
}

function Invoke-LoadAssertion {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][bool]$Expected,
        [Parameter(Mandatory = $true)][string]$Label
    )
    $output = (& $harness --effect-document $Path 2>&1 | Out-String).Trim()
    $accepted = 0 -eq $LASTEXITCODE
    if ($accepted -ne $Expected) {
        throw "$Label expected accepted=$Expected actual=$accepted output=$output"
    }
    Write-Host "[PASS] $Label"
}

try {
    New-Item -ItemType Directory -Path $tempRoot | Out-Null

    Invoke-LoadAssertion -Path $candidate -Expected $true `
        -Label "transitional v14 without class-lineage transport"

    $bound = Read-BoundDocument
    Invoke-LoadAssertion -Path (Write-Document $bound "bound.json") `
        -Expected $true -Label "receipt-bound exact class with empty alias"

    $missingAlias = Read-BoundDocument
    $missingAlias.elements[0].sourceRecipe.moduleCoverage[0].PSObject.Properties.Remove(
        "aliasId")
    Invoke-LoadAssertion -Path (Write-Document $missingAlias "missing-alias.json") `
        -Expected $false -Label "missing paired alias is rejected"

    $mismatchedClass = Read-BoundDocument
    $mismatchedClass.elements[0].sourceRecipe.moduleCoverage[0].exactSourceClass =
        "forgedparticlemodule"
    Invoke-LoadAssertion -Path (Write-Document $mismatchedClass "mismatch.json") `
        -Expected $false -Label "exact class mismatch is rejected"

    $aliasWithoutClass = Read-BoundDocument
    $aliasWithoutClass.elements[0].sourceRecipe.moduleCoverage[0].PSObject.Properties.Remove(
        "exactSourceClass")
    $aliasWithoutClass.elements[0].sourceRecipe.moduleCoverage[0].aliasId =
        "forged.alias"
    Invoke-LoadAssertion -Path (Write-Document $aliasWithoutClass "alias-only.json") `
        -Expected $false -Label "alias without exact class is rejected"

    $blankClass = Read-BoundDocument
    $blankClass.elements[0].sourceRecipe.moduleCoverage[0].exactSourceClass = ""
    Invoke-LoadAssertion -Path (Write-Document $blankClass "blank-class.json") `
        -Expected $false -Label "blank explicit exact class is rejected"

    Write-Host "Effect source class-lineage transport audit passed."
}
finally {
    if (Test-Path -LiteralPath $tempRoot) {
        Remove-Item -LiteralPath $tempRoot -Recurse -Force
    }
}
