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

$previousBytecode = $env:PYTHONDONTWRITEBYTECODE
$env:PYTHONDONTWRITEBYTECODE = '1'
Push-Location $RepositoryRoot
try {
    $unitRunner = @'
import sys
import unittest

suite = unittest.defaultTestLoader.loadTestsFromName(
    "Tools.EffectPipeline.test_build_artist_31470_reconstructed_runtime_program"
)
result = unittest.TextTestRunner(stream=sys.stdout, verbosity=1).run(suite)
raise SystemExit(0 if result.wasSuccessful() else 1)
'@
    $unitOutput = ($unitRunner | & python -B - | Out-String).Trim()
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F reconstructed runtime program tests failed:`n$unitOutput"
    }
    if ($unitOutput -notmatch 'Ran 12 tests' -or
        $unitOutput -notmatch '(?m)^OK$') {
        throw "unexpected runtime program unit-test denominator:`n$unitOutput"
    }

    $generatorOutput = (& python -B `
        'Tools/EffectPipeline/build_artist_31470_reconstructed_runtime_program.py' `
        '--check' 2>&1 | Out-String).Trim()
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F reconstructed runtime program check failed:`n$generatorOutput"
    }
    if ($generatorOutput -notmatch
        'CHECK PASS bytes=[0-9]+ sha=[0-9a-f]{64} runtime=false product=false') {
        throw "unexpected runtime program generator summary: $generatorOutput"
    }

    $candidatePath =
        'Data/Effects/Imported/Artist/Candidates/skill.31470.reconstructed-runtime-program.candidate.json'
    $candidate = Get-Content -LiteralPath $candidatePath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $summary = $candidate.summary
    if ($candidate.schema -cne 'lostark.artist-31470-reconstructed-runtime-program' -or
        [int]$candidate.formatVersion -ne 1 -or
        @($candidate.inputArtifacts).Count -ne 13 -or
        [int]$summary.emitterCount -ne 35 -or
        [int]$summary.actionScheduleCount -ne 7 -or
        [int]$summary.moduleCount -ne 399 -or
        [int]$summary.propertyCount -ne 1434 -or
        [int]$summary.primitiveLeafCount -ne 1572 -or
        [int]$summary.literalCount -ne 1590 -or
        [int]$summary.distributionCount -ne 629 -or
        [int]$summary.seedPolicyCount -ne 14 -or
        [int]$summary.implicitDefaultCount -ne 14 -or
        [int]$summary.pointLightFieldCount -ne 8 -or
        [int]$summary.materialFamilyCount -ne 23 -or
        [int]$summary.materialRecipeCount -ne 27 -or
        [int]$summary.materialOccurrenceCount -ne 34 -or
        [int]$summary.materialPolicyCount -ne 255 -or
        [int]$summary.materialTextureBindingCount -ne 72 -or
        [int]$summary.resolvedMaterialTextureBindingCount -ne 68 -or
        [int]$summary.rendererTextureResourceCount -ne 57 -or
        [int]$summary.geometryCarrierCount -ne 7 -or
        [int]$summary.geometryUseCount -ne 13 -or
        [int]$summary.sourceExactRowCount -ne 0 -or
        [bool]$candidate.admission.sourceExact -or
        [bool]$candidate.admission.runtimeExecution -or
        [bool]$candidate.admission.product -or
        @($candidate.blockerUnion) -contains
            'DECAL_YAW_ONLY_CDO_DEFAULT_CAPABILITY_OUTPUT_CONFLICT' -or
        @($candidate.blockerUnion) -notcontains 'R5_DECAL_RUNTIME_PROBE_NOT_COMPLETE') {
        throw 'Artist F reconstructed runtime program denominator/admission mismatch'
    }

    Write-Output (
        'PASS: Artist F 31470 reconstructed runtime program tests=12 ' +
        'emitters=35 schedules=7 modules=399 properties=1434 leaves=1572 ' +
        'distributions=629 material=23/27/34/255 textures=68/72+57 ' +
        'geometry=7/13 sourceExact=0 runtime=false product=false')
}
finally {
    Pop-Location
    $env:PYTHONDONTWRITEBYTECODE = $previousBytecode
}
