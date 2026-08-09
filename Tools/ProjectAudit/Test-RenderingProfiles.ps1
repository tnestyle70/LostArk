[CmdletBinding()]
param(
    [string]$RepoRoot = ''
)

$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrWhiteSpace($RepoRoot)) {
    $RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
}
$publisher = Join-Path $RepoRoot `
    'Tools\RenderingPipeline\Publish-RenderingProfiles.ps1'
$authoredPath = Join-Path $RepoRoot `
    'Data\Rendering\Authored\RenderingProfiles.json'
$runtimePath = Join-Path $RepoRoot `
    'Client\Bin\DataFiles\Rendering\RenderingProfiles.runtime.json'

function Invoke-Validation([string]$Path, [bool]$ShouldPass, [string]$Case) {
	$previousErrorActionPreference = $ErrorActionPreference
	$ErrorActionPreference = 'Continue'
    $output = & powershell.exe -NoProfile -ExecutionPolicy Bypass `
        -File $publisher -Mode Validate -SourcePath $Path 2>&1
    $passed = $LASTEXITCODE -eq 0
	$ErrorActionPreference = $previousErrorActionPreference
    if ($passed -ne $ShouldPass) {
        throw "$Case expected pass=$ShouldPass, actual pass=$passed. $($output -join ' ')"
    }
}

function Invoke-PublishCase(
    [string]$Path,
    [string]$Destination,
    [bool]$ShouldPass,
    [string]$Case) {
    $previousErrorActionPreference = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    $output = & powershell.exe -NoProfile -ExecutionPolicy Bypass `
        -File $publisher -Mode Publish -SourcePath $Path `
        -DestinationPath $Destination 2>&1
    $passed = $LASTEXITCODE -eq 0
    $ErrorActionPreference = $previousErrorActionPreference
    if ($passed -ne $ShouldPass) {
        throw "$Case expected pass=$ShouldPass, actual pass=$passed. $($output -join ' ')"
    }
}

function Write-TestJson([string]$Path, [object]$Value) {
    [IO.File]::WriteAllText(
        $Path,
        ($Value | ConvertTo-Json -Depth 12) + [Environment]::NewLine,
        [Text.UTF8Encoding]::new($false))
}

Invoke-Validation $authoredPath $true 'authored canonical document'
Invoke-Validation $runtimePath $true 'runtime canonical document'

$authored = Get-Content -LiteralPath $authoredPath -Raw | ConvertFrom-Json
$runtime = Get-Content -LiteralPath $runtimePath -Raw | ConvertFrom-Json
$authoredCanonical = $authored | ConvertTo-Json -Depth 12 -Compress
$runtimeCanonical = $runtime | ConvertTo-Json -Depth 12 -Compress
if ($authoredCanonical -cne $runtimeCanonical) {
    throw 'Authored and runtime rendering profiles differ semantically.'
}

$temporaryRoot = Join-Path ([IO.Path]::GetTempPath()) `
    ("lostark-rendering-profile-test-" + [guid]::NewGuid().ToString('N'))
[IO.Directory]::CreateDirectory($temporaryRoot) | Out-Null
try {
    $invalidVersion = $authored | ConvertTo-Json -Depth 12 | ConvertFrom-Json
    $invalidVersion.formatVersion = 999
    $path = Join-Path $temporaryRoot 'invalid-version.json'
    Write-TestJson $path $invalidVersion
    Invoke-Validation $path $false 'unsupported formatVersion'

    $preservedRuntime = Join-Path $temporaryRoot 'preserved-runtime.json'
    Copy-Item -LiteralPath $runtimePath -Destination $preservedRuntime
    $preservedHash = (Get-FileHash -LiteralPath $preservedRuntime `
        -Algorithm SHA256).Hash
    Invoke-PublishCase $path $preservedRuntime $false `
        'invalid publish preserves existing runtime'
    $afterRejectedPublishHash = (Get-FileHash -LiteralPath $preservedRuntime `
        -Algorithm SHA256).Hash
    if ($preservedHash -cne $afterRejectedPublishHash) {
        throw 'Rejected rendering publish changed the existing runtime document.'
    }

    $duplicate = $authored | ConvertTo-Json -Depth 12 | ConvertFrom-Json
    $duplicate.profiles = @($duplicate.profiles) + @(
        $duplicate.profiles[0] | ConvertTo-Json -Depth 12 | ConvertFrom-Json)
    $path = Join-Path $temporaryRoot 'duplicate-profile.json'
    Write-TestJson $path $duplicate
    Invoke-Validation $path $false 'duplicate profile ID'

    $duplicateFieldPath = Join-Path $temporaryRoot 'duplicate-field.json'
    $sourceText = [IO.File]::ReadAllText($authoredPath)
    $rootStart = [regex]::new('^\s*\{')
    $duplicateFieldText = $rootStart.Replace(
        $sourceText,
        '{ "schema": "lostark.rendering-profiles",',
        1)
    if ($duplicateFieldText -ceq $sourceText) {
        throw 'Duplicate-field fixture did not modify the authored document.'
    }
    [IO.File]::WriteAllText(
        $duplicateFieldPath, $duplicateFieldText,
        [Text.UTF8Encoding]::new($false))
    Invoke-Validation $duplicateFieldPath $false 'duplicate JSON object field'

    $invalidDirection = $authored | ConvertTo-Json -Depth 12 | ConvertFrom-Json
    $invalidDirection.profiles[0].light.direction = @(0.0, 0.0, 0.0, 0.0)
    $path = Join-Path $temporaryRoot 'zero-direction.json'
    Write-TestJson $path $invalidDirection
    Invoke-Validation $path $false 'zero directional light vector'

    $invalidEffective = $authored | ConvertTo-Json -Depth 12 | ConvertFrom-Json
    $invalidEffective.globalQuality.exposure = 32.0
    $invalidEffective.profiles[0].exposureMultiplier = 4.0
    $path = Join-Path $temporaryRoot 'invalid-effective.json'
    Write-TestJson $path $invalidEffective
    Invoke-Validation $path $false 'out-of-range effective exposure'

    $invalidSSAO = $authored | ConvertTo-Json -Depth 12 | ConvertFrom-Json
    $invalidSSAO.globalQuality.ssaoBias =
        $invalidSSAO.globalQuality.ssaoRadius
    $path = Join-Path $temporaryRoot 'invalid-ssao-bias.json'
    Write-TestJson $path $invalidSSAO
    Invoke-Validation $path $false 'SSAO bias greater than or equal to radius'

    $invalidShadow = $authored | ConvertTo-Json -Depth 12 | ConvertFrom-Json
    $invalidShadow.profiles[0].shadow.far =
        $invalidShadow.profiles[0].shadow.near
    $path = Join-Path $temporaryRoot 'invalid-shadow-range.json'
    Write-TestJson $path $invalidShadow
    Invoke-Validation $path $false 'shadow far less than or equal to near'

	$invalidType = $authored | ConvertTo-Json -Depth 12 | ConvertFrom-Json
	$invalidType.globalQuality.exposure = '2.0'
	$path = Join-Path $temporaryRoot 'invalid-number-type.json'
	Write-TestJson $path $invalidType
	Invoke-Validation $path $false 'string where a JSON number is required'

    $unknownField = $authored | ConvertTo-Json -Depth 12 | ConvertFrom-Json
    $unknownField.profiles[0] | Add-Member -NotePropertyName 'shadowPlaceholder' `
        -NotePropertyValue 1.0
    $path = Join-Path $temporaryRoot 'unknown-field.json'
    Write-TestJson $path $unknownField
    Invoke-Validation $path $false 'unsupported placeholder field'

    $nonFinitePath = Join-Path $temporaryRoot 'nonfinite.json'
    $numberToken = '[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?'
    $exposurePattern = [regex]::new(
        '"exposure"\s*:\s*' + $numberToken)
    $sourceText = [IO.File]::ReadAllText($authoredPath)
    $nonFiniteText = $exposurePattern.Replace(
        $sourceText, '"exposure": 1e999', 1)
    if ($nonFiniteText -ceq $sourceText) {
        throw 'Non-finite fixture did not modify the authored document.'
    }
    [IO.File]::WriteAllText(
        $nonFinitePath, $nonFiniteText, [Text.UTF8Encoding]::new($false))
    Invoke-Validation $nonFinitePath $false 'non-finite exposure'

    $wrongCasePath = Join-Path $temporaryRoot 'wrong-field-case.json'
    $sourceText = [IO.File]::ReadAllText($authoredPath)
    $wrongCaseText = $sourceText.Replace(
        '"ssaoEnabled"', '"SSAOEnabled"')
    if ($wrongCaseText -ceq $sourceText) {
        throw 'Wrong-case fixture did not modify the authored document.'
    }
    [IO.File]::WriteAllText(
        $wrongCasePath, $wrongCaseText, [Text.UTF8Encoding]::new($false))
    Invoke-Validation $wrongCasePath $false 'case-mismatched field name'

    $numericStringPath = Join-Path $temporaryRoot 'numeric-string.json'
    $ssaoRadiusPattern = [regex]::new(
        '"ssaoRadius"\s*:\s*' + $numberToken)
    $numericStringText = $ssaoRadiusPattern.Replace(
        $sourceText, '"ssaoRadius": "0.75"', 1)
    if ($numericStringText -ceq $sourceText) {
        throw 'Numeric-string fixture did not modify the authored document.'
    }
    [IO.File]::WriteAllText(
        $numericStringPath, $numericStringText,
        [Text.UTF8Encoding]::new($false))
    Invoke-Validation $numericStringPath $false 'numeric string in SSAO field'
}
finally {
    if (Test-Path -LiteralPath $temporaryRoot) {
        Remove-Item -LiteralPath $temporaryRoot -Recurse -Force
    }
}

$service = [IO.File]::ReadAllText(
    (Join-Path $RepoRoot 'Client\Private\RenderingProfileService.cpp'))
$registry = [IO.File]::ReadAllText(
    (Join-Path $RepoRoot 'Client\Private\LevelRegistry.cpp'))
$camera = [IO.File]::ReadAllText(
    (Join-Path $RepoRoot 'Client\Private\Level_CharacterSelect.cpp'))
$mainApp = [IO.File]::ReadAllText(
    (Join-Path $RepoRoot 'Client\Private\MainApp.cpp'))

foreach ($requiredToken in @(
    'OutEffective = GlobalQuality',
    'GlobalQuality.fExposure * Profile.fExposureMultiplier',
    'GlobalQuality.fBloomIntensity * Profile.fBloomIntensityMultiplier',
    'Build_ShadowDesc(Profile, stagedShadow)',
    'Apply_Shadow_Light(stagedShadow)',
    'Reload rejected: active scene profile is missing',
    'MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH')) {
    if (-not $service.Contains($requiredToken)) {
        throw "Rendering profile service contract is missing: $requiredToken"
    }
}
$characterSelectProfile = @($authored.profiles | Where-Object {
    $_.profileId -ceq 'scene.character-select.warm-high-key.v1'
})
$valtanProfile = @($authored.profiles | Where-Object {
    $_.profileId -ceq 'scene.valtan.cool-low-key.v1'
})
if ($characterSelectProfile.Count -ne 1 -or
    $characterSelectProfile[0].shadow.enabled -ne $true -or
    $valtanProfile.Count -ne 1 -or
    $valtanProfile[0].shadow.enabled -ne $true) {
    throw 'Character Select and Valtan must own distinct enabled shadow profiles.'
}
foreach ($profileId in @(
    'scene.lobby.neutral.v1',
    'scene.character-select.warm-high-key.v1',
    'scene.bern.neutral-day.v1',
    'scene.valtan.cool-low-key.v1',
    'scene.development.neutral.v1')) {
    if (-not $registry.Contains($profileId)) {
        throw "CLIENT_LEVEL_DESCRIPTOR profile mapping is missing: $profileId"
    }
}
if (($camera.Split('CharacterSelectCameraPositionOffset()').Count - 1) -lt 4 -or
    -not $camera.Contains('CHARACTER_SELECT_CAMERA_FOV_Y = 45.f') -or
    -not $camera.Contains('desc.vLookOffset = lookOffset')) {
    throw 'Character Select Server Arena does not use one fixed initial/rebind camera preset.'
}
if ($mainApp -notmatch
        'Activate_Profile\([\s\S]{0,180}LOADING_PROFILE_ID[\s\S]{0,500}Change_Level\(' -or
    $mainApp -notmatch
        'Activate_Profile\([\s\S]{0,180}pTarget->pRenderingProfileId[\s\S]{0,500}Change_Level\(' -or
    $mainApp -notmatch
        'Rendering profile rollback failed after level activation failure') {
    throw 'Level transitions must activate the staged rendering profile before replacing the level and restore it on failure.'
}

Write-Host 'Rendering profile parser/service audit PASS'
Write-Host '  authored/runtime: semantic match'
Write-Host '  failure cases: version, duplicate, non-finite, invalid, unsupported field'
Write-Host '  strict types/casing: numeric strings and case-mismatched fields rejected'
Write-Host '  publish rollback: rejected source preserves the existing runtime document'
Write-Host '  runtime: full global snapshot plus non-cumulative scene multipliers'
Write-Host '  camera: Character Select Server Arena initial/rebind preset'
Write-Host '  level transition: profile-first commit with failure rollback'
