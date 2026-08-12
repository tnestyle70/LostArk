[CmdletBinding()]
param(
    [string]$RepositoryRoot = '',
    [switch]$Deep
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    $RepositoryRoot = [IO.Path]::GetFullPath($RepositoryRoot)
}

$identity = 'Tools/LevelPlacementExtractor/derive_artist_31470_main_shader_map_identity.py'
$identityTest = 'Tools/LevelPlacementExtractor/test_derive_artist_31470_main_shader_map_identity.py'
$reference = 'Tools/LevelPlacementExtractor/extract_artist_31470_main_ref_shader_cache.py'
$referenceTest = 'Tools/LevelPlacementExtractor/test_extract_artist_31470_main_ref_shader_cache.py'
$replay = 'Tools/LevelPlacementExtractor/replay_artist_31470_main_original_dxbc.py'
$replayTest = 'Tools/LevelPlacementExtractor/test_replay_artist_31470_main_original_dxbc.py'
$identityReceipt = 'Data/Effects/Imported/Artist/Materials/skill.31470.main-shader-map-identity.receipt.json'
$referenceReceipt = 'Data/Effects/Imported/Artist/Materials/skill.31470.main-ref-shader-cache.receipt.json'
$replayReceipt = 'Data/Effects/Imported/Artist/Materials/skill.31470.main-original-dxbc-replay.receipt.json'

Push-Location $RepositoryRoot
try {
	foreach ($test in @($identityTest, $referenceTest, $replayTest)) {
		$previousErrorActionPreference = $ErrorActionPreference
		$ErrorActionPreference = 'Continue'
		$unitOutput = (& python -B $test 2>&1 | Out-String).Trim()
		$unitExitCode = $LASTEXITCODE
		$ErrorActionPreference = $previousErrorActionPreference
		if ($unitExitCode -ne 0) {
			throw "Artist F main original-DXBC unit test failed: $test exit=$unitExitCode`n$unitOutput"
		}
	}

    if ($Deep) {
        # This order is the admission boundary.  The replay must never trust a
        # re-sealed shallow receipt without reconstructing both upstream joins.
        & python -B $identity '--output' $identityReceipt '--check'
        if ($LASTEXITCODE -ne 0) {
            throw "Artist F main ShaderMap identity deep check failed: $LASTEXITCODE"
        }
        & python -B $reference '--output' $referenceReceipt '--check'
        if ($LASTEXITCODE -ne 0) {
            throw "Artist F main RefShaderCache deep check failed: $LASTEXITCODE"
        }
        & python -B $replay '--output' $replayReceipt '--check'
        if ($LASTEXITCODE -ne 0) {
            throw "Artist F main original-DXBC deep replay failed: $LASTEXITCODE"
        }
    }
    else {
        & python -B $identity '--output' $identityReceipt '--validate-only'
        if ($LASTEXITCODE -ne 0) {
            throw "Artist F main ShaderMap identity shallow check failed: $LASTEXITCODE"
        }
        & python -B $reference '--output' $referenceReceipt '--validate-only'
        if ($LASTEXITCODE -ne 0) {
            throw "Artist F main RefShaderCache shallow check failed: $LASTEXITCODE"
        }
        & python -B $replay '--output' $replayReceipt '--validate-only'
        if ($LASTEXITCODE -ne 0) {
            throw "Artist F main original-DXBC shallow check failed: $LASTEXITCODE"
        }
    }

    $receipt = Get-Content -LiteralPath $replayReceipt -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $decision = $receipt.decision
    if ($receipt.schema -cne 'lostark.artist-31470-main-original-dxbc-replay-receipt' -or
        [int]$receipt.formatVersion -ne 1 -or
        [int]$receipt.numericContract.caseCount -ne 21 -or
        @($receipt.originalPixelShaders.PSObject.Properties).Count -ne 2 -or
        -not [bool]$decision.fixedInputOriginalDxbcReplayAdmission -or
        -not [bool]$decision.parameterRegisterTextureChannelClosureAdmission -or
        -not [bool]$decision.candidateOfflineEvaluatorImplementationAdmission -or
        [bool]$decision.occurrenceRuntimeEvaluatorMutationAdmission -or
        [bool]$decision.runtimeHlslMutationAdmission -or
        [bool]$decision.visualProgressAdmission) {
        throw 'Artist F main original-DXBC replay denominator or admission boundary changed.'
    }

    $mode = if ($Deep) { 'deep' } else { 'shallow' }
    Write-Output "PASS: Artist F 31470 main original DXBC mode=$mode shaders=2 cases=21 candidate=true occurrence=false hlsl=false visual=false"
}
finally {
    Pop-Location
}
