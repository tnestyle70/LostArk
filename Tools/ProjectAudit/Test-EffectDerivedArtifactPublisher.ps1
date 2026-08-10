[CmdletBinding()]
param(
    [string]$RepositoryRoot = ''
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    $RepositoryRoot = [IO.Path]::GetFullPath($RepositoryRoot)
}

$builder = Join-Path $RepositoryRoot `
    'Tools\EffectPipeline\build_effect_derived_artifact.py'
$tests = Join-Path $RepositoryRoot `
    'Tools\EffectPipeline\test_build_effect_derived_artifact.py'
$publisher = Join-Path $RepositoryRoot `
    'Tools\EffectPipeline\Publish-Effects.ps1'
$schema = Join-Path $RepositoryRoot `
    'Tools\EffectPipeline\Schemas\lostark.effect-derived-artifact-contract.schema.json'

foreach ($path in @($builder, $tests, $publisher, $schema)) {
    if (-not [IO.File]::Exists($path)) {
        throw "Missing derived Effect publisher contract file: $path"
    }
}

$schemaRoot = [IO.File]::ReadAllText($schema, [Text.Encoding]::UTF8) |
    ConvertFrom-Json
if ($schemaRoot.'$schema' -cne 'https://json-schema.org/draft/2020-12/schema' -or
    $null -eq $schemaRoot.'$defs'.buildRequest -or
    $null -eq $schemaRoot.'$defs'.authoringCarrier -or
    $null -eq $schemaRoot.'$defs'.assemblyCarrier -or
    $null -eq $schemaRoot.'$defs'.compiledArtifact -or
    $null -eq $schemaRoot.'$defs'.compiledReceipt -or
    $null -eq $schemaRoot.'$defs'.runtimeCatalog) {
    throw 'Derived Effect artifact JSON schema is incomplete.'
}

$tokens = $null
$errors = $null
[void][System.Management.Automation.Language.Parser]::ParseFile(
    $publisher, [ref]$tokens, [ref]$errors)
if (@($errors).Count -ne 0) {
    throw "Publish-Effects.ps1 parse failed: $($errors[0].Message)"
}

$previousErrorActionPreference = $ErrorActionPreference
$ErrorActionPreference = 'Continue'
$output = (& python -B $tests 2>&1 | Out-String).Trim()
$testExitCode = $LASTEXITCODE
$ErrorActionPreference = $previousErrorActionPreference
if ($testExitCode -ne 0 -or $output -notmatch 'Ran 14 tests' -or
    $output -notmatch '(?m)^OK$') {
    throw "Derived Effect artifact publisher tests failed: $output"
}

Write-Output 'PASS: derived Effect artifact publisher schema tests=14 execution=true product=false'
