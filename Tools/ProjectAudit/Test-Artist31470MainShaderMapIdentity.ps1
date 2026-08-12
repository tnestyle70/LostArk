[CmdletBinding()]
param(
    [string]$RepositoryRoot = '',
    [switch]$Deep,
    [string]$SourcePackageRoot = '',
    [string]$EFEnginePath = ''
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    $RepositoryRoot = [IO.Path]::GetFullPath($RepositoryRoot)
}

$generator = 'Tools/LevelPlacementExtractor/derive_artist_31470_main_shader_map_identity.py'
$test = 'Tools/LevelPlacementExtractor/test_derive_artist_31470_main_shader_map_identity.py'
$receiptPath = 'Data/Effects/Imported/Artist/Materials/skill.31470.main-shader-map-identity.receipt.json'

Push-Location $RepositoryRoot
try {
	$previousErrorActionPreference = $ErrorActionPreference
	$ErrorActionPreference = 'Continue'
	$unitOutput = (& python -B $test 2>&1 | Out-String).Trim()
	$unitExitCode = $LASTEXITCODE
	$ErrorActionPreference = $previousErrorActionPreference
	if ($unitExitCode -ne 0) {
		throw "Artist F main ShaderMap identity unit tests failed: $unitExitCode`n$unitOutput"
	}

    if ($Deep) {
        if ([string]::IsNullOrWhiteSpace($SourcePackageRoot) -or
            -not (Test-Path -LiteralPath $SourcePackageRoot -PathType Container) -or
            [string]::IsNullOrWhiteSpace($EFEnginePath) -or
            -not (Test-Path -LiteralPath $EFEnginePath -PathType Leaf)) {
            throw 'Deep Artist F main ShaderMap identity audit requires source package root and EFEngine path.'
        }
        & python -B $generator `
            '--source-root' $SourcePackageRoot `
            '--efengine' $EFEnginePath `
            '--output' $receiptPath `
            '--check'
    }
    else {
        & python -B $generator '--output' $receiptPath '--validate-only'
    }
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F main ShaderMap identity receipt check failed: $LASTEXITCODE"
    }

    $receipt = Get-Content -LiteralPath $receiptPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $decision = $receipt.decision
    $abi = $receipt.currentInstalledBinaryAbi.abiConclusion
    $boundary = $receipt.centralInstalledCacheMapSerialBoundary
    if ($receipt.schema -cne 'lostark.artist-31470-main-shader-map-identity-receipt' -or
        [int]$receipt.formatVersion -ne 2 -or
        @($receipt.mainTargets).Count -ne 2 -or
        (@($receipt.mainTargets | ForEach-Object { @($_.occurrenceIds).Count }) |
            Measure-Object -Sum).Sum -ne 3 -or
        $abi.lookupKeyType -cne 'FStaticParameterSet' -or
        $abi.platformSelectionType -cne 'EShaderPlatform' -or
        [bool]$abi.separate16ByteMaterialShaderMapIdSupported -or
        [int]$boundary.materialShaderMapCount -ne 25 -or
        [int]$boundary.absoluteMapEndEqualityCount -ne 25 -or
        -not [bool]$decision.identityDerivationImplemented -or
        [int]$decision.exactSameRevisionShaderMapJoinCount -ne 0 -or
        [int]$decision.dxbcReplayCount -ne 0 -or
        [bool]$decision.runtimeHlslMutationAdmission -or
        [bool]$decision.visualProgressAdmission) {
        throw 'Artist F main ShaderMap identity denominator or admission boundary changed.'
    }

    $mode = if ($Deep) { 'deep' } else { 'shallow' }
    Write-Output "PASS: Artist F 31470 main ShaderMap identity mode=$mode families=2 occurrences=3 key=FStaticParameterSet platform=4 map-end=25/25 join=0 dxbc=0 hlsl=false visual=false"
}
finally {
    Pop-Location
}
