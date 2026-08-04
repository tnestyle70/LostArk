[CmdletBinding()]
param(
    [switch]$DeepAssetHash,
    [string]$ReportPath = '.codex_tmp/ProjectAudit.json'
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Get-Location).Path)
$checks = [Collections.Generic.List[object]]::new()
$failures = [Collections.Generic.List[string]]::new()

function Add-Check {
    param(
        [string]$Name,
        [bool]$Passed,
        [string]$Detail
    )

    $checks.Add([ordered]@{
        name = $Name
        passed = $Passed
        detail = $Detail
    })
    if (-not $Passed) {
        $failures.Add("$Name`: $Detail")
    }
}

function Read-Json {
    param([string]$Path)
	return Get-Content -LiteralPath $Path -Raw -Encoding UTF8 | ConvertFrom-Json
}

function Get-ProjectItems {
    param([string]$ProjectPath)

    [xml]$project = Get-Content -LiteralPath $ProjectPath
    $manager = [Xml.XmlNamespaceManager]::new($project.NameTable)
    $manager.AddNamespace('m', 'http://schemas.microsoft.com/developer/msbuild/2003')
    return @($project.SelectNodes('//m:ClInclude|//m:ClCompile|//m:FxCompile|//m:None', $manager))
}

function Get-WModelTextureReferences {
    param([string]$Path)

    $decoded = [Text.Encoding]::Unicode.GetString(
        [IO.File]::ReadAllBytes($Path))
    return @([regex]::Matches(
        $decoded,
        '(?:Resource|Resources)[\\/][^\x00\r\n]{1,259}?\.(?:dds|png|tga|jpg|jpeg|bmp)',
        [Text.RegularExpressions.RegexOptions]::IgnoreCase) |
        ForEach-Object { $_.Value })
}

try {
    $resourceRoot = Join-Path $repoRoot 'Client\Bin\Resources'
    $expectedDomains = @('Character', 'Deploy', 'Effect', 'Fonts', 'Map', 'UI')
    $actualDomains = @(Get-ChildItem -LiteralPath $resourceRoot -Force |
        ForEach-Object Name | Sort-Object)
    Add-Check 'resource.top-level' (($actualDomains -join "`n") -eq (($expectedDomains | Sort-Object) -join "`n")) "actual=$($actualDomains -join ',')"

    $forbiddenResourceNames = @('LostArk', 'Models', 'Textures', 'SourceData', 'Sound')
    $presentForbidden = @($forbiddenResourceNames | Where-Object {
        Test-Path -LiteralPath (Join-Path $resourceRoot $_) })
    Add-Check 'resource.forbidden-roots' ($presentForbidden.Count -eq 0) "present=$($presentForbidden -join ',')"

    $forbiddenRuntimeFiles = @(Get-ChildItem -LiteralPath $resourceRoot -Recurse -File -Force |
        Where-Object { $_.Extension.ToLowerInvariant() -in @('.cfg', '.exe', '.dll', '.pdb') })
    Add-Check 'resource.forbidden-files' ($forbiddenRuntimeFiles.Count -eq 0) "count=$($forbiddenRuntimeFiles.Count)"

    $lock = Read-Json 'Data\AssetPacks.lock.json'
    $pack = @($lock.packs | Where-Object packId -eq 'lostark-resources')
    Add-Check 'asset-lock.identity' ($lock.schema -eq 'lostark.asset-pack-lock' -and $lock.formatVersion -eq 1 -and $pack.Count -eq 1) "packs=$($pack.Count)"
    if ($pack.Count -eq 1) {
        $manifest = Read-Json $pack[0].manifest
        $actualFileCount = @(Get-ChildItem -LiteralPath $resourceRoot -Recurse -File -Force).Count
        $actualBytes = [long]((Get-ChildItem -LiteralPath $resourceRoot -Recurse -File -Force |
            Measure-Object Length -Sum).Sum)
        Add-Check 'asset-lock.inventory' ($actualFileCount -eq $manifest.fileCount -and $actualBytes -eq $manifest.totalBytes) "files=$actualFileCount bytes=$actualBytes"
        $manifestHash = (Get-FileHash -LiteralPath $pack[0].manifest -Algorithm SHA256).Hash.ToLowerInvariant()
        Add-Check 'asset-lock.manifest-hash' ($manifestHash -eq $pack[0].manifestSha256) $manifestHash
    }

    if ($DeepAssetHash) {
        $verifyOutput = & .\Tools\AssetPipeline\Manage-ResourcePack.ps1 -Mode Verify 2>&1
        Add-Check 'asset-lock.deep-verify' $true ($verifyOutput -join ' ')
    }

    $snapshotRollbackPassed = $false
    $snapshotRollbackDetail = ''
    $snapshotFixtureRoot = Join-Path $repoRoot ".codex_tmp\ResourceSnapshotFixture-$PID"
    $resourcePackScript = Join-Path $repoRoot 'Tools\AssetPipeline\Manage-ResourcePack.ps1'
    try {
        $snapshotResourceRoot = Join-Path $snapshotFixtureRoot 'Client\Bin\Resources'
        foreach ($domain in @('Character', 'Deploy', 'Effect', 'Fonts', 'Map', 'UI')) {
            [IO.Directory]::CreateDirectory((Join-Path $snapshotResourceRoot $domain)) | Out-Null
        }
        [IO.File]::WriteAllText(
            (Join-Path $snapshotResourceRoot 'Fonts\fixture.asset'),
            'fixture',
            [Text.UTF8Encoding]::new($false))
        $snapshotLockPath = Join-Path $snapshotFixtureRoot 'Data\AssetPacks.lock.json'
        [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($snapshotLockPath)) | Out-Null
        [IO.File]::WriteAllText(
            $snapshotLockPath,
            "{`"schema`":`"lostark.asset-pack-lock`",`"formatVersion`":1,`"packs`":[]}",
            [Text.UTF8Encoding]::new($false))

        Push-Location $snapshotFixtureRoot
        try {
            $injectedFailureRejected = $false
            try {
                & $resourcePackScript `
                    -Mode Snapshot `
                    -PackId fixture-resources `
                    -Version 1.0.0 `
                    -FailureAfterManifest | Out-Null
            }
            catch {
                $injectedFailureRejected = $true
            }
            $orphanAfterFailure = @(Get-ChildItem -LiteralPath 'Data\AssetManifests' -File -ErrorAction SilentlyContinue)
            $temporaryAfterFailure = @(Get-ChildItem -LiteralPath 'Data' -Recurse -Force -ErrorAction SilentlyContinue |
                Where-Object Name -Match '\.tmp$|\.replace-backup$')
            $lockAfterFailure = Get-Content -LiteralPath 'Data\AssetPacks.lock.json' -Raw | ConvertFrom-Json

            & $resourcePackScript `
                -Mode Snapshot `
                -PackId fixture-resources `
                -Version 1.0.0 | Out-Null
            & $resourcePackScript `
                -Mode Verify `
                -PackId fixture-resources | Out-Null
            $manifestAfterSuccess = @(Get-ChildItem -LiteralPath 'Data\AssetManifests' -File)
            $lockAfterSuccess = Get-Content -LiteralPath 'Data\AssetPacks.lock.json' -Raw | ConvertFrom-Json
            $snapshotRollbackPassed =
                $injectedFailureRejected -and
                $orphanAfterFailure.Count -eq 0 -and
                $temporaryAfterFailure.Count -eq 0 -and
                @($lockAfterFailure.packs).Count -eq 0 -and
                $manifestAfterSuccess.Count -eq 1 -and
                @($lockAfterSuccess.packs).Count -eq 1
            $snapshotRollbackDetail =
                "rejected=$injectedFailureRejected orphan=$($orphanAfterFailure.Count) temp=$($temporaryAfterFailure.Count)"
        }
        finally {
            Pop-Location
        }
    }
    catch {
        $snapshotRollbackDetail = $_.Exception.Message
    }
    finally {
        if ([IO.Directory]::Exists($snapshotFixtureRoot)) {
            Remove-Item -LiteralPath $snapshotFixtureRoot -Recurse -Force
        }
    }
    Add-Check 'asset-lock.snapshot-rollback' $snapshotRollbackPassed $snapshotRollbackDetail

    $uiDocuments = @(
        'Data\UI\HUD\HUD_Layout.json',
        'Data\UI\ScreenUI\ScreenUI.json')
    $totalSlots = 0
    $uiValid = $true
    foreach ($uiPath in $uiDocuments) {
        $ui = Read-Json $uiPath
        $slotIds = @($ui.slots | ForEach-Object id)
        $duplicates = @($slotIds | Group-Object | Where-Object Count -gt 1)
        $badPaths = @($ui.slots | ForEach-Object {
            $_.layers.path
            $_.layers.hoverPath
            $_.shine.texture
            $_.animation.frames
        } | Where-Object { $_ -and $_ -notlike 'UI/*' })
        $uiValid = $uiValid -and $ui.schema -eq 'lostark.ui-layout' -and
            $ui.formatVersion -eq 1 -and $duplicates.Count -eq 0 -and
            $badPaths.Count -eq 0 -and @($ui.slots).Count -gt 0
        $totalSlots += @($ui.slots).Count
    }
    Add-Check 'ui.json-contract' $uiValid "documents=$($uiDocuments.Count) slots=$totalSlots"

    $mapCatalog = Read-Json 'Data\Maps\MapCatalog.json'
    $mapRoot = 'Client\Bin\DataFiles\Map'
    $missingMapFiles = [Collections.Generic.List[string]]::new()
    $invalidMapPaths = [Collections.Generic.List[string]]::new()
    foreach ($area in $mapCatalog.areas) {
        foreach ($requiredProperty in @('sourceCatalog', 'sourcePlacements', 'catalog')) {
            if ($area.PSObject.Properties.Name -notcontains $requiredProperty) {
                $invalidMapPaths.Add("$($area.id):missing-$requiredProperty")
            }
        }
        foreach ($property in @(
            'sourceCatalog', 'sourcePlacements',
            'sourceDeployCatalog', 'sourceDeployPlacements',
            'catalog', 'placements', 'deployCatalog', 'deployPlacements',
            'navigationSource', 'navigationPaint', 'navigationBlockers',
            'navigationRuntime',
            'gameplayDocument')) {
            $isCharacterSelectNavBootstrap =
                $area.id -eq 'LV_LOBBY_CLASSSELECT_SL00' -and
                $property -in @('navigationSource', 'navigationPaint')
            if ($area.PSObject.Properties.Name -contains $property -and
                -not $isCharacterSelectNavBootstrap -and
                -not (Test-Path -LiteralPath $area.$property)) {
                $missingMapFiles.Add($area.$property)
            }
        }
        foreach ($property in @(
            'sourceCatalog', 'sourcePlacements',
            'sourceDeployCatalog', 'sourceDeployPlacements')) {
            if ($area.PSObject.Properties.Name -contains $property -and
                -not ([string]$area.$property).StartsWith('Data/Maps/', [StringComparison]::Ordinal)) {
                $invalidMapPaths.Add("$($area.id):$property")
            }
        }
        foreach ($property in @('catalog', 'placements', 'deployCatalog', 'deployPlacements')) {
            if ($area.PSObject.Properties.Name -contains $property -and
                -not ([string]$area.$property).StartsWith('Client/Bin/DataFiles/Map/', [StringComparison]::Ordinal)) {
                $invalidMapPaths.Add("$($area.id):$property")
            }
        }
        foreach ($property in @('navigationSource', 'navigationPaint', 'navigationBlockers')) {
            if ($area.PSObject.Properties.Name -contains $property -and
                -not ([string]$area.$property).StartsWith('Data/Navigation/', [StringComparison]::Ordinal)) {
                $invalidMapPaths.Add("$($area.id):$property")
            }
        }
        if ($area.PSObject.Properties.Name -contains 'navigationRuntime' -and
            -not ([string]$area.navigationRuntime).StartsWith(
                'Client/Bin/DataFiles/Navigation/', [StringComparison]::Ordinal)) {
            $invalidMapPaths.Add("$($area.id):navigationRuntime")
        }
    }
    $mapDataNames = @(Get-ChildItem -LiteralPath $mapRoot -File | ForEach-Object Name)
    $legacyMapFiles = @($mapDataNames | Where-Object { $_ -like 'BG_RAD_VALTAN_A*' })
	Add-Check 'maps.catalog' ($mapCatalog.schema -eq 'lostark.map-catalog' -and
		$missingMapFiles.Count -eq 0 -and $invalidMapPaths.Count -eq 0 -and
		$legacyMapFiles.Count -eq 0) "missing=$($missingMapFiles.Count) invalidPaths=$($invalidMapPaths.Count) legacy=$($legacyMapFiles.Count)"
	$editorAreas = @{}
	foreach ($area in $mapCatalog.areas) { $editorAreas[[string]$area.id] = $area }
	$characterSelectEditor = $editorAreas['LV_LOBBY_CLASSSELECT_SL00']
	$bernEditor = $editorAreas['LV_BER_BERNCASTLE']
	$valtanEditor = $editorAreas['LV_LUT_HEARTRB_ED']
	$trainingEditor = $editorAreas['LV_SHS_RCARENA_D']
	$mapToolSource = Get-Content 'Client\Private\MapTool.cpp' -Raw
	$loaderSource = Get-Content 'Client\Private\Loader.cpp' -Raw
	$mainAppSource = Get-Content 'Client\Private\MainApp.cpp' -Raw
	$lobbySource = Get-Content 'Client\Private\Level_Lobby.cpp' -Raw
	$editorPoliciesValid =
		$null -ne $characterSelectEditor -and
		$characterSelectEditor.navigationSource -eq 'Data/Navigation/LV_LOBBY_CLASSSELECT_SL00.navsource' -and
		$characterSelectEditor.navigationPaint -eq 'Data/Navigation/LV_LOBBY_CLASSSELECT_SL00.navpaint' -and
		$characterSelectEditor.PSObject.Properties.Name -notcontains 'gameplayDocument' -and
		$null -ne $bernEditor -and
		$bernEditor.PSObject.Properties.Name -contains 'gameplayDocument' -and
		$bernEditor.PSObject.Properties.Name -notcontains 'navigationSource' -and
		$null -ne $valtanEditor -and
		$valtanEditor.PSObject.Properties.Name -contains 'navigationSource' -and
		$valtanEditor.PSObject.Properties.Name -contains 'navigationPaint' -and
		$valtanEditor.PSObject.Properties.Name -contains 'navigationBlockers' -and
		$valtanEditor.PSObject.Properties.Name -contains 'gameplayDocument' -and
		$null -ne $trainingEditor -and
		$trainingEditor.PSObject.Properties.Name -notcontains 'navigationSource' -and
		$trainingEditor.PSObject.Properties.Name -notcontains 'gameplayDocument' -and
		$mapToolSource -match 'Load_Source\(' -and
		$mapToolSource -notmatch '\.Export_Runtime\(' -and
		$mapToolSource -match 'MapEditorWorkspaceService::Is_Active' -and
		$loaderSource -match 'Ready_MapAuthoringCore' -and
		$loaderSource -match 'MapEditorWorkspaceService::Is_Requested' -and
		$mainAppSource -notmatch 'debug\.map-editor' -and
		$lobbySource -match 'MapEditorWorkspaceService::Request\(' -and
		$lobbySource -match 'LEVEL::DEVELOPMENT == approvedLevel'
	Add-Check 'maps.editor-workspace-policy' $editorPoliciesValid 'Lobby Test owns editor entry; F1 only toggles tools; four exact editor Areas; Data-only save'
	$singleAreaContractErrors = [Collections.Generic.List[string]]::new()
	$exclusiveRuntimeAreaIds = @(
		'LV_LOBBY_CLASSSELECT_SL00',
		'LV_SHS_RCARENA_D'
	)
	$exclusiveRuntimeAreas = @($mapCatalog.areas | Where-Object {
		$_.id -in $exclusiveRuntimeAreaIds
	})
	if ($exclusiveRuntimeAreas.Count -ne $exclusiveRuntimeAreaIds.Count) {
		$singleAreaContractErrors.Add('missing extracted single-area contract')
	}
	foreach ($area in $exclusiveRuntimeAreas) {
		$catalogLines = @(Get-Content -LiteralPath $area.catalog -Encoding utf8)
		$catalogHeaderMatch = if ($catalogLines.Count -gt 0) {
			[regex]::Match($catalogLines[0], '^LOSTARK_MAP_ASSET_CATALOG\s+\d+\s+"(?<Area>[^"]+)"\s+(?<Count>\d+)$')
		} else { $null }
		if ($null -eq $catalogHeaderMatch -or -not $catalogHeaderMatch.Success -or
			$catalogHeaderMatch.Groups['Area'].Value -ne $area.id -or
			($area.PSObject.Properties.Name -contains 'assetCount' -and
				[int]$catalogHeaderMatch.Groups['Count'].Value -ne [int]$area.assetCount)) {
			$singleAreaContractErrors.Add("$($area.id): catalog header/count")
			continue
		}

		$runtimePrefix = [string]$area.runtimeAssetRoot
		$runtimePrefix = $runtimePrefix.Replace('\', '/').TrimEnd('/')
		$modelPaths = [Collections.Generic.List[string]]::new()
		$catalogAssetKeys = [Collections.Generic.List[string]]::new()
		foreach ($line in @($catalogLines | Select-Object -Skip 1)) {
			if ($line -notmatch '^"(?<Asset>[^"]+)"\s+"[^"]+"\s+"(?<Model>[^"]+)"') {
				$singleAreaContractErrors.Add("$($area.id): malformed asset row")
				continue
			}
			$assetId = $Matches.Asset
			$modelPath = $Matches.Model.Replace('\', '/')
			$modelPaths.Add($modelPath)
			if (-not $modelPath.StartsWith($runtimePrefix + '/', [StringComparison]::OrdinalIgnoreCase)) {
				$singleAreaContractErrors.Add("$($area.id): model outside runtimeAssetRoot")
			} else {
				$relativeModel = $modelPath.Substring($runtimePrefix.Length + 1)
				$catalogAssetKeys.Add("$assetId|$relativeModel")
			}
		}

		if ($area.PSObject.Properties.Name -contains 'placements') {
			$placementHeader = Get-Content -LiteralPath $area.placements -Encoding utf8 -TotalCount 1
			$placementHeaderMatch = [regex]::Match($placementHeader, '^LOSTARK_MAP_PLACEMENTS\s+\d+\s+"(?<Area>[^"]+)"\s+(?<Count>\d+)$')
			if (-not $placementHeaderMatch.Success -or
				$placementHeaderMatch.Groups['Area'].Value -ne $area.id -or
				($area.PSObject.Properties.Name -contains 'placementCount' -and
					[int]$placementHeaderMatch.Groups['Count'].Value -ne [int]$area.placementCount)) {
				$singleAreaContractErrors.Add("$($area.id): placement header/count")
			}
		}

		$runtimeDirectory = Join-Path 'Client\Bin\Resources' $runtimePrefix.Replace('/', '\')
		if (Test-Path -LiteralPath $runtimeDirectory -PathType Container) {
			foreach ($modelPath in $modelPaths) {
				$modelFile = Join-Path 'Client\Bin\Resources' $modelPath.Replace('/', '\')
				if (-not (Test-Path -LiteralPath $modelFile -PathType Leaf)) {
					$singleAreaContractErrors.Add("$($area.id): missing runtime model")
					continue
				}

				$textureReferences = @(Get-WModelTextureReferences $modelFile)
				if ($textureReferences.Count -eq 0) {
					$singleAreaContractErrors.Add("$($area.id): model has no material texture")
					continue
				}
				foreach ($textureReference in $textureReferences) {
					if ($textureReference.Contains('\') -or
						$textureReference.Contains(':') -or
						-not $textureReference.StartsWith('Resource/', [StringComparison]::Ordinal)) {
						$singleAreaContractErrors.Add("$($area.id): invalid material texture path")
						continue
					}

					$relativeTexture = $textureReference.Substring('Resource/'.Length)
					$invalidSegments = @($relativeTexture.Split('/') | Where-Object {
						[string]::IsNullOrWhiteSpace($_) -or $_ -eq '.' -or $_ -eq '..'
					})
					if ($invalidSegments.Count -gt 0 -or
						-not $relativeTexture.StartsWith($runtimePrefix + '/', [StringComparison]::OrdinalIgnoreCase)) {
						$singleAreaContractErrors.Add("$($area.id): material outside runtimeAssetRoot")
						continue
					}

					try {
						$textureFile = [IO.Path]::GetFullPath((Join-Path $resourceRoot $relativeTexture.Replace('/', '\')))
					} catch {
						$singleAreaContractErrors.Add("$($area.id): invalid material texture path")
						continue
					}
					$resourcePrefix = $resourceRoot + [IO.Path]::DirectorySeparatorChar
					if (-not $textureFile.StartsWith($resourcePrefix, [StringComparison]::OrdinalIgnoreCase) -or
						-not (Test-Path -LiteralPath $textureFile -PathType Leaf)) {
						$singleAreaContractErrors.Add("$($area.id): unresolved material texture")
					}
				}
			}
			$runtimeManifestPath = Join-Path $runtimeDirectory 'map_asset_runtime_manifest.json'
			if (Test-Path -LiteralPath $runtimeManifestPath -PathType Leaf) {
				$runtimeManifest = Read-Json $runtimeManifestPath
				$runtimeManifestKeys = @($runtimeManifest.assets | ForEach-Object {
					"$($_.assetId)|$(([string]$_.model).Replace('\', '/'))"
				} | Sort-Object)
				$catalogKeys = @($catalogAssetKeys | Sort-Object)
				if ($runtimeManifest.areaId -ne $area.id -or
					[int]$runtimeManifest.assetCount -ne $modelPaths.Count -or
					@($runtimeManifest.assets).Count -ne $modelPaths.Count -or
					($runtimeManifestKeys -join "`n") -ne ($catalogKeys -join "`n")) {
					$singleAreaContractErrors.Add("$($area.id): runtime manifest set/count")
				}
			} else {
				$singleAreaContractErrors.Add("$($area.id): missing runtime manifest")
			}
		} else {
			$singleAreaContractErrors.Add("$($area.id): missing runtime directory")
		}
	}
	Add-Check 'maps.extracted-area-runtime-roots' ($singleAreaContractErrors.Count -eq 0) "errors=$($singleAreaContractErrors.Count)"
	$trainingArea = @($mapCatalog.areas | Where-Object id -eq 'LV_DEV_TRAINING_GROUND')
	$trainingAssetRows = if (Test-Path -LiteralPath 'Client\Bin\DataFiles\Map\LV_DEV_TRAINING_GROUND.mapassets') {
		@(Get-Content -LiteralPath 'Client\Bin\DataFiles\Map\LV_DEV_TRAINING_GROUND.mapassets' | Select-Object -Skip 1).Count
	} else { 0 }
	$trainingPlacementRows = if (Test-Path -LiteralPath 'Client\Bin\DataFiles\Map\LV_DEV_TRAINING_GROUND.mapplacements') {
		@(Get-Content -LiteralPath 'Client\Bin\DataFiles\Map\LV_DEV_TRAINING_GROUND.mapplacements' | Select-Object -Skip 1).Count
	} else { 0 }
	$trainingWorld = Read-Json 'Data\Worlds\LV_DEV_TRAINING_GROUND\Gameplay.world.json'
	$invalidTrainingSpawns = @($trainingWorld.placements | Where-Object {
		$_.kind -ne 'playerSpawn' -or $null -ne $_.archetypeId })
	Add-Check 'maps.training-area-contract' (
		$trainingArea.Count -eq 1 -and
		$trainingArea[0].assetCount -eq 10 -and
		$trainingArea[0].placementCount -eq 18 -and
		$trainingAssetRows -eq 10 -and
		$trainingPlacementRows -eq 18 -and
		@($trainingWorld.placements).Count -eq 4 -and
		$invalidTrainingSpawns.Count -eq 0 -and
		(Test-Path -LiteralPath 'Client\Bin\DataFiles\Navigation\LV_DEV_TRAINING_GROUND.navgrid')) "assets=$trainingAssetRows placements=$trainingPlacementRows spawns=$(@($trainingWorld.placements).Count)"
	$characterSelectArea = @($mapCatalog.areas |
		Where-Object id -eq 'LV_LOBBY_CLASSSELECT_SL00')
	$characterSelectAssetRows = if (Test-Path -LiteralPath 'Client\Bin\DataFiles\Map\LV_LOBBY_CLASSSELECT_SL00.mapassets') {
		@(Get-Content -LiteralPath 'Client\Bin\DataFiles\Map\LV_LOBBY_CLASSSELECT_SL00.mapassets' | Select-Object -Skip 1).Count
	} else { 0 }
	$characterSelectPlacementRows = if (Test-Path -LiteralPath 'Client\Bin\DataFiles\Map\LV_LOBBY_CLASSSELECT_SL00.mapplacements') {
		@(Get-Content -LiteralPath 'Client\Bin\DataFiles\Map\LV_LOBBY_CLASSSELECT_SL00.mapplacements' | Select-Object -Skip 1).Count
	} else { 0 }
	$characterSelectRuntimeRoot = if ($characterSelectArea.Count -eq 1) {
		[string]$characterSelectArea[0].runtimeAssetRoot
	} else { '' }
	$characterSelectManifestPath = if ([string]::IsNullOrWhiteSpace($characterSelectRuntimeRoot)) {
		''
	} else {
		Join-Path 'Client\Bin\Resources' `
			(Join-Path $characterSelectRuntimeRoot 'map_asset_runtime_manifest.json')
	}
	$characterSelectManifest = if (-not [string]::IsNullOrWhiteSpace($characterSelectManifestPath) -and
		(Test-Path -LiteralPath $characterSelectManifestPath)) {
		Read-Json $characterSelectManifestPath
	} else { $null }
	Add-Check 'maps.character-select-area-contract' (
		$characterSelectArea.Count -eq 1 -and
		$characterSelectArea[0].kind -eq 'product' -and
		$characterSelectArea[0].catalogType -eq 'single' -and
		$characterSelectArea[0].assetCount -eq 55 -and
		$characterSelectArea[0].placementCount -eq 803 -and
		$characterSelectAssetRows -eq 55 -and
		$characterSelectPlacementRows -eq 803 -and
		$null -ne $characterSelectManifest -and
		$characterSelectManifest.areaId -eq 'LV_LOBBY_CLASSSELECT_SL00' -and
		$characterSelectManifest.assetCount -eq 55 -and
		@($characterSelectManifest.assets).Count -eq 55) "assets=$characterSelectAssetRows placements=$characterSelectPlacementRows manifest=$($characterSelectManifest.assetCount)"
	Add-Check 'resource.no-lol-annie' (
		-not (Test-Path -LiteralPath 'Client\Bin\Resources\Map\LoL\Annie')) 'legacy Annie resources are quarantined outside the repository'

	$mapPublishPassed = $false
	$mapPublishDetail = ''
	$mapFixtureRoot = Join-Path $repoRoot ".codex_tmp\MapPublishFixture-$PID"
	try {
		$mapFixtureAuthoring = Join-Path $mapFixtureRoot 'Data\Maps\Authoring\FIXTURE'
		$mapFixtureImported = Join-Path $mapFixtureRoot 'Data\Maps\Imported\FIXTURE'
		$mapFixtureRuntime = Join-Path $mapFixtureRoot 'Client\Bin\DataFiles\Map'
		[IO.Directory]::CreateDirectory($mapFixtureAuthoring) | Out-Null
		[IO.Directory]::CreateDirectory($mapFixtureImported) | Out-Null
		[IO.Directory]::CreateDirectory($mapFixtureRuntime) | Out-Null
		$fixtureUtf8 = [Text.UTF8Encoding]::new($false)
		$rowA = '1 "source:a" "A" "editor" "ASSET_A" 0 0 0 0 0 0 1 1 1 1 1'
		$rowB = '2 "source:b" "B" "editor" "ASSET_B" 0 0 0 0 0 0 1 1 1 1 1'
		$rowNew = '3 "source:new" "A" "editor" "ASSET_A" 1 0 0 0 0 0 1 1 1 1 1'
		[IO.File]::WriteAllLines((Join-Path $mapFixtureImported 'FIXTURE.mapset'), @(
			'LOSTARK_MAP_SHARD_SET 1 "FIXTURE" 2',
			'"A" "A.mapassets" "A.mapplacements" 1 1',
			'"B" "B.mapassets" "B.mapplacements" 1 1'), $fixtureUtf8)
		[IO.File]::WriteAllLines((Join-Path $mapFixtureImported 'A.mapassets'), @(
			'LOSTARK_MAP_ASSET_CATALOG 4 "FIXTURE" 1', '"ASSET_A" placeholder'), $fixtureUtf8)
		[IO.File]::WriteAllLines((Join-Path $mapFixtureImported 'B.mapassets'), @(
			'LOSTARK_MAP_ASSET_CATALOG 4 "FIXTURE" 1', '"ASSET_B" placeholder'), $fixtureUtf8)
		[IO.File]::WriteAllLines((Join-Path $mapFixtureImported 'A.mapplacements'), @(
			'LOSTARK_MAP_PLACEMENTS 2 "FIXTURE" 1', $rowA), $fixtureUtf8)
		[IO.File]::WriteAllLines((Join-Path $mapFixtureImported 'B.mapplacements'), @(
			'LOSTARK_MAP_PLACEMENTS 2 "FIXTURE" 1', $rowB), $fixtureUtf8)
		[IO.File]::WriteAllLines((Join-Path $mapFixtureAuthoring 'FIXTURE.mapplacements'), @(
			'LOSTARK_MAP_PLACEMENTS 2 "FIXTURE" 3', $rowA, $rowB, $rowNew), $fixtureUtf8)

		& .\Tools\MapPipeline\Publish-MapAuthoring.ps1 `
			-AreaId FIXTURE `
			-ProjectRoot $mapFixtureRoot | Out-Null
		$publishedMapSet = [IO.File]::ReadAllText((Join-Path $mapFixtureRuntime 'FIXTURE.mapset'))
		$publishedShardA = [IO.File]::ReadAllText((Join-Path $mapFixtureRuntime 'A.mapplacements'))
		$publishedShardB = [IO.File]::ReadAllText((Join-Path $mapFixtureRuntime 'B.mapplacements'))
		$staleMapPublish = @(Get-ChildItem -LiteralPath $mapFixtureRuntime -Force |
			Where-Object Name -Match 'staging|rollback')
		$baselineRuntimeHashes = @{}
		foreach ($runtimeName in @('A.mapplacements', 'B.mapplacements', 'FIXTURE.mapset')) {
			$baselineRuntimeHashes[$runtimeName] =
				(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $mapFixtureRuntime $runtimeName)).Hash
		}
		$rowChanged = '3 "source:new" "A" "editor" "ASSET_A" 2 0 0 0 0 0 1 1 1 1 1'
		[IO.File]::WriteAllLines((Join-Path $mapFixtureAuthoring 'FIXTURE.mapplacements'), @(
			'LOSTARK_MAP_PLACEMENTS 2 "FIXTURE" 3', $rowA, $rowB, $rowChanged), $fixtureUtf8)
		$rollbackRejected = $false
		try {
			& .\Tools\MapPipeline\Publish-MapAuthoring.ps1 `
				-AreaId FIXTURE `
				-ProjectRoot $mapFixtureRoot `
				-FailureAfterPromote 1 | Out-Null
		}
		catch { $rollbackRejected = $true }
		$rollbackPreserved = $true
		foreach ($runtimeName in $baselineRuntimeHashes.Keys) {
			if ($baselineRuntimeHashes[$runtimeName] -ne
				(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $mapFixtureRuntime $runtimeName)).Hash) {
				$rollbackPreserved = $false
			}
		}
		$rollbackStale = @(Get-ChildItem -LiteralPath $mapFixtureRuntime -Force |
			Where-Object Name -Match 'staging|rollback')
		$baselineRuntimeHash = (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $mapFixtureRuntime 'A.mapplacements')).Hash
		[IO.File]::WriteAllLines((Join-Path $mapFixtureAuthoring 'FIXTURE.mapplacements'), @(
			'LOSTARK_MAP_PLACEMENTS 2 "FIXTURE" 3', $rowA, $rowB, ($rowNew + ' EXTRA')), $fixtureUtf8)
		$malformedRejected = $false
		try {
			& .\Tools\MapPipeline\Publish-MapAuthoring.ps1 -AreaId FIXTURE -ProjectRoot $mapFixtureRoot | Out-Null
		}
		catch { $malformedRejected = $true }
		$malformedPreservedRuntime = $baselineRuntimeHash -eq
			(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $mapFixtureRuntime 'A.mapplacements')).Hash

		[IO.File]::WriteAllLines((Join-Path $mapFixtureAuthoring 'FIXTURE.mapplacements'), @(
			'LOSTARK_MAP_PLACEMENTS 2 "FIXTURE" 3', $rowA, $rowB, $rowNew), $fixtureUtf8)
		[IO.File]::WriteAllLines((Join-Path $mapFixtureImported 'FIXTURE.mapset'), @(
			'LOSTARK_MAP_SHARD_SET 1 "FIXTURE" 2',
			'"A" "..\outside.mapassets" "A.mapplacements" 1 2',
			'"B" "B.mapassets" "B.mapplacements" 1 1'), $fixtureUtf8)
		$escapeRejected = $false
		try {
			& .\Tools\MapPipeline\Publish-MapAuthoring.ps1 -AreaId FIXTURE -ProjectRoot $mapFixtureRoot | Out-Null
		}
		catch { $escapeRejected = $true }
		$mapPublishPassed =
			$publishedMapSet -match '"A" "A\.mapassets" "A\.mapplacements" 1 2' -and
			$publishedMapSet -match '"B" "B\.mapassets" "B\.mapplacements" 1 1' -and
			$publishedShardA -match 'LOSTARK_MAP_PLACEMENTS 2 "FIXTURE" 2' -and
			$publishedShardB -match 'LOSTARK_MAP_PLACEMENTS 2 "FIXTURE" 1' -and
			$staleMapPublish.Count -eq 0 -and
			$rollbackRejected -and $rollbackPreserved -and
			$rollbackStale.Count -eq 0 -and
			$malformedRejected -and $malformedPreservedRuntime -and
			$escapeRejected
		$mapPublishDetail = "stale=$($staleMapPublish.Count) rollbackRejected=$rollbackRejected rollbackPreserved=$rollbackPreserved malformedRejected=$malformedRejected escapeRejected=$escapeRejected"
	}
	catch {
		$mapPublishDetail = $_.Exception.Message
	}
	finally {
		if ([IO.Directory]::Exists($mapFixtureRoot)) {
			Remove-Item -LiteralPath $mapFixtureRoot -Recurse -Force
		}
	}
	Add-Check 'maps.sharded-authoring-publish' $mapPublishPassed $mapPublishDetail

	$agentsGuide = Get-Content -LiteralPath 'AGENTS.md' -Raw -Encoding utf8
	$claudeGuide = Get-Content -LiteralPath 'CLAUDE.md' -Raw -Encoding utf8
	$teamReadme = Get-Content -LiteralPath '.md\TEAM\README.md' -Raw -Encoding utf8
	$teamGameplayHandbook = Get-Content -LiteralPath '.md\TEAM\TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md' -Raw -Encoding utf8
	$verticalSliceContractMarker =
		'team-contract: vertical-slice-feature-owner; roles-are-not-file-permissions'
	Add-Check 'team.vertical-slice-ownership' (
		$agentsGuide.Contains($verticalSliceContractMarker) -and
		$claudeGuide.Contains($verticalSliceContractMarker) -and
		$teamReadme.Contains($verticalSliceContractMarker) -and
		$teamGameplayHandbook.Contains($verticalSliceContractMarker)) 'team roles define authority interfaces while feature owners may implement required Data/Shared/Server/Client/harness slices'

    $clientItems = Get-ProjectItems 'Client\Default\Client.vcxproj'
    $engineItems = Get-ProjectItems 'Engine\Default\Engine.vcxproj'
    $serverItems = Get-ProjectItems 'Server\Default\Server.vcxproj'
    $missingProjectItems = [Collections.Generic.List[string]]::new()
    foreach ($pair in @(
        @('Client\Default', $clientItems),
        @('Engine\Default', $engineItems),
        @('Server\Default', $serverItems))) {
        $projectDirectory = [IO.Path]::GetFullPath($pair[0])
        foreach ($item in $pair[1]) {
            $fullName = [IO.Path]::GetFullPath((Join-Path $projectDirectory $item.Include))
            if (-not (Test-Path -LiteralPath $fullName)) {
                $missingProjectItems.Add($fullName)
            }
        }
    }
    Add-Check 'projects.registered-files-exist' ($missingProjectItems.Count -eq 0) "missing=$($missingProjectItems.Count)"

    [xml]$clientFilters = Get-Content -LiteralPath 'Client\Default\Client.vcxproj.filters'
    [xml]$engineFilters = Get-Content -LiteralPath 'Engine\Default\Engine.vcxproj.filters'
    [xml]$serverFilters = Get-Content -LiteralPath 'Server\Default\Server.vcxproj.filters'
    Add-Check 'projects.filters-xml' ($null -ne $clientFilters.Project -and $null -ne $engineFilters.Project -and $null -ne $serverFilters.Project) 'XML parsed'

	$expectedDataIncludes = @(& git ls-files --cached --others --exclude-standard -- Data |
		Where-Object { Test-Path -LiteralPath $_ } |
		ForEach-Object { '..\..\' + $_.Replace('/', '\') } | Sort-Object)
	$clientDataItems = @($clientItems | Where-Object Include -Like '..\..\Data\*')
	$actualDataIncludes = @($clientDataItems | ForEach-Object Include | Sort-Object)
	$filterManager = [Xml.XmlNamespaceManager]::new($clientFilters.NameTable)
	$filterManager.AddNamespace('m', 'http://schemas.microsoft.com/developer/msbuild/2003')
	$clientDataFilterItems = @($clientFilters.SelectNodes('//m:None', $filterManager) |
		Where-Object Include -Like '..\..\Data\*')
	$invalidDataFilters = @($clientDataFilterItems | Where-Object {
		$null -eq $_.Filter -or -not $_.Filter.StartsWith('96.DataFiles') })
	Add-Check 'projects.data-source-visibility' (
		$clientDataItems.Count -eq $expectedDataIncludes.Count -and
		($actualDataIncludes -join "`n") -eq ($expectedDataIncludes -join "`n") -and
		@($clientDataItems | Where-Object LocalName -ne 'None').Count -eq 0 -and
		$clientDataFilterItems.Count -eq $expectedDataIncludes.Count -and
		$invalidDataFilters.Count -eq 0) "expected=$($expectedDataIncludes.Count) project=$($clientDataItems.Count) filters=$($clientDataFilterItems.Count)"

    $activeSourceRoots = @('Client\Private', 'Client\Public', 'Engine\Private', 'Engine\Public')
    $activeFiles = @(Get-ChildItem -Path $activeSourceRoots -Recurse -File |
        Where-Object Extension -in @('.cpp', '.h'))
    $clientSourceFiles = @(Get-ChildItem -Path @('Client\Private', 'Client\Public') -Recurse -File |
        Where-Object Extension -in @('.cpp', '.h'))
    $forbiddenSymbols = 'CCookedModel|CBinaryAssetObject|\bCMonster\b|CVIBuffer_(Cube|Terrain|Instance_Point|Instance_Rect)|LEVEL_(ASSET_TEST|TEST_LEVEL2|BAREN|GAMEPLAY|MAINMENU|LOGO|CHAOS)'
    $legacyHits = @($activeFiles | Select-String -Pattern $forbiddenSymbols -CaseSensitive)
    Add-Check 'source.legacy-symbols' ($legacyHits.Count -eq 0) "hits=$($legacyHits.Count)"
	Add-Check 'source.no-course-sample-root' (
		-not (Test-Path -LiteralPath 'astar') -and
		-not (Test-Path -LiteralPath '99.수업') -and
		-not (Test-Path -LiteralPath '_work\resource-layout-backup\legacy-map-packs\dev-lol-annie') -and
		-not (Test-Path -LiteralPath 'Data\Effects\Editor\IntakeTest')) 'course and legacy smoke samples are quarantined outside the repository'

    $wrapperHits = @($activeFiles | Select-String -Pattern 'Resources[\\/]LostArk')
    Add-Check 'source.resource-wrapper' ($wrapperHits.Count -eq 0) "hits=$($wrapperHits.Count)"

    $effectCookSource = Get-Content -LiteralPath 'Tools\EffectResourceIntake\Cook-SelectedEffectAsset.ps1' -Raw
    $legacyEffectCookHits = @($effectCookSource | Select-String -Pattern 'WintersAssetConverter|\.wmesh|\.wmat|CCookedModel' -AllMatches)
    Add-Check 'effect.no-legacy-mesh-cook' ($legacyEffectCookHits.Count -eq 0) "hits=$($legacyEffectCookHits.Count)"

    $legacyLaunchHits = @($clientSourceFiles | Select-String -Pattern 'CLIENT_SCENARIO|CLIENT_ENTRY_MODE|LOCAL_PREVIEW|CClientLaunchOptions|CLevelCatalog|COfflinePlayerPreview|--smoke|--scenario')
    Add-Check 'source.no-client-runtime-harness' (
        $legacyLaunchHits.Count -eq 0) "legacyRuntimeHits=$($legacyLaunchHits.Count)"

    $readyGaraHits = @($activeFiles | Select-String -Pattern 'Ready_Gara')
    Add-Check 'source.runtime-authoring' ($readyGaraHits.Count -eq 0) "Ready_Gara hits=$($readyGaraHits.Count)"

    $activeCfgFiles = @(Get-ChildItem -Path @('Data', 'Client\Private', 'Client\Public') -Recurse -File -Force |
        Where-Object { $_.Extension.ToLowerInvariant() -eq '.cfg' })
    $cfgReaderHits = @($activeFiles | Select-String -Pattern '\.cfg\b')
    Add-Check 'data.json-only' ($activeCfgFiles.Count -eq 0 -and $cfgReaderHits.Count -eq 0) "cfgFiles=$($activeCfgFiles.Count) readers=$($cfgReaderHits.Count)"

    $animationFiles = @(Get-ChildItem -LiteralPath 'Data\Animation' -Recurse -File -Force)
    $animationLegacyFiles = @(Get-ChildItem -LiteralPath 'Client\Bin\DataFiles\Anim' -File -Force -ErrorAction SilentlyContinue)
    $animationLegacyHits = @($activeFiles | Select-String -Pattern 'DataFiles[\\/]Anim')
    Add-Check 'animation.data-boundary' ($animationFiles.Count -gt 0 -and $animationLegacyFiles.Count -eq 0 -and $animationLegacyHits.Count -eq 0) "data=$($animationFiles.Count) legacyFiles=$($animationLegacyFiles.Count) legacyRefs=$($animationLegacyHits.Count)"

    $functionKeyHits = @($clientSourceFiles | Select-String -Pattern '(VK|DIK)_F([2-5]|[7-9]|1[0-2])\b')
    $f1Hits = @($clientSourceFiles | Select-String -Pattern '(VK|DIK)_F1\b')
    $f6Hits = @($clientSourceFiles | Select-String -Pattern '(VK|DIK)_F6\b')
    Add-Check 'input.official-function-keys' (
		$functionKeyHits.Count -eq 0 -and
		$f1Hits.Count -eq 1 -and
		$f6Hits.Count -eq 1) "forbidden=$($functionKeyHits.Count) f1=$($f1Hits.Count) f6=$($f6Hits.Count)"

	$lanceLogicSource = Get-Content -LiteralPath 'Client\Private\Logic_LanceMaster.cpp' -Raw
	$characterSource = Get-Content -LiteralPath 'Client\Private\Character.cpp' -Raw
	$modelSource = Get-Content -LiteralPath 'Engine\Private\Model.cpp' -Raw
	$boneSource = Get-Content -LiteralPath 'Engine\Private\Bone.cpp' -Raw
	$obsoletePrRuntimeFiles = @(
		'Client\Private\Level_Test2.cpp',
		'Client\Private\SkillData.cpp',
		'Client\Public\SkillData.h')
	$presentObsoletePrRuntimeFiles = @($obsoletePrRuntimeFiles |
		Where-Object { Test-Path -LiteralPath $_ })
	Add-Check 'integration.pr34-pr35-reconciliation' (
		$presentObsoletePrRuntimeFiles.Count -eq 0 -and
		$lanceLogicSource -notmatch 'Get_DIKeyState|DIK_|Play_Skill' -and
		$characterSource -match 'CLIP_BLEND_SECONDS' -and
		$modelSource -match 'Begin_AnimBlend' -and
		$boneSource -match 'Blend_TransformationMatrix') "obsolete=$($presentObsoletePrRuntimeFiles -join ',') localInput=$($lanceLogicSource -match 'Get_DIKeyState|DIK_|Play_Skill') crossfade=$($characterSource -match 'CLIP_BLEND_SECONDS')"

    $changeLevelHits = @($clientSourceFiles | Select-String -Pattern 'Change_Level\(')
    $unexpectedChangeLevelHits = @($changeLevelHits | Where-Object {
        $_.Path -notlike '*Client\Private\MainApp.cpp' })
    Add-Check 'levels.transition-boundary' (
        $changeLevelHits.Count -eq 2 -and
        $unexpectedChangeLevelHits.Count -eq 0) "calls=$($changeLevelHits.Count) unexpected=$($unexpectedChangeLevelHits.Count)"

    $mainAppSource = Get-Content -LiteralPath 'Client\Private\MainApp.cpp' -Raw
    $mainAppHeader = Get-Content -LiteralPath 'Client\Public\MainApp.h' -Raw
    $lobbySource = Get-Content -LiteralPath 'Client\Private\Level_Lobby.cpp' -Raw
    $characterSelectSource = Get-Content -LiteralPath 'Client\Private\Level_CharacterSelect.cpp' -Raw
    $levelRegistrySource = Get-Content -LiteralPath 'Client\Private\LevelRegistry.cpp' -Raw
    $loaderSource = Get-Content -LiteralPath 'Client\Private\Loader.cpp' -Raw
    $replicationHeader = Get-Content -LiteralPath 'Client\Public\ClientReplication.h' -Raw
    $replicationSource = Get-Content -LiteralPath 'Client\Private\ClientReplication.cpp' -Raw
    $bernLevelSource = Get-Content -LiteralPath 'Client\Private\Level_Bern.cpp' -Raw
    $valtanLevelSource = Get-Content -LiteralPath 'Client\Private\Level_ValtanArena.cpp' -Raw
    $developmentLevelSource = Get-Content -LiteralPath 'Client\Private\Level_Development.cpp' -Raw
    $clientProjectText = Get-Content -LiteralPath 'Client\Default\Client.vcxproj' -Raw
    $clientFilterText = Get-Content -LiteralPath 'Client\Default\Client.vcxproj.filters' -Raw
    $clientEntrySource = Get-Content -LiteralPath 'Client\Default\Client.cpp' -Raw

    Add-Check 'levels.lobby-start' (
        $mainAppSource -match 'Start_Level\(LEVEL::LOBBY\)') 'MainApp starts Lobby'
	$characterSelectLoaderFunction = [regex]::Match(
		$loaderSource,
		'HRESULT CLoader::Ready_For_CharacterSelect\(\)[\s\S]*?(?=HRESULT CLoader::Ready_For_Bern\(\))').Value
	$lobbyCommandHeaderSource = Get-Content -LiteralPath 'Client\Public\LobbyCommandService.h' -Raw
	$lobbyCommandSource = Get-Content -LiteralPath 'Client\Private\LobbyCommandService.cpp' -Raw
	$transitionHeaderSource = Get-Content -LiteralPath 'Client\Public\LevelTransitionService.h' -Raw
	$loadingSource = Get-Content -LiteralPath 'Client\Private\Level_Loading.cpp' -Raw
	$frontendHarnessSource = Get-Content -LiteralPath 'Tools\ClientFrontendHarness\Private\ClientFrontendHarness.cpp' -Raw
	$frontendHarnessProject = Get-Content -LiteralPath 'Tools\ClientFrontendHarness\Default\ClientFrontendHarness.vcxproj' -Raw
	$buildRegressionSource = Get-Content -LiteralPath 'Tools\Build\Invoke-BuildAndRegression.ps1' -Raw
    Add-Check 'levels.character-select-contract' (
        $levelRegistrySource -match 'LEVEL::CHARACTER_SELECT' -and
		$levelRegistrySource -match 'LV_LOBBY_CLASSSELECT_SL00' -and
		$levelRegistrySource -match '\{ true, true, -792\.f, 158\.f, -750\.f, 218\.f \}' -and
		$levelRegistrySource -match 'Ready_For_CharacterSelect' -and
		$characterSelectLoaderFunction -match 'Ready_MapArea\(' -and
		$characterSelectLoaderFunction -notmatch 'Ready_Camera_Prototype\(' -and
		$characterSelectLoaderFunction -match 'Ready_Character_Rendering\(' -and
		$characterSelectLoaderFunction -match 'Ready_AnimationPreviewModels\(' -and
		$characterSelectLoaderFunction -match 'CCharacterSelectionState::Try_Get_SelectedClass' -and
		$characterSelectLoaderFunction -match 'const std::array characterClasses = \{ initialClass \}' -and
		$characterSelectSource -match 'CPlayableCharacterAssetService::Is_Ready' -and
		$characterSelectSource -match 'CPlayableCharacterAssetService::Ensure_Prototypes' -and
		$characterSelectSource -match 'm_MapRuntime\.Load_Area' -and
		$characterSelectSource -match 'CMapPlacementRuntime::Ensure_DefaultLight' -and
        $characterSelectSource -match 'CCharacterSelectionState::Select' -and
		$characterSelectSource -match 'CLobbyCommandService::Request\([\s\S]{0,120}eStage[\s\S]{0,120}commandToken' -and
		$characterSelectSource -match 'character-select\.enter-test' -and
		$characterSelectSource -match 'character-select\.enter-bern' -and
		$characterSelectSource -match 'character-select\.enter-valtan' -and
		$characterSelectSource -match 'ImGui::Button\("Enter Test"\)' -and
		$characterSelectSource -match 'ImGui::Button\("Enter Bern"\)' -and
		$characterSelectSource -match 'ImGui::Button\("Enter Valtan"\)' -and
		$characterSelectSource -match 'ImGuiKey_Enter' -and
		$characterSelectSource -notmatch 'NetworkManager|Connect_To_Server|Send_EnterWorld' -and
		$lobbySource -match 'DEFAULT_ENTRY_CLASS' -and
		$lobbySource -match 'Resolve_EntryCharacterClass' -and
		$lobbySource -match 'Send_EnterWorld\(' -and
        $lobbySource -match '"Test"' -and
        $lobbySource -match '"Character Select"' -and
        $lobbySource -match '"Valtan"' -and
		$lobbySource -match '"Bern"') 'Character Select loads one map path and reuses the Lobby server-authorized Test, Bern and Valtan paths for five playable classes'
	Add-Check 'levels.loading-progress-overlay' (
		$loadingSource -match 'CLoader::Get_ActiveStatus\(\)' -and
		$loadingSource -match '"Loading progress"' -and
		$loadingSource -match 'viewport->WorkPos\.y \+ 16\.f' -and
		$loaderSource -match 'Character %zu/%zu \| %s models %zu/%zu \| %s' -and
		$loaderSource -match 'completedModelCount' -and
		$loaderSource -match 'totalModelCount') 'Loading renders top-screen character class/model counts and the active binary file'
	Add-Check 'levels.character-select-handoff-ticket' (
		$lobbyCommandHeaderSource -match 'LOBBY_COMMAND_TOKEN' -and
		$lobbyCommandHeaderSource -match 'Cancel\(' -and
		$lobbyCommandSource -match 'g_iNextToken' -and
		$lobbyCommandSource -match 'g_PendingCommand->iToken != token' -and
		$transitionHeaderSource -match 'iLobbyCommandToken' -and
		$loadingSource -match 'Request_Activation\([\s\S]{0,160}m_iLobbyCommandToken' -and
		$loadingSource -match 'Cancel_LobbyCommand\("target level loading failed"\)' -and
		$mainAppSource -match 'target level loading could not start' -and
		$mainAppSource -match 'target level activation failed' -and
		$frontendHarnessSource -match 'Exact Cancellation Leaves No Stale Command' -and
		$frontendHarnessSource -match 'Stale Failure Cannot Cancel New Handoff' -and
		$frontendHarnessProject -match 'Client\\Private\\LobbyCommandService\.cpp' -and
		$buildRegressionSource -match 'ClientFrontendHarness') 'token service has an executable unit harness and integration failure hooks are statically admitted'
    Add-Check 'levels.release-imgui-entry' (
        $mainAppSource -match 'ReadyImGuiRuntime\(\)' -and
        $lobbySource -match 'Render_StagePanel\(\);' -and
        $lobbySource -notmatch '#ifdef _DEBUG[\s\S]{0,80}Render_StagePanel' -and
		$characterSelectSource -match 'Render_SelectionPanel\(\);' -and
		$characterSelectSource -notmatch '#ifdef _DEBUG[\s\S]{0,80}Render_SelectionPanel' -and
        $clientEntrySource -match 'CImGuiLayer::HandleWindowMessage' -and
        $clientEntrySource -notmatch '#ifdef _DEBUG\s*\r?\n\s*if \(CImGuiLayer::HandleWindowMessage') 'Lobby and Character Select ImGui are available in Release'

    $deletedRuntimeFiles = @(
        'Client\Public\ClientLaunchOptions.h',
        'Client\Private\ClientLaunchOptions.cpp',
        'Client\Public\LevelCatalog.h',
        'Client\Private\LevelCatalog.cpp',
        'Client\Public\OfflinePlayerPreview.h',
        'Client\Private\OfflinePlayerPreview.cpp',
        'Data\Levels\LevelCatalog.json',
        'Tools\Build\Invoke-OfflineClientSmoke.ps1',
        'Tools\Build\Invoke-NetworkEndpointSmoke.ps1')
    $presentDeletedRuntimeFiles = @($deletedRuntimeFiles | Where-Object {
        Test-Path -LiteralPath $_ })
    Add-Check 'levels.legacy-runtime-removed' (
        $presentDeletedRuntimeFiles.Count -eq 0 -and
        $mainAppHeader -notmatch 'SmokeHarness|OfflinePreview|CLIENT_SCENARIO' -and
        $mainAppSource -notmatch 'SmokeHarness|OfflinePreview|CLIENT_SCENARIO' -and
        $replicationHeader -notmatch 'LOCAL_PREVIEW|LocalPreview' -and
        $replicationSource -notmatch 'LOCAL_PREVIEW|LocalPreview' -and
        $clientProjectText -notmatch 'ClientLaunchOptions|LevelCatalog|OfflinePlayerPreview|SceneTransitionService' -and
        $clientFilterText -notmatch 'ClientLaunchOptions|LevelCatalog|OfflinePlayerPreview|SceneTransitionService') "present=$($presentDeletedRuntimeFiles -join ',')"

    $networkManagerSource = Get-Content -LiteralPath 'Client\Private\NetworkManager.cpp' -Raw
    $serverMainSource = Get-Content -LiteralPath 'Server\Private\Main.cpp' -Raw
    $tcpListenerSource = Get-Content -LiteralPath 'Server\Private\TcpListener.cpp' -Raw
    Add-Check 'network.server-authorized-entry' (
        $networkManagerSource -match 'Connect_To_Server\(' -and
        $networkManagerSource -match 'InetPtonA' -and
        $serverMainSource -match '--bind-address' -and
        $tcpListenerSource -match 'INADDR_ANY' -and
        $tcpListenerSource -match 'INADDR_LOOPBACK' -and
        $lobbySource -match 'LOSTARK_SERVER_HOST' -and
        $lobbySource -match 'Connect_To_Server\(serverHost, SERVER_PORT\)' -and
        $lobbySource -notmatch '192\.168\.' -and
        $lobbySource -match 'Send_EnterWorld\(' -and
        $lobbySource -match 'Try_Consume_EnterAccepted' -and
        $lobbySource -match 'seconds\(5\)' -and
        $bernLevelSource -match 'network\.connection-lost' -and
        $valtanLevelSource -match 'network\.connection-lost' -and
        $developmentLevelSource -match 'network\.connection-lost') 'Lobby waits for server approval and product levels recover to Lobby on disconnect'

    $playerControllerSource = Get-Content -LiteralPath 'Client\Private\PlayerController.cpp' -Raw
    Add-Check 'player.command-sink-boundary' ($playerControllerSource -notmatch 'NetworkManager' -and $playerControllerSource -match 'IPlayerCommandSink') 'PlayerController depends on command sink'

    $characterLogicFiles = @(Get-ChildItem -LiteralPath 'Client\Private' -Filter 'Logic_*.cpp' -File)
    $characterLogicBoundaryHits = @($characterLogicFiles | Select-String -Pattern 'Get_DIKey|Get_DIMouse|NetworkManager|Play_Skill\(')
    $characterSpecSource = Get-Content -LiteralPath 'Client\Public\CharacterSpec.h' -Raw
    Add-Check 'character.presentation-boundary' (
        $characterLogicBoundaryHits.Count -eq 0 -and
        $characterSpecSource -match 'Update_Presentation') "forbiddenCalls=$($characterLogicBoundaryHits.Count)"

    $effectRuntimeSource = Get-Content -LiteralPath 'Client\Private\Effect_Runtime.cpp' -Raw
    $effectToolSource = Get-Content -LiteralPath 'Client\Private\Effect_Tool.cpp' -Raw
    Add-Check 'effect.resource-confinement' ($effectRuntimeSource -notmatch 'RequestedPath\.is_absolute\(\)\s*&&' -and $effectToolSource -notmatch 'Candidates\[\][\s\S]{0,120}RequestedPath') 'absolute path candidates rejected'
	$dimensionistEffectAdmission = Read-Json 'Data\Effects\SourceCatalog\dimensionist_admission.json'
	$dimensionistEffectCandidates = Read-Json 'Data\Effects\SourceCatalog\dimensionist_candidates.json'
	$dimensionistEffectFiles = @(Get-ChildItem -LiteralPath 'Data\Effects\Authored\Dimensionist\Candidates' -File -Filter '*.effect')
	$dimensionistEffectTextures = @(Get-ChildItem -LiteralPath 'Client\Bin\Resources\Effect\Dimensionist' -Recurse -File -Filter '*.dds')
	$dimensionistEffectModels = @(Get-ChildItem -LiteralPath 'Client\Bin\Resources\Effect\Dimensionist' -Recurse -File -Filter '*.wmodel')
	Add-Check 'effect.dimensionist-candidate-admission' (
		$dimensionistEffectAdmission.schema -eq 'lostark.effect-authoring-admission' -and
		$dimensionistEffectAdmission.status -eq 'candidate_only' -and
		$dimensionistEffectAdmission.resourceRoot -eq 'Effect/Dimensionist' -and
		[int]$dimensionistEffectAdmission.summary.particleSystemCandidateCount -eq 459 -and
		[int]$dimensionistEffectAdmission.summary.effectFileCount -eq 459 -and
		[int]$dimensionistEffectAdmission.summary.runtimeTextureCount -eq 693 -and
		[int]$dimensionistEffectAdmission.summary.runtimeMeshCount -eq 139 -and
		[int]$dimensionistEffectCandidates.count -eq 459 -and
		@($dimensionistEffectCandidates.rows).Count -eq 459 -and
		$dimensionistEffectFiles.Count -eq 459 -and
		$dimensionistEffectTextures.Count -eq 693 -and
		$dimensionistEffectModels.Count -eq 139 -and
		$effectToolSource -match 'recursive_directory_iterator') "effects=$($dimensionistEffectFiles.Count) textures=$($dimensionistEffectTextures.Count) models=$($dimensionistEffectModels.Count) status=$($dimensionistEffectAdmission.status)"

    $materialReaderSource = Get-Content -LiteralPath 'Engine\Private\BinaryAsset\Winters\WMaterialReader.cpp' -Raw
    $modelSource = Get-Content -LiteralPath 'Engine\Private\Model.cpp' -Raw
    Add-Check 'model.resource-confinement' (
        $materialReaderSource -match 'IsBelowRoot' -and
        $materialReaderSource -notmatch 'Resources/LostArk' -and
        $materialReaderSource -notmatch 'filesystem::exists\(normalizedPath\)' -and
        $modelSource -notmatch 'L"LostArk"') 'material textures remain below the flat Resources root'
	$loaderSource = Get-Content -LiteralPath 'Client\Private\Loader.cpp' -Raw
	$loaderFactoryFiles = @(
		'Engine\Private\Shader.cpp',
		'Engine\Private\Material.cpp',
		'Engine\Private\Model.cpp',
		'Engine\Private\Navigation.cpp',
		'Client\Private\Camera_Free.cpp',
		'Client\Private\Character.cpp',
		'Client\Private\Part_Body.cpp',
		'Client\Private\Part_Equipment.cpp',
		'Client\Private\Valtan.cpp',
		'Client\Private\Body_Valtan.cpp')
	$loaderFactoryModalHits = @($loaderFactoryFiles |
		ForEach-Object { Select-String -LiteralPath $_ -Pattern 'MSG_BOX|MessageBox' })
    Add-Check 'loader.no-worker-modal' (
		$loaderFactoryModalHits.Count -eq 0) "factoryModalHits=$($loaderFactoryModalHits.Count)"
	Add-Check 'loader.no-thread-termination' (
		$loaderSource -notmatch 'TerminateThread' -and
		$loaderSource -match 'TerminateProcess\(GetCurrentProcess\(\), ERROR_TIMEOUT\)') 'cooperative cancellation escalates to bounded process fail-fast, never thread termination'

    $regressionHarnessSource = Get-Content -LiteralPath 'Tools\Build\Invoke-BuildAndRegression.ps1' -Raw
	Add-Check 'harness.runtime-boundary' (
		$regressionHarnessSource -match 'Assert-RuntimeLayout' -and
		$regressionHarnessSource -match 'NetworkProtocolHarness' -and
		$regressionHarnessSource -match '--contract-test' -and
		$regressionHarnessSource -notmatch 'Invoke-ClientSmoke|--smoke|--scenario') 'automation verifies protocol and server contracts without embedding a Client runtime harness'
	Add-Check 'harness.server-gameplay-contract' (
		$regressionHarnessSource -match '--contract-test' -and
		(Test-Path -LiteralPath 'Server\Private\ServerGameplayContractTests.cpp')) 'regression runs server skill, damage, boss, and navigation contracts'

	$solutionLaunch = Read-Json 'Framework.slnLaunch'
	$serverClientProfiles = @($solutionLaunch | Where-Object {
		$_.Name -eq 'Server + Client' })
	$launchProjects = if ($serverClientProfiles.Count -eq 1) {
		@($serverClientProfiles[0].Projects)
	}
	else {
		@()
	}
	Add-Check 'harness.server-client-launch' (
		$launchProjects.Count -eq 2 -and
		$launchProjects[0].Path -eq 'Server\Default\Server.vcxproj' -and
		$launchProjects[0].Action -eq 'Start' -and
		$launchProjects[1].Path -eq 'Client\Default\Client.vcxproj' -and
		$launchProjects[1].Action -eq 'Start') 'Visual Studio launches the real Server before the real Client'

    $characterCatalog = Read-Json 'Data\Actors\CharacterCatalog.json'
    $bossCatalog = Read-Json 'Data\Actors\BossCatalog.json'
    $missingActorAssets = [Collections.Generic.List[string]]::new()
    foreach ($character in @($characterCatalog.characters | Where-Object runtimeStatus -eq 'supported')) {
		foreach ($assetPath in @($character.bodyModel) + @($character.weaponModels) + @($character.equipmentModels)) {
			if ($assetPath -and -not (Test-Path -LiteralPath (Join-Path $resourceRoot $assetPath))) {
				$missingActorAssets.Add($assetPath)
			}
        }
    }
    foreach ($boss in @($bossCatalog.bosses)) {
        foreach ($assetPath in @($boss.bodyModel, $boss.weaponModel)) {
            if ($assetPath -and -not (Test-Path -LiteralPath (Join-Path $resourceRoot $assetPath))) {
                $missingActorAssets.Add($assetPath)
            }
        }
    }
    Add-Check 'actors.catalog-assets' ($missingActorAssets.Count -eq 0) "missing=$($missingActorAssets -join ',')"
	$lobbySource = Get-Content -LiteralPath 'Client\Private\Level_Lobby.cpp' -Raw
	$characterSelectionStateSource = Get-Content -LiteralPath 'Client\Private\CharacterSelectionState.cpp' -Raw
	$playableRoster = @($characterCatalog.characters | ForEach-Object networkClassId) -join ','
	$rosterStatus = @($characterCatalog.characters | ForEach-Object runtimeStatus) -join ','
	Add-Check 'actors.playable-roster' (
		$playableRoster -eq 'LANCE_MASTER,GUNSLINGER,SLAYER,ARTIST,DIMENSIONIST' -and
		$rosterStatus -eq 'supported,supported,supported,supported,supported' -and
		$lobbySource -match 'CHARACTER_CLASS_ID::LANCE_MASTER' -and
		$lobbySource -match 'CHARACTER_CLASS_ID::GUNSLINGER' -and
		$lobbySource -match 'CHARACTER_CLASS_ID::SLAYER' -and
		$lobbySource -match 'CHARACTER_CLASS_ID::ARTIST' -and
		$lobbySource -match 'CHARACTER_CLASS_ID::DIMENSIONIST' -and
		$lobbySource -notmatch 'CHARACTER_CLASS_ID::DESTROYER' -and
		$characterSelectionStateSource -match 'Is_Supported_Playable_Character_Class') "roster=$playableRoster status=$rosterStatus"
	$dimensionistActor = @($characterCatalog.characters | Where-Object networkClassId -eq 'DIMENSIONIST')
	$dimensionistAnimationContracts = @(
		[pscustomobject]@{ Path = 'Data\Animation\Authored\Dimensionist\Dimensionist.animevents'; Header = 'LOSTARK_ANIM_EVENTS 3 "Dimensionist" 0' },
		[pscustomobject]@{ Path = 'Data\Animation\Reference\Dimensionist\Dimensionist.animnotify'; Header = 'LOSTARK_ANIM_NOTIFY 1 "Dimensionist" 0' },
		[pscustomobject]@{ Path = 'Data\Animation\Reference\Dimensionist\Dimensionist.clipmap'; Header = 'LOSTARK_CLIP_MAP 1 "Dimensionist" 0' },
		[pscustomobject]@{ Path = 'Data\Animation\Reference\Dimensionist\Dimensionist.clipseq'; Header = 'LOSTARK_CLIP_SEQ 2 "Dimensionist" 0' },
		[pscustomobject]@{ Path = 'Data\Animation\Reference\Dimensionist\Dimensionist.skilltiming'; Header = 'LOSTARK_SKILL_TIMING 2 "Dimensionist" 0' })
	$invalidDimensionistAnimationDocuments = @(
		foreach ($contract in $dimensionistAnimationContracts) {
			if (-not (Test-Path -LiteralPath $contract.Path -PathType Leaf) -or
				(Get-Content -LiteralPath $contract.Path -Raw).Trim() -cne $contract.Header) {
				$contract.Path
			}
		})
	Add-Check 'actors.dimensionist-runtime-animation' (
		$dimensionistActor.Count -eq 1 -and
		[int]$characterCatalog.formatVersion -eq 2 -and
		$dimensionistActor[0].bodyModel -eq 'Character/Dimensionist/Dimensionist_Character.wmodel' -and
		@($dimensionistActor[0].equipmentModels).Count -eq 0 -and
		(@($dimensionistActor[0].weaponModels) -join ',') -eq (
			'Character/WP_WSWP_M_06/WP_WSWP_M_06L.wmodel,' +
			'Character/WP_WSWP_M_06/WP_WSWP_M_06S.wmodel,' +
			'Character/WP_WSWP_M_06/WP_WSWP_M_06P.wmodel,' +
			'Character/WP_WSWP_M_06/WP_WSWP_M_06E.wmodel') -and
		(Test-Path -LiteralPath 'Client\Bin\Resources\Character\Dimensionist\Dimensionist_Character.wmodel' -PathType Leaf) -and
		$invalidDimensionistAnimationDocuments.Count -eq 0) "combined body, four socketed weapon assets and exact 0-row Animation Tool documents exist; invalid=$($invalidDimensionistAnimationDocuments -join ',')"
	$playableAssetServiceSource = Get-Content -LiteralPath 'Client\Private\PlayableCharacterAssetService.cpp' -Raw
	$dimensionistLogicSource = Get-Content -LiteralPath 'Client\Private\Logic_Dimensionist.cpp' -Raw
	Add-Check 'actors.dimensionist-four-part-weapon' (
		$playableAssetServiceSource -match 'Prototype_Component_Model_Dimensionist_Weapon_L' -and
		$playableAssetServiceSource -match 'Prototype_Component_Model_Dimensionist_Weapon_S' -and
		$playableAssetServiceSource -match 'Prototype_Component_Model_Dimensionist_Weapon_P' -and
		$playableAssetServiceSource -match 'Prototype_Component_Model_Dimensionist_Weapon_E' -and
		$dimensionistLogicSource -match 'b_wp_swm_m_1' -and
		$dimensionistLogicSource -match 'b_wp_swm_m_2' -and
		$dimensionistLogicSource -match 'b_wp_swm_m_3' -and
		$dimensionistLogicSource -match 'b_wp_swm_m_4_02' -and
		$dimensionistLogicSource -match '180\.f') 'Dimensionist L/S/P/E prototype tags and exact battle sockets are connected'
	Add-Check 'actors.dimensionist-actorx-scale' (
		$playableAssetServiceSource -match 'CHARACTER_CLASS_ID::DIMENSIONIST == characterClass[\s\S]{0,80}0\.01f : 0\.0001f') 'ActorX Dimensionist uses 0.01 while legacy character packages retain 0.0001'
	$hudViewModelHeader = Get-Content -LiteralPath 'Client\Public\CombatHUDViewModel.h' -Raw
	$hudViewModelSource = Get-Content -LiteralPath 'Client\Private\CombatHUDViewModel.cpp' -Raw
	$characterSelectSource = Get-Content -LiteralPath 'Client\Private\Level_CharacterSelect.cpp' -Raw
	Add-Check 'hud.selected-class-boundary' (
		$hudViewModelHeader -match 'HUD_PLAYER_STATE[\s\S]{0,300}eCharacterClass' -and
		$hudViewModelHeader -match 'Apply_CharacterPreview' -and
		$hudViewModelSource -match 'definition\.eCharacterClass != characterClass' -and
		$hudViewModelSource -match 'Balance/PlayerProfiles\.json' -and
		$characterSelectSource -match 'Apply_CharacterPreview\(characterClass\)' -and
		$mainAppSource -match 'LEVEL::CHARACTER_SELECT' -and
		$mainAppSource -match 'RenderCombatHUD\(\);') 'runtime HUD carries local class and Character Select preview while filtering skill definitions by class'
	$playerSkillDocument = Read-Json 'Data\Balance\PlayerSkills.json'
	$missingQuickSlots = [Collections.Generic.List[string]]::new()
	foreach ($className in @('LANCE_MASTER','GUNSLINGER','SLAYER','ARTIST')) {
		foreach ($slotName in @('Q','W')) {
			$bindings = @($playerSkillDocument.skills | Where-Object {
				$_.characterClass -eq $className -and $_.inputSlot -eq $slotName
			})
			if ($bindings.Count -ne 1) {
				$missingQuickSlots.Add("${className}:$slotName")
			}
		}
	}
	$dimensionistSkillRows = @($playerSkillDocument.skills |
		Where-Object characterClass -eq 'DIMENSIONIST')
	Add-Check 'gameplay.playable-qw-contract' (
		$missingQuickSlots.Count -eq 0 -and
		$dimensionistSkillRows.Count -eq 0) "missing=$($missingQuickSlots -join ',') dimensionistUnverified=$($dimensionistSkillRows.Count)"

	$quickSkillAnimationContracts = @(
		[pscustomobject]@{ Class = 'LANCE_MASTER'; Asset = 'LanceMaster'; Skills = @(34120, 34080) },
		[pscustomobject]@{ Class = 'GUNSLINGER'; Asset = 'GunSlinger'; Skills = @(38020, 38050) },
		[pscustomobject]@{ Class = 'SLAYER'; Asset = 'Slayer'; Skills = @(45050, 45060) },
		[pscustomobject]@{ Class = 'ARTIST'; Asset = 'Artist'; Skills = @(31210, 31230) }
	)
	$quickSkillAnimationErrors = [Collections.Generic.List[string]]::new()
	foreach ($contract in $quickSkillAnimationContracts) {
		$sequencePath = "Data\Animation\Reference\$($contract.Asset)\$($contract.Asset).clipseq"
		$clipMapPath = "Data\Animation\Reference\$($contract.Asset)\$($contract.Asset).clipmap"
		if (-not (Test-Path -LiteralPath $sequencePath) -or
			-not (Test-Path -LiteralPath $clipMapPath)) {
			$quickSkillAnimationErrors.Add("$($contract.Class):missing animation document")
			continue
		}
		$sequenceSource = Get-Content -LiteralPath $sequencePath -Raw
		$clipMapSource = Get-Content -LiteralPath $clipMapPath -Raw
		foreach ($skillId in $contract.Skills) {
			if ($sequenceSource -notmatch "(?m)^$skillId\s") {
				$quickSkillAnimationErrors.Add("$($contract.Class):$skillId missing clipseq")
			}
			if ($clipMapSource -notmatch "skill=$skillId(?:\s|$)") {
				$quickSkillAnimationErrors.Add("$($contract.Class):$skillId missing clipmap")
			}
		}
	}
	$characterRuntimeSource = Get-Content -LiteralPath 'Client\Private\Character.cpp' -Raw
	Add-Check 'gameplay.playable-qw-animation-contract' (
		$quickSkillAnimationErrors.Count -eq 0 -and
		$characterRuntimeSource -match 'Load_ClipChains\(\)' -and
		$characterRuntimeSource -match 'filesystem::path\(assetName \+ "\.clipseq"\)' -and
		$characterRuntimeSource -match 'Play_Skill\(static_cast<int32_t>\(skillId\)\)') "errors=$($quickSkillAnimationErrors -join ',')"
	$actorCatalogSource = Get-Content -LiteralPath 'Client\Private\ActorCatalog.cpp' -Raw
	$actorLoaderSource = Get-Content -LiteralPath 'Client\Private\Loader.cpp' -Raw
	$playableAssetServiceSource = Get-Content -LiteralPath 'Client\Private\PlayableCharacterAssetService.cpp' -Raw
	$replicationSource = Get-Content -LiteralPath 'Client\Private\ClientReplication.cpp' -Raw
	$hardcodedActorModelHits = @(($actorLoaderSource + $playableAssetServiceSource) |
		Select-String -AllMatches -Pattern 'Character/[A-Za-z0-9_./-]+\.wmodel')
	Add-Check 'actors.runtime-catalog-boundary' (
		$actorCatalogSource -match 'Actors/CharacterCatalog\.json' -and
		$actorCatalogSource -match 'Actors/BossCatalog\.json' -and
		$playableAssetServiceSource -match 'CActorCatalog::Find_Character' -and
		$actorLoaderSource -match 'CActorCatalog::Find_Boss' -and
		$replicationSource -match 'CActorCatalog::Find_Boss\(spawned\.strArchetypeId\)' -and
		$hardcodedActorModelHits.Count -eq 0) "hardcodedModelPaths=$($hardcodedActorModelHits.Count)"
	Add-Check 'actors.selected-first-on-demand-load' (
		$actorLoaderSource -match 'Get_LocalCharacterClass\(\)' -and
		$actorLoaderSource -match 'CPlayableCharacterAssetService::Begin_LevelLoad' -and
		$actorLoaderSource -match 'CPlayableCharacterAssetService::Ensure_Prototypes' -and
		$replicationSource -match 'CPlayableCharacterAssetService::Ensure_Prototypes' -and
		$playableAssetServiceSource -match 'g_ReadyClassesByLevel') 'loader admits selected class; replication admits a remote class once'

    $worldValidationPassed = $true
    $worldValidationDetail = ''
    try {
        $worldValidationDetail = (& .\Tools\WorldPipeline\Publish-WorldGameplay.ps1 -Mode Validate 2>&1) -join ' '
    }
    catch {
        $worldValidationPassed = $false
        $worldValidationDetail = $_.Exception.Message
    }
    Add-Check 'world.publish-contract' $worldValidationPassed $worldValidationDetail

	$gameplayValidationPassed = $true
	$gameplayValidationDetail = ''
	try {
		$gameplayValidationDetail = (& .\Tools\GameplayPipeline\Publish-GameplayBalance.ps1 -Mode Validate 2>&1) -join ' '
	}
	catch {
		$gameplayValidationPassed = $false
		$gameplayValidationDetail = $_.Exception.Message
	}
	Add-Check 'gameplay.balance-publish-contract' $gameplayValidationPassed $gameplayValidationDetail

	$navigationValidationPassed = $true
	$navigationValidationDetail = ''
	$legacyNavigationFiles = @(Get-ChildItem -LiteralPath 'Client\Bin\DataFiles\Navigation' `
		-File -Filter 'ValtanArena.*' -ErrorAction SilentlyContinue)
	$valtanNavigationAuthoring = @(
		'Data\Navigation\LV_LUT_HEARTRB_ED.navsource',
		'Data\Navigation\LV_LUT_HEARTRB_ED.navpaint',
		'Data\Navigation\LV_LUT_HEARTRB_ED.navblockers')
	$missingValtanNavigationAuthoring = @($valtanNavigationAuthoring | Where-Object {
		-not (Test-Path -LiteralPath $_ -PathType Leaf)
	})
	Add-Check 'navigation.data-root-contract' (
		$legacyNavigationFiles.Count -eq 0 -and
		$missingValtanNavigationAuthoring.Count -eq 0 -and
		(Test-Path -LiteralPath 'Client\Bin\DataFiles\Navigation\LV_LUT_HEARTRB_ED.navgrid' -PathType Leaf)) `
		"legacy=$($legacyNavigationFiles.Count) missingAuthoring=$($missingValtanNavigationAuthoring.Count)"
	try {
		$navigationValidationDetail = (& .\Tools\NavigationPipeline\Publish-ServerNavigation.ps1 -Mode Validate 2>&1) -join ' '
	}
	catch {
		$navigationValidationPassed = $false
		$navigationValidationDetail = $_.Exception.Message
	}
	Add-Check 'navigation.server-publish-contract' $navigationValidationPassed $navigationValidationDetail

	$worldRollbackPassed = $false
	$worldRollbackDetail = ''
	$worldFixtureRelative = ".codex_tmp/ProjectAuditWorldPublish-$PID"
	$worldFixturePath = Join-Path $repoRoot $worldFixtureRelative
	try {
		[IO.Directory]::CreateDirectory($worldFixturePath) | Out-Null
		$bernFixture = Join-Path $worldFixturePath 'BERN.worldbootstrap'
		$valtanFixture = Join-Path $worldFixturePath 'VALTAN_ARENA.worldbootstrap'
		$trainingFixture = Join-Path $worldFixturePath 'TRAINING_GROUND.worldbootstrap'
		[IO.File]::WriteAllText($bernFixture, 'original-bern')
		[IO.File]::WriteAllText($valtanFixture, 'original-valtan')
		[IO.File]::WriteAllText($trainingFixture, 'original-training')
		try {
			& .\Tools\WorldPipeline\Publish-WorldGameplay.ps1 `
				-Mode Publish `
				-OutputRoot $worldFixtureRelative `
				-FailureAfterPromote 1 | Out-Null
			throw 'Failure injection unexpectedly succeeded.'
		}
		catch {
			if ($_.Exception.Message -eq 'Failure injection unexpectedly succeeded.') {
				throw
			}
		}
		$staleTransactionFiles = @(Get-ChildItem -LiteralPath $worldFixturePath -Force |
			Where-Object Name -Match '^\.staging\.|\.rollback\.')
		$worldRollbackPassed =
			[IO.File]::ReadAllText($bernFixture) -eq 'original-bern' -and
			[IO.File]::ReadAllText($valtanFixture) -eq 'original-valtan' -and
			[IO.File]::ReadAllText($trainingFixture) -eq 'original-training' -and
			$staleTransactionFiles.Count -eq 0
		$worldRollbackDetail = "stale=$($staleTransactionFiles.Count)"
	}
	catch {
		$worldRollbackDetail = $_.Exception.Message
	}
	finally {
		if ([IO.Directory]::Exists($worldFixturePath)) {
			Remove-Item -LiteralPath $worldFixturePath -Recurse -Force
		}
	}
	Add-Check 'world.publish-generation-rollback' $worldRollbackPassed $worldRollbackDetail

    $worldContractFiles = @(
        'Client\Public\WorldGameplayDocument.h',
        'Client\Private\WorldGameplayDocument.cpp',
        'Server\Public\WorldBootstrap.h',
        'Server\Private\WorldBootstrap.cpp',
        'Shared\Public\Network\PacketMessages.h',
        'Tools\WorldPipeline\Publish-WorldGameplay.ps1')
    $monsterContractHits = @($worldContractFiles |
        ForEach-Object { Select-String -LiteralPath $_ -Pattern '\bMONSTER\b|"monster"' })
    $staleWorldPublishFiles = @(Get-ChildItem -LiteralPath 'Server\Bin\DataFiles\World' -File -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -match '\.(tmp|rollback)\.' })
    Add-Check 'world.no-legacy-monster-contract' (
        -not (Test-Path -LiteralPath 'Data\Actors\MonsterCatalog.json') -and
        $monsterContractHits.Count -eq 0) "contractHits=$($monsterContractHits.Count) catalogExists=$(Test-Path -LiteralPath 'Data\Actors\MonsterCatalog.json')"
    Add-Check 'world.publish-cleanup' ($staleWorldPublishFiles.Count -eq 0) "stale=$($staleWorldPublishFiles.Name -join ',')"

    $serverRoomSource = Get-Content -LiteralPath 'Server\Private\GameRoom.cpp' -Raw
    $serverProjectSource = Get-Content -LiteralPath 'Server\Default\Server.vcxproj' -Raw
    Add-Check 'server.world-bootstrap-boundary' (
        $serverRoomSource -match 'Find_AvailablePlayerSpawn' -and
        $serverRoomSource -match 'Update_WorldEntities' -and
        $serverRoomSource -match 'iPatternTelegraphMs' -and
        $serverRoomSource -notmatch 'fActionElapsedSeconds >= 0\.8f' -and
        $serverRoomSource -notmatch 'm_Players\.size\(\)\) \* 2\.f' -and
        $serverProjectSource -match 'PublishWorldGameplay') 'MapTool world documents publish before Server compile'

    $packetMessagesSource = Get-Content -LiteralPath 'Shared\Public\Network\PacketMessages.h' -Raw
    $clientReplicationSource = Get-Content -LiteralPath 'Client\Private\ClientReplication.cpp' -Raw
    Add-Check 'server.world-entity-replication' (
        $packetMessagesSource -match 'S2C_WORLD_ENTITY_SPAWNED' -and
        $packetMessagesSource -match 'WORLD_ENTITY_ACTION' -and
		$clientReplicationSource -match 'CActorCatalog::Find_Boss\(spawned\.strArchetypeId\)' -and
        $serverRoomSource -match 'Broadcast_WorldSnapshot' -and
        $serverRoomSource -match 'Update_WorldEntities') 'server owns world entity state; client consumes presentation snapshots'

	$playerControllerSource = Get-Content -LiteralPath 'Client\Private\PlayerController.cpp' -Raw
	$hudViewModelSource = Get-Content -LiteralPath 'Client\Private\CombatHUDViewModel.cpp' -Raw
	Add-Check 'gameplay.command-server-truth' (
		$packetMessagesSource -match 'C2S_USE_SKILL' -and
		$packetMessagesSource -match 'PLAYER_ACTION_STATE' -and
		$serverRoomSource -match 'Handle_UseSkill' -and
		$playerControllerSource -match 'Request_UseSkill' -and
		$playerControllerSource -notmatch 'Play_Skill') 'input emits intent and presentation consumes approved snapshot action'
	$skillCatalogSource = Get-Content -LiteralPath 'Client\Private\PlayerSkillCatalog.cpp' -Raw
	Add-Check 'gameplay.skill-binding-is-data' (
		$playerControllerSource -match 'CPlayerSkillCatalog::Find_BySlot' -and
		$playerControllerSource -notmatch '\b3\d{4}\b' -and
		$skillCatalogSource -match 'Balance/PlayerSkills\.json') 'quick slots resolve through balance data instead of skill IDs hardcoded per class'
	Add-Check 'ui.combat-viewmodel-boundary' (
		$skillCatalogSource -match 'Balance/PlayerSkills\.json' -and
		$skillCatalogSource -match 'Balance/DamageProfiles\.json' -and
		$hudViewModelSource -match 'CPlayerSkillCatalog::Get_Skills' -and
		$clientReplicationSource -match 'Apply_LocalPlayer' -and
		$clientReplicationSource -match 'Apply_Boss') 'HUD consumes server snapshot plus validated balance definitions without packets'

    $report = [ordered]@{
        schema = 'lostark.project-audit-report'
        formatVersion = 1
        generatedUtc = [DateTime]::UtcNow.ToString('o')
        passed = $failures.Count -eq 0
        checks = $checks
        failures = $failures
    }
    $resolvedReport = [IO.Path]::GetFullPath($ReportPath)
    [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($resolvedReport)) | Out-Null
    [IO.File]::WriteAllText(
        $resolvedReport,
        ($report | ConvertTo-Json -Depth 8) + [Environment]::NewLine,
        [Text.UTF8Encoding]::new($false))

    if ($failures.Count -gt 0) {
        throw "Project audit failed ($($failures.Count)): $($failures -join '; ')"
    }
    Write-Output "Project audit passed: $($checks.Count) checks. Report: $resolvedReport"
}
catch {
    Write-Error ("{0}`n{1}" -f $_.Exception.Message, $_.ScriptStackTrace)
    exit 1
}
