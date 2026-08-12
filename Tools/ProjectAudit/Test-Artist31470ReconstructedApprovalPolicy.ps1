[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$previousBytecode = $env:PYTHONDONTWRITEBYTECODE
$env:PYTHONDONTWRITEBYTECODE = '1'

try {
    $unitRunner = @'
import sys
import unittest

suite = unittest.defaultTestLoader.loadTestsFromName(
    "Tools.EffectPipeline.test_build_reconstructed_effect_approval_policy"
)
result = unittest.TextTestRunner(stream=sys.stdout, verbosity=1).run(suite)
raise SystemExit(0 if result.wasSuccessful() else 1)
'@
    $unitOutput = ($unitRunner | & python -B - | Out-String).Trim()
    if ($LASTEXITCODE -ne 0) {
        throw "reconstructed approval policy unit tests failed:`n$unitOutput"
    }
    if ($unitOutput -notmatch 'Ran 41 tests' -or
        $unitOutput -notmatch '(?m)^OK$') {
        throw "unexpected reconstructed policy unit-test denominator:`n$unitOutput"
    }

    $generatorOutput = (& python -B `
        'Tools/EffectPipeline/build_reconstructed_effect_approval_policy.py' `
        '--check' 2>&1 | Out-String).Trim()
    if ($LASTEXITCODE -ne 0) {
        throw "reconstructed approval policy generator check failed:`n$generatorOutput"
    }
    if ($generatorOutput -notmatch
        'source=29 material=260 sampler=77 geometry=7 sourceExact=false execution=false product=false') {
        throw "unexpected reconstructed policy generator summary: $generatorOutput"
    }

    $receiptPath =
        'Data/Effects/Policies/Artist/artist.31470.f.reconstructed-approved-v1.policy.receipt.json'
    $schemaPath =
        'Tools/EffectPipeline/Schemas/lostark.effect-reconstruction-approval-policy.schema.json'
    $sourcePath =
        'Data/Effects/Policies/Artist/artist.31470.f.reconstructed-approved-v1.policy-source.json'
    foreach ($path in @($receiptPath, $schemaPath, $sourcePath)) {
        if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
            throw "reconstructed policy artifact missing: $path"
        }
        $null = Get-Content -LiteralPath $path -Raw -Encoding UTF8 |
            ConvertFrom-Json
    }

    $receipt = Get-Content -LiteralPath $receiptPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    if ($receipt.schema -cne 'lostark.effect-reconstruction-approval-policy' -or
        [int]$receipt.formatVersion -ne 1 -or
        @($receipt.sourceRows).Count -ne 29 -or
        @($receipt.materialRows).Count -ne 260 -or
        @($receipt.materialArithmeticRows).Count -ne 23 -or
        @($receipt.geometryRows).Count -ne 7 -or
        [int]$receipt.summary.materialRenderStateRowCount -ne 89 -or
        [int]$receipt.summary.materialStaticPermutationRowCount -ne 94 -or
        [int]$receipt.summary.materialSamplerRowCount -ne 77 -or
        [int]$receipt.summary.formerExactSamplerReauditCount -ne 4 -or
        [int]$receipt.summary.formerExactSamplerPartialTagsCount -ne 1 -or
        [int]$receipt.summary.formerExactSamplerDefaultUnprovenCount -ne 3 -or
        [int]$receipt.summary.samplerFullSourceExactCount -ne 0 -or
        [int]$receipt.summary.forbiddenFullFidelityLabelCount -ne 0 -or
        [bool]$receipt.fidelityPolicy.sourceExactAdmission -or
        [bool]$receipt.admissionPolicy.executionAdmission -or
        [bool]$receipt.admissionPolicy.productAdmission -or
        [int]$receipt.manualValidation.completedOccurrenceCount -ne 0 -or
        [bool]$receipt.manualValidation.automatedScreenshotOrImageOracleAllowed) {
        throw 'reconstructed approval policy is not fail-closed at 29/260/77/23/7'
    }

    Write-Output (
        'PASS: Artist F 31470 reconstructed approval policy ' +
        'tests=41 source=29 material=260 sampler=77 arithmetic=23 geometry=7 ' +
        'sourceExact=false execution=false product=false')
}
finally {
    $env:PYTHONDONTWRITEBYTECODE = $previousBytecode
}
