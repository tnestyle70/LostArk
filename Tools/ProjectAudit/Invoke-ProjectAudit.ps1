[CmdletBinding()]
param(
    [string]$ReportPath = '.codex_tmp/ProjectAudit.json',
    [string]$ResourceRoot = ''
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Get-Location).Path)
$resourceRoot = if ([string]::IsNullOrWhiteSpace($ResourceRoot)) {
    Join-Path $repoRoot 'Client\Bin\Resources'
}
else {
    [IO.Path]::GetFullPath($ResourceRoot)
}
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
            if ($area.PSObject.Properties.Name -contains $property -and
                -not (Test-Path -LiteralPath $area.$property) -and
                -not ($area.id -eq 'LV_BER_BERNCASTLE' -and
                    $property -in @('navigationSource', 'navigationPaint'))) {
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
		$characterSelectEditor.PSObject.Properties.Name -contains 'gameplayDocument' -and
		$null -ne $bernEditor -and
		$bernEditor.PSObject.Properties.Name -contains 'gameplayDocument' -and
		$bernEditor.navigationSource -eq 'Data/Navigation/LV_BER_BERNCASTLE.navsource' -and
		$bernEditor.navigationPaint -eq 'Data/Navigation/LV_BER_BERNCASTLE.navpaint' -and
		$null -ne $valtanEditor -and
		$valtanEditor.PSObject.Properties.Name -contains 'navigationSource' -and
		$valtanEditor.PSObject.Properties.Name -contains 'navigationPaint' -and
		$valtanEditor.PSObject.Properties.Name -contains 'navigationBlockers' -and
		$valtanEditor.PSObject.Properties.Name -contains 'gameplayDocument' -and
		$null -ne $trainingEditor -and
		$trainingEditor.PSObject.Properties.Name -notcontains 'navigationSource' -and
		$trainingEditor.PSObject.Properties.Name -notcontains 'gameplayDocument' -and
		$mapToolSource -match 'Load_Source\(' -and
		$mapToolSource -match 'if \(descriptor\.areaId == "LV_LOBBY_CLASSSELECT_SL00" \|\|[\s\S]{0,160}descriptor\.areaId == "LV_BER_BERNCASTLE"' -and
		$mapToolSource -match 'allowNavigationBootstrap\s*=\s*descriptor\.areaId == "LV_BER_BERNCASTLE"' -and
		$mapToolSource -notmatch '\.Export_Runtime\(' -and
		$mapToolSource -match 'MapEditorWorkspaceService::Is_Active' -and
		$loaderSource -match 'Ready_MapAuthoringCore' -and
		$loaderSource -match 'MapEditorWorkspaceService::Is_Requested' -and
		$mainAppSource -notmatch 'debug\.map-editor' -and
		$lobbySource -match 'MapEditorWorkspaceService::Request\(' -and
		$lobbySource -match 'LOBBY_COMMAND_PURPOSE::MAP_EDITOR_WORKSPACE'
	Add-Check 'maps.editor-workspace-policy' $editorPoliciesValid 'Lobby Test owns editor entry; F1 only toggles tools; four exact editor Areas; Data-only save'
	$mapLoadScopeHeader = Get-Content 'Client\Public\MapLoadScope.h' -Raw
	$mapPlacementRuntimeSource = Get-Content 'Client\Private\MapPlacementRuntime.cpp' -Raw
	$levelRegistryMapScopeSource = Get-Content 'Client\Private\LevelRegistry.cpp' -Raw
	$productEditorVisualScopeValid =
		$mapLoadScopeHeader -match 'std::string excludedAssetGroupId' -and
		$mapPlacementRuntimeSource -match 'left\.excludedAssetGroupId == right\.excludedAssetGroupId' -and
		$mapPlacementRuntimeSource -match 'pAsset->groupId == loadScope\.excludedAssetGroupId' -and
		$levelRegistryMapScopeSource -match 'MakeFullMapScope\("landscape"\)' -and
		$levelRegistryMapScopeSource -match '"LV_LUT_HEARTRB_ED",\s*"scene\.valtan\.cool-low-key\.v1",\s*MakeFullMapScope\(\)' -and
		$mapToolSource -match 'IsBernLandscapePlacement' -and
		$mapToolSource -match 'Show Bern Landscape'
	Add-Check 'maps.product-editor-visual-scope' $productEditorVisualScopeValid 'Bern full published map excluding quarantined landscape; Valtan full published map; MapTool reversible Bern preview'

	$physicsSdkRoot = Join-Path $repoRoot 'Engine\ThirdPartyLib\PhysX'
	$physicsSdkRequired = @(
		'Inc\PxPhysicsAPI.h',
		'Lib\Debug\PhysX_64.lib',
		'Lib\Debug\PhysXCommon_64.lib',
		'Lib\Debug\PhysXFoundation_64.lib',
		'Lib\Debug\PhysXExtensions_static_64.lib',
		'Lib\Debug\PhysXPvdSDK_static_64.lib',
		'Lib\Release\PhysX_64.lib',
		'Lib\Release\PhysXCommon_64.lib',
		'Lib\Release\PhysXFoundation_64.lib',
		'Lib\Release\PhysXExtensions_static_64.lib',
		'Lib\Release\PhysXPvdSDK_static_64.lib',
		'Bin\Debug\PhysX_64.dll',
		'Bin\Debug\PhysXCommon_64.dll',
		'Bin\Debug\PhysXFoundation_64.dll',
		'Bin\Release\PhysX_64.dll',
		'Bin\Release\PhysXCommon_64.dll',
		'Bin\Release\PhysXFoundation_64.dll',
		'LICENSE.md',
		'NOTICE.md')
	$missingPhysicsSdk = @($physicsSdkRequired | Where-Object {
		-not (Test-Path -LiteralPath (Join-Path $physicsSdkRoot $_))
	})
	Add-Check 'physics.physx-sdk-layout' ($missingPhysicsSdk.Count -eq 0) `
		"missing=$($missingPhysicsSdk -join ',')"
	$clientRuntimeProjectSource = Get-Content 'Client\Default\Client.vcxproj' -Raw
	$updateLibSource = Get-Content 'UpdateLib.bat' -Raw
	$physxRuntimeNames = @(
		'PhysX_64.dll',
		'PhysXCommon_64.dll',
		'PhysXFoundation_64.dll')
	$physxRuntimeDeploymentValid =
		$clientRuntimeProjectSource -match 'DeployClientRuntimeDependencies' -and
		$clientRuntimeProjectSource -match 'PhysXRuntimeRoot'
	foreach ($runtimeName in $physxRuntimeNames) {
		$physxRuntimeDeploymentValid = $physxRuntimeDeploymentValid -and
			$clientRuntimeProjectSource.Contains($runtimeName) -and
			$updateLibSource.Contains($runtimeName)
	}
	Add-Check 'physics.client-runtime-deployment' $physxRuntimeDeploymentValid `
		'Direct Client builds and UpdateLib both deploy all three configuration-matched PhysX runtimes'

	$physicsManagerHeader = Get-Content 'Engine\Public\Physics_Manager.h' -Raw
	$physicsManagerSource = Get-Content 'Engine\Private\Physics_Manager.cpp' -Raw
	$rigidBodyHeader = Get-Content 'Engine\Public\RigidBody.h' -Raw
	$gameInstanceSource = Get-Content 'Engine\Private\GameInstance.cpp' -Raw
	$destructionRuntimeSource = Get-Content 'Client\Private\DestructionSimulationRuntime.cpp' -Raw
	$destructionControllerSource = Get-Content 'Client\Private\DestructionSimulationController.cpp' -Raw
	$physicsContractValid =
		$physicsManagerHeader -match 'PHYSICS_ACTOR_HANDLE' -and
		$physicsManagerHeader -match 'Simulate_DebugSteps\(uint32_t' -and
		$physicsManagerHeader -notmatch '#include\s*[<\"]Px' -and
		$physicsManagerSource -match 'simulate\(CPhysics_Manager::FIXED_TIMESTEP\)' -and
		$physicsManagerSource -match 'fetchResults\(true\)' -and
		$rigidBodyHeader -match 'Create_Runtime' -and
		$gameInstanceSource -match 'm_pPhysics_Manager->Update\(fTimeDelta\)' -and
		$gameInstanceSource -match 'Post_Physics_Update\(fTimeDelta\)' -and
		$destructionRuntimeSource -match 'Begin_PhysicsPreview' -and
		$destructionRuntimeSource -match 'Simulate_DebugSteps' -and
		$destructionControllerSource -match 'Advance_Timeline\(FIXED_DELTA_SECONDS' -and
		$destructionControllerSource -match 'Simulate_PhysicsSteps\(1u' -and
		$destructionControllerSource -match 'Post_Physics_Update\(status\)'
	Add-Check 'physics.fixed-step-destruction-preview' $physicsContractValid `
		'handle facade; no public PhysX include; one paused 1/60 clock; Deploy pose pull'

	$simulationPath = 'Data\Maps\Authoring\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.destructionsimulation.json'
	$worldEventsPath = 'Data\Encounters\Valtan\ValtanWorldEvents.json'
	$simulation = Read-Json $simulationPath
	$worldEvents = Read-Json $worldEventsPath
	$profile = @($simulation.profiles)[0]
	$group = @($worldEvents.groups | Where-Object {
		$_.groupId -eq $profile.groupId
	})[0]
	$simulationElementIds = @($profile.elements | ForEach-Object {
		[string]$_.elementId
	})
	$simulationPlacementIds = @($profile.elements | ForEach-Object {
		[string]$_.sourceRuntimePlacementId
	})
	$simulationVectorsValid = $true
	foreach ($element in @($profile.elements)) {
		$direction = @($element.direction)
		$lengthSquared = if ($direction.Count -eq 3) {
			[double]$direction[0] * [double]$direction[0] +
			[double]$direction[1] * [double]$direction[1] +
			[double]$direction[2] * [double]$direction[2]
		} else { 0.0 }
		$simulationVectorsValid = $simulationVectorsValid -and
			[math]::Abs([math]::Sqrt($lengthSquared) - 1.0) -le 0.001 -and
			[double]$element.speedMetersPerSecond -ge 0.0 -and
			[double]$element.gravityScale -ge 0.0 -and
			[double]$element.lifetimeSeconds -gt 0.0
	}
	$simulationAuthoringValid =
		$simulation.schema -eq 'lostark.destruction-simulation' -and
		[int]$simulation.formatVersion -eq 1 -and
		$simulation.areaId -eq 'LV_LUT_HEARTRB_ED' -and
		@($simulation.profiles).Count -eq 1 -and
		$null -ne $group -and
		$simulationElementIds.Count -eq 5 -and
		@($simulationElementIds | Select-Object -Unique).Count -eq 5 -and
		@($simulationPlacementIds | Select-Object -Unique).Count -eq 5 -and
		@($simulationPlacementIds | Where-Object {
			$_ -notin @($group.memberPlacementIds | ForEach-Object { [string]$_ })
		}).Count -eq 0 -and
		$simulationVectorsValid -and
		@($worldEvents.bindings | Where-Object enabled).Count -eq 0
	Add-Check 'maps.valtan-destruction-simulation-authoring' `
		$simulationAuthoringValid `
		"profiles=$(@($simulation.profiles).Count) elements=$($simulationElementIds.Count) bindingsEnabled=$(@($worldEvents.bindings | Where-Object enabled).Count)"

	$worldPublisherSource = Get-Content 'Tools\WorldPipeline\Publish-WorldGameplay.ps1' -Raw
	$destroyableProductGateClosed =
		$worldPublisherSource -match "placement\.kind -notin @\('playerSpawn','npc','boss','triggerBox','collisionBox'\)" -and
		$worldPublisherSource -notmatch "placement\.kind -eq 'destroyable'"
	Add-Check 'world.destroyable-product-gate' $destroyableProductGateClosed `
		'MapTool physics authoring is available; product destroyable admission remains fail-closed'
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

		$runtimeDirectory = Join-Path $resourceRoot $runtimePrefix.Replace('/', '\')
		if (Test-Path -LiteralPath $runtimeDirectory -PathType Container) {
			foreach ($modelPath in $modelPaths) {
				$modelFile = Join-Path $resourceRoot $modelPath.Replace('/', '\')
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
		Join-Path $resourceRoot `
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
		-not (Test-Path -LiteralPath (Join-Path $resourceRoot 'Map\LoL\Annie'))) 'legacy Annie resources are quarantined outside the repository'

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
		-not (Test-Path -LiteralPath '_work\resource-layout-backup\legacy-map-packs\dev-lol-annie')) 'course and legacy smoke samples are quarantined outside the repository'

	$effectDocumentHeader = Get-Content -LiteralPath 'Client\Public\Effect_AuthoringDocument.h' -Raw
	$effectToolHeader = Get-Content -LiteralPath 'Client\Public\Effect_Tool.h' -Raw
	$effectToolSource = Get-Content -LiteralPath 'Client\Private\Effect_Tool.cpp' -Raw
	$effectMeshPreviewShader = Get-Content -LiteralPath 'Client\Bin\ShaderFiles\Shader_VtxEffectMeshPreview.hlsl' -Raw
	$effectRectPreviewShader = Get-Content -LiteralPath 'Client\Bin\ShaderFiles\Shader_VtxEffectRectPreview.hlsl' -Raw
	$mainAppSource = Get-Content -LiteralPath 'Client\Private\MainApp.cpp' -Raw
	$clientEntrySource = Get-Content -LiteralPath 'Client\Default\Client.cpp' -Raw
	$engineImGuiSource = Get-Content -LiteralPath 'Engine\Private\ImGuiLayer.cpp' -Raw
	$clientProjectSource = Get-Content -LiteralPath 'Client\Default\Client.vcxproj' -Raw
	$removedEffectPaths = @(
		'Client\Public\Effect_Types.h',
		'Client\Public\Effect_AssetIO.h',
		'Client\Private\Effect_AssetIO.cpp',
		'Client\Public\Effect_ParticleSimulator.h',
		'Client\Private\Effect_ParticleSimulator.cpp',
		'Client\Public\Effect_Runtime.h',
		'Client\Private\Effect_Runtime.cpp',
		'Client\Public\Effect_ResourceCatalog.h',
		'Client\Private\Effect_ResourceCatalog.cpp',
		'Client\Public\Effect_Preview.h',
		'Client\Private\Effect_Preview.cpp')
	$removedEffectPathHits = @($removedEffectPaths |
		Where-Object { Test-Path -LiteralPath $_ })
	$authoredEffectFiles = @(Get-ChildItem -LiteralPath 'Data\Effects\Authored' -Recurse -File -ErrorAction SilentlyContinue)
	$unexpectedAuthoredEffectFiles = @($authoredEffectFiles |
		Where-Object { $_.Name -notmatch '^[A-Za-z0-9_.-]+\.effect\.json$' })
	$effectIntakeFiles = @(Get-ChildItem -LiteralPath 'Tools\EffectResourceIntake' -Recurse -File -ErrorAction SilentlyContinue)
	$effectShaderFiles = @(Get-ChildItem -LiteralPath 'Client\Bin\ShaderFiles' -File -Filter 'Shader_Effect*' -ErrorAction SilentlyContinue)
	$legacyEffectSymbolHits = @($clientSourceFiles | Select-String -Pattern 'Effect_(AssetIO|ParticleSimulator|Runtime|ResourceCatalog|Types)|CEffect_Runtime|EFFECT_ASSET_DESC')
	$legacyEffectProjectHits = @($clientProjectSource | Select-String -Pattern 'Effect_(AssetIO|ParticleSimulator|Runtime|ResourceCatalog|Types)')
	$legacyEffectEntry =
		$clientEntrySource -match 'Effect_(AssetIO|ParticleSimulator|Runtime|ResourceCatalog|Types)|CEffect_Runtime|EFFECT_ASSET_DESC|--effect-' -or
		$engineImGuiSource -match '--effect-'
	$effectPassOrderPattern =
		'pass\s+OpaqueBackDepthWrite[\s\S]*pass\s+AlphaTwoSidedDepthRead[\s\S]*pass\s+AdditiveTwoSidedDepthRead'
	$effectG6DetailPreviewShape =
		$effectDocumentHeader -match 'EFFECT_AUTHORING_FORMAT_VERSION\s*=\s*12u' -and
		$effectDocumentHeader -match 'struct EFFECT_SOURCE_MATERIAL_DESC[\s\S]*DynamicParameterSemantics[\s\S]*strSubUVMode' -and
		$effectDocumentHeader -match 'struct EFFECT_PARTICLE_SYSTEM_DESC[\s\S]*fUniformScaleMultiplier[\s\S]*fDirectionYawDegrees[\s\S]*fInitialSpeedMultiplier' -and
		$effectDocumentHeader -match 'EFFECT_AUTHORING_MIN_SUPPORTED_VERSION\s*=\s*3u' -and
		$effectDocumentHeader -match 'struct EFFECT_ELEMENT_DESC[\s\S]*ResourceBindings[\s\S]*Material[\s\S]*Detail' -and
		$effectDocumentHeader -match 'struct EFFECT_DETAIL_DESC[\s\S]*Transform[\s\S]*Color[\s\S]*UV[\s\S]*Timing[\s\S]*Mesh[\s\S]*Sprite[\s\S]*Decal[\s\S]*LinearLerp[\s\S]*Particle[\s\S]*Trail[\s\S]*AfterImage' -and
		$effectDocumentHeader -notmatch 'filesystem|DataJson|Parse_EffectDocument|Serialize_EffectDocument' -and
		$effectToolHeader -match 'weak_ptr<CEffectObject>\s+m_pWorldPreviewObject' -and
		$effectToolHeader -match 'Render_EffectDetailWindow' -and
		$effectToolHeader -match 'Render_ModelViewWindow' -and
		$effectToolSource -match 'Try_CommitDocument' -and
		$effectToolSource -match 'Stage_WorldPreview' -and
		$effectToolSource -match 'Render_ResourceGrid' -and
		$effectToolSource -match 'Render_SourceRecipeDetail' -and
		$effectToolSource -match 'duplicate classes execute in source order'
	$effectG6DetailPreviewShape =
		$effectG6DetailPreviewShape -and
		$mainAppSource -match 'make_unique<CEffect_Tool>\(\s*m_pDevice,\s*m_pContext,\s*m_pCharacterPreviewPanel\s*\)' -and
		$mainAppSource -match 'Prototype_GameObject_EffectObject' -and
		$effectMeshPreviewShader -match $effectPassOrderPattern -and
		$effectRectPreviewShader -match $effectPassOrderPattern -and
		(Test-Path -LiteralPath 'Client\Private\Effect_Playback.cpp') -and
		(Test-Path -LiteralPath 'Client\Private\Effect_DocumentRenderer.cpp') -and
		(Test-Path -LiteralPath 'Client\Private\Effect_PresentationService.cpp') -and
		(Test-Path -LiteralPath 'Tools\EffectPipeline\Publish-Effects.ps1') -and
		$clientProjectSource -match 'Shader_VtxEffectMeshPreview\.hlsl' -and
		$clientProjectSource -match 'Shader_VtxEffectRectPreview\.hlsl' -and
		$clientProjectSource -match 'Shader_VtxEffectParticle\.hlsl' -and
		$clientProjectSource -match 'Effect_DocumentRenderer\.cpp' -and
		-not (Test-Path -LiteralPath 'Client\Private\Effect_AuthoringDocument.cpp')
	Add-Check 'effect.g09-authoring-world-runtime-boundary' (
		$removedEffectPathHits.Count -eq 0 -and
		$unexpectedAuthoredEffectFiles.Count -eq 0 -and
		$effectIntakeFiles.Count -eq 0 -and
		$effectShaderFiles.Count -le 1 -and
		$legacyEffectSymbolHits.Count -eq 0 -and
		$legacyEffectProjectHits.Count -eq 0 -and
		-not $legacyEffectEntry -and
		$effectG6DetailPreviewShape) "paths=$($removedEffectPathHits.Count) authoredUnexpected=$($unexpectedAuthoredEffectFiles.Count) intake=$($effectIntakeFiles.Count) shaders=$($effectShaderFiles.Count) symbols=$($legacyEffectSymbolHits.Count) project=$($legacyEffectProjectHits.Count) entry=$legacyEffectEntry detailPreview=$effectG6DetailPreviewShape"
	$effectFinalAuditPassed = $false
	$effectFinalAuditDetail = ''
	try {
		$effectFinalAuditDetail = (& .\Tools\ProjectAudit\Test-EffectToolFinal.ps1 `
			-ResourceRoot $resourceRoot 2>&1) -join ' '
		$effectFinalAuditPassed = $true
	}
	catch {
		$effectFinalAuditDetail = $_.Exception.Message
	}
	Add-Check 'effect.g09-cross-document-contract' $effectFinalAuditPassed $effectFinalAuditDetail
	$artistSourceContractPassed = $false
	$artistSourceContractDetail = ''
	try {
		$artistSourceContractDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470SourceContract.ps1' `
			2>&1 | Out-String).Trim()
		$artistSourceContractPassed =
			$artistSourceContractDetail -match
			'PASS: Artist F 31470 Source Contract cues=7 elements=35'
	}
	catch {
		$artistSourceContractDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-source-contract' `
		$artistSourceContractPassed `
		$artistSourceContractDetail
	$artistSourceExecutionPassed = $false
	$artistSourceExecutionDetail = ''
	try {
		$savedErrorActionPreference = $ErrorActionPreference
		try {
			$ErrorActionPreference = 'Continue'
			$artistSourceExecutionDetail = (& `
				'.\Tools\ProjectAudit\Test-Artist31470SourceExecutionSemantics.ps1' `
				*>&1 | Out-String -Width 4096).Trim()
			$artistSourceExecutionInvocationPassed = $?
		}
		finally {
			$ErrorActionPreference = $savedErrorActionPreference
		}
		$artistSourceExecutionPassed =
			$artistSourceExecutionInvocationPassed -and
			$artistSourceExecutionDetail -match
			'PASS: Artist F 31470 Source execution mode=shallow modules=399 ready=370 blocked=29'
	}
	catch {
		$artistSourceExecutionDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-source-execution-semantics' `
		$artistSourceExecutionPassed `
		$artistSourceExecutionDetail
	$artistCustomHandlerOraclePassed = $false
	$artistCustomHandlerOracleDetail = ''
	try {
		$savedErrorActionPreference = $ErrorActionPreference
		try {
			$ErrorActionPreference = 'Continue'
			$artistCustomHandlerOracleDetail = (& `
				'.\Tools\ProjectAudit\Test-Artist31470CustomHandlerOracle.ps1' `
				*>&1 | Out-String -Width 4096).Trim()
			$artistCustomHandlerOracleInvocationPassed = $?
		}
		finally {
			$ErrorActionPreference = $savedErrorActionPreference
		}
		$artistCustomHandlerOraclePassed =
			$artistCustomHandlerOracleInvocationPassed -and
			$artistCustomHandlerOracleDetail -match
			'PASS: Artist F 31470 custom handler oracle mode=shallow ready=370 blocked=29 distributionReady=626 distributionBlocked=3 outputOracles=0 ownerless=0 product=false'
	}
	catch {
		$artistCustomHandlerOracleDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-custom-handler-oracle' `
		$artistCustomHandlerOraclePassed `
		$artistCustomHandlerOracleDetail
	$artistSourceOracleAcquisitionPassed = $false
	$artistSourceOracleAcquisitionDetail = ''
	try {
		$artistSourceOracleAcquisitionDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470SourceOracleAcquisition.ps1' `
			| Out-String).Trim()
		$artistSourceOracleAcquisitionPassed =
			$artistSourceOracleAcquisitionDetail -match
			'PASS: Artist F 31470 Source oracle acquisition mode=shallow classes=15 families=7 blocked=29 providers=0 pilots=0 vss=permission-unchecked nextStage=NO-GO product=false'
	}
	catch {
		$artistSourceOracleAcquisitionDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-source-oracle-acquisition' `
		$artistSourceOracleAcquisitionPassed `
		$artistSourceOracleAcquisitionDetail
	$artistReconstructedSourceCapabilityPassed = $false
	$artistReconstructedSourceCapabilityDetail = ''
	try {
		$artistReconstructedSourceCapabilityDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470ReconstructedSourceCapability.ps1' `
			2>&1 | Out-String).Trim()
		$artistReconstructedSourceCapabilityPassed =
			$artistReconstructedSourceCapabilityDetail -match
			'PASS: Artist F 31470 reconstructed Source capability mode=shallow families=7 occurrences=29 properties=148 distributions=65 samples=87 unknown=0 ownerless=0 genericFallback=0 sourceExact=0 execution=false product=false'
	}
	catch {
		$artistReconstructedSourceCapabilityDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-reconstructed-source-capability' `
		$artistReconstructedSourceCapabilityPassed `
		$artistReconstructedSourceCapabilityDetail
	$artistMaterialContractPassed = $false
	$artistMaterialContractDetail = ''
	try {
		$artistMaterialContractDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470MaterialEvidenceContract.ps1' `
			2>&1 | Out-String).Trim()
		$artistMaterialContractPassed =
			$artistMaterialContractDetail -match
			'PASS: Artist F 31470 Material evidence mode=shallow recipes=27 occurrences=34'
	}
	catch {
		$artistMaterialContractDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-material-evidence-contract' `
		$artistMaterialContractPassed `
		$artistMaterialContractDetail
	$artistShaderCachePassed = $false
	$artistShaderCacheDetail = ''
	try {
		$artistShaderCacheDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470ShaderCacheOracle.ps1' `
			2>&1 | Out-String).Trim()
		$artistShaderCachePassed =
			$artistShaderCacheDetail -match
			'PASS: Artist F 31470 ShaderCache mode=shallow material=23 recipe=27 mic=25/24'
	}
	catch {
		$artistShaderCacheDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-shader-cache-oracle' `
		$artistShaderCachePassed `
		$artistShaderCacheDetail
	$artistMaterialRuntimePassed = $false
	$artistMaterialRuntimeDetail = ''
	try {
		$artistMaterialRuntimeDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470MaterialRuntimeOracle.ps1' `
			2>&1 | Out-String).Trim()
		$artistMaterialRuntimePassed =
			$artistMaterialRuntimeDetail -match
			'PASS: Artist F 31470 Material runtime mode=shallow family=23 recipe=27 occurrence=34'
	}
	catch {
		$artistMaterialRuntimeDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-material-runtime-oracle' `
		$artistMaterialRuntimePassed `
		$artistMaterialRuntimeDetail
	$artistMaterialPolicyPassed = $false
	$artistMaterialPolicyDetail = ''
	try {
		$artistMaterialPolicyDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470MaterialReconstructedPolicy.ps1' `
			2>&1 | Out-String).Trim()
		$artistMaterialPolicyPassed =
			$artistMaterialPolicyDetail -match
			'PASS: Artist F 31470 Material reconstructed policy mode=shallow rows=89\+94\+72/255'
	}
	catch {
		$artistMaterialPolicyDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-material-reconstructed-policy' `
		$artistMaterialPolicyPassed `
		$artistMaterialPolicyDetail
	$artistMaterialTextureBindingPassed = $false
	$artistMaterialTextureBindingDetail = ''
	try {
		$artistMaterialTextureBindingDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470MaterialTextureRuntimeBinding.ps1' `
			2>&1 | Out-String).Trim()
		$artistMaterialTextureBindingPassed =
			$artistMaterialTextureBindingDetail -match
			'PASS: Artist F 31470 Material texture runtime binding mode=shallow rows=68\+4/72 unique=44\+4/48 proposals=4 product=false'
	}
	catch {
		$artistMaterialTextureBindingDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-material-texture-runtime-binding' `
		$artistMaterialTextureBindingPassed `
		$artistMaterialTextureBindingDetail
	$artistMaterialRenderResourceApprovalPassed = $false
	$artistMaterialRenderResourceApprovalDetail = ''
	try {
		$artistMaterialRenderResourceApprovalDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470MaterialRenderResourceBindingApproval.ps1' `
			2>&1 | Out-String).Trim()
		$artistMaterialRenderResourceApprovalPassed =
			$artistMaterialRenderResourceApprovalDetail -match
			'PASS: Artist F 31470 Material render-resource approval recipes=27 renderer=57 ambiguous=3 descriptors=27\+18\+1/46 autocrlf=20/20\+check bytes=376183 CR=0 BOM=false'
	}
	catch {
		$artistMaterialRenderResourceApprovalDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-material-render-resource-binding-approval' `
		$artistMaterialRenderResourceApprovalPassed `
		$artistMaterialRenderResourceApprovalDetail
	$artistExactDdsDeploymentPassed = $false
	$artistExactDdsDeploymentDetail = ''
	try {
		$artistExactDdsDeploymentDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470ExactDdsRuntimeDeployment.ps1' `
			2>&1 | Out-String).Trim()
		$artistExactDdsDeploymentPassed =
			$artistExactDdsDeploymentDetail -match
			'PASS: Artist F 31470 exact DDS runtime deployment mode=shallow assets=4/4 sourceExactMaterial=false r4=false product=false'
	}
	catch {
		$artistExactDdsDeploymentDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-exact-dds-runtime-deployment' `
		$artistExactDdsDeploymentPassed `
		$artistExactDdsDeploymentDetail
	$artistGeometryContractPassed = $false
	$artistGeometryContractDetail = ''
	try {
		& '.\Tools\ProjectAudit\Test-Artist31470WModelGeometryContract.ps1' `
			-Configuration Debug
		$artistGeometryContractPassed = 0 -eq $LASTEXITCODE
		$artistGeometryContractDetail = if ($artistGeometryContractPassed) {
			'focused Debug cooker/EOL/decoder contract PASS'
		}
		else {
			"focused Debug geometry audit exit=$LASTEXITCODE"
		}
	}
	catch {
		$artistGeometryContractDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-wmodel-geometry-contract' `
		$artistGeometryContractPassed `
		$artistGeometryContractDetail
	$artistGeometryBindingPassed = $false
	$artistGeometryBindingDetail = ''
	try {
		$artistGeometryBindingDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470GeometryResourceBinding.ps1' `
			2>&1 | Out-String).Trim()
		$artistGeometryBindingPassed =
			$artistGeometryBindingDetail -match
			'PASS: Artist F 31470 GeometryBinding mode=shallow carriers=7'
	}
	catch {
		$artistGeometryBindingDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-geometry-resource-binding' `
		$artistGeometryBindingPassed `
		$artistGeometryBindingDetail
	$effectCascadeCompilerPassed = $false
	$effectCascadeCompilerDetail = ''
	try {
		$effectCascadeCompilerDetail = (& `
			'.\Tools\ProjectAudit\Test-EffectCascadeCompiler.ps1' `
			2>&1 | Out-String).Trim()
		$effectCascadeCompilerPassed =
			$effectCascadeCompilerDetail -match
			'PASS: non-executable Cascade source-inspection IR'
	}
	catch {
		$effectCascadeCompilerDetail = $_.Exception.Message
	}
	Add-Check 'effect.typed-cascade-compiler' `
		$effectCascadeCompilerPassed `
		$effectCascadeCompilerDetail
	$effectDerivedPublisherPassed = $false
	$effectDerivedPublisherDetail = ''
	try {
		$effectDerivedPublisherDetail = (& `
			'.\Tools\ProjectAudit\Test-EffectDerivedArtifactPublisher.ps1' `
			2>&1 | Out-String).Trim()
		$effectDerivedPublisherPassed =
			$effectDerivedPublisherDetail -match
			'PASS: derived Effect artifact publisher schema tests=24 reserved-reconstructed-id=true current-tools=true duplicate-json-keys=true duplicate-json-walk=true clean-checkout-lf=true reconstructed-source-id=true authenticated-blocker-union=true reconstructed-product=false rollback=true'
	}
	catch {
		$effectDerivedPublisherDetail = $_.Exception.Message
	}
	Add-Check 'effect.derived-artifact-publisher' `
		$effectDerivedPublisherPassed `
		$effectDerivedPublisherDetail
	$effectRuntimeAuthorityPassed = $false
	$effectRuntimeAuthorityDetail = ''
	try {
		$effectRuntimeAuthorityDetail = (& `
			'.\Tools\ProjectAudit\Test-EffectRuntimeAuthority.ps1' `
			2>&1 | Out-String).Trim()
		$effectRuntimeAuthorityPassed =
			$effectRuntimeAuthorityDetail -match
			'PASS: format3 immutable compiled authority'
	}
	catch {
		$effectRuntimeAuthorityDetail = $_.Exception.Message
	}
	Add-Check 'effect.runtime-compiled-authority' `
		$effectRuntimeAuthorityPassed `
		$effectRuntimeAuthorityDetail
	$artistReconstructedPolicyPassed = $false
	$artistReconstructedPolicyDetail = ''
	try {
		$artistReconstructedPolicyDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470ReconstructedApprovalPolicy.ps1' `
			2>&1 | Out-String).Trim()
		$artistReconstructedPolicyPassed =
			$artistReconstructedPolicyDetail -match
			'PASS: Artist F 31470 reconstructed approval policy tests=41 source=29 material=255 sampler=72 arithmetic=23 geometry=7 sourceExact=false execution=false product=false'
	}
	catch {
		$artistReconstructedPolicyDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-reconstructed-approval-policy' `
		$artistReconstructedPolicyPassed `
		$artistReconstructedPolicyDetail
	$artistReconstructedRuntimeProgramPassed = $false
	$artistReconstructedRuntimeProgramDetail = ''
	try {
		$artistReconstructedRuntimeProgramDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470ReconstructedRuntimeProgram.ps1' `
			2>&1 | Out-String).Trim()
		$artistReconstructedRuntimeProgramPassed =
			$artistReconstructedRuntimeProgramDetail -match
			'PASS: Artist F 31470 reconstructed runtime program tests=12 emitters=35 schedules=7 modules=399 properties=1434 leaves=1572 distributions=629 material=23/27/34/255 textures=68/72\+57 geometry=7/13 sourceExact=0 runtime=false product=false'
	}
	catch {
		$artistReconstructedRuntimeProgramDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-reconstructed-runtime-program' `
		$artistReconstructedRuntimeProgramPassed `
		$artistReconstructedRuntimeProgramDetail
	$effectComponentAuditPassed = $false
	$effectComponentAuditDetail = ''
	$effectSkillDocument = Read-Json 'Data\Balance\PlayerSkills.json'
	$dimensionMasterEffectSkills = @($effectSkillDocument.skills | Where-Object {
		$_.characterClass -eq 'DIMENSIONMASTER' -and
		$_.inputSlot -ne 'SPACE'
	})
	$missingDimensionMasterEffectMappings = @($dimensionMasterEffectSkills |
		Where-Object { [string]::IsNullOrWhiteSpace([string]$_.effectId) })
	try {
		$effectComponentAuditDetail = (& python `
			'.\Tools\LevelPlacementExtractor\build_effect_components.py' `
			'--all-product-classes' `
			'--receipt-root' `
			'.\Data\Effects\AuthoredCorrections\Generated\ComponentBuild' `
			'--verify-existing' 2>&1 | Out-String).Trim()
		if ($LASTEXITCODE -ne 0) {
			throw "WFX verifier exited with code $LASTEXITCODE`: $effectComponentAuditDetail"
		}
		$effectComponentAuditResult = $effectComponentAuditDetail |
			ConvertFrom-Json
		$allProductClassComponentsValid = $true
		foreach ($className in @('DimensionMaster','LanceMaster','Artist','Warlord')) {
			$classAudit = $effectComponentAuditResult.classes.$className
			if ($null -eq $classAudit -or
				-not [bool]$classAudit.compileIdentityComplete -or
				[int]$classAudit.effectCount -le 0 -or
				[int]$classAudit.componentCount -le 0 -or
				[int]$classAudit.emitterCount -le 0) {
				$allProductClassComponentsValid = $false
			}
		}
		$effectComponentAuditPassed =
			$missingDimensionMasterEffectMappings.Count -eq 0 -and
			[bool]$effectComponentAuditResult.compileIdentityComplete -and
			[int]$effectComponentAuditResult.effectCount -gt 0 -and
			[int]$effectComponentAuditResult.componentCount -gt 0 -and
			[int]$effectComponentAuditResult.emitterCount -gt 0 -and
			$allProductClassComponentsValid
	}
	catch {
		$effectComponentAuditDetail = $_.Exception.Message
	}
	$effectComponentAuditDetail =
		"incrementalCompile=true missingDimensionMasterMappings=$($missingDimensionMasterEffectMappings.Count) $effectComponentAuditDetail"
	Add-Check 'effect.wfx-component-assembly' `
		$effectComponentAuditPassed $effectComponentAuditDetail

	$representativeMaterializationPassed = $false
	$representativeMaterializationDetail = ''
	try {
		$representativeMaterializationJson = (& python `
			'.\Tools\EffectPipeline\materialize_representative_authored_baselines.py' `
			2>&1 | Out-String).Trim()
		if ($LASTEXITCODE -ne 0) {
			throw "Representative materializer exited with code $LASTEXITCODE."
		}
		$representativeMaterialization =
			$representativeMaterializationJson | ConvertFrom-Json
		$representativeSkills = @($representativeMaterialization.skills)
		$expectedRepresentatives = @(
			@{ characterClass = 'DIMENSIONMASTER'; skillId = 2050210; targetCount = 1 },
			@{ characterClass = 'LANCE_MASTER'; skillId = 34010; targetCount = 4 },
			@{ characterClass = 'ARTIST'; skillId = 31000; targetCount = 4 },
			@{ characterClass = 'WARLORD'; skillId = 17000; targetCount = 3 }
		)
		$representativeRowsValid = $representativeSkills.Count -eq 4
		foreach ($expected in $expectedRepresentatives) {
			$matches = @($representativeSkills | Where-Object {
				$_.characterClass -eq $expected.characterClass -and
				[int]$_.skillId -eq [int]$expected.skillId
			})
			if ($matches.Count -ne 1 -or
				[int]$matches[0].targetCount -ne [int]$expected.targetCount -or
				[int]$matches[0].materializedTargetCount -ne 0 -or
				$matches[0].status -notin @('preserveExisting', 'blocked')) {
				$representativeRowsValid = $false
			}
		}
		$preservedRepresentatives = @($representativeSkills | Where-Object {
			$_.status -eq 'preserveExisting'
		})
		$blockedRepresentatives = @($representativeSkills | Where-Object {
			$_.status -eq 'blocked'
		})
		$preservedProductGatesValid = @($preservedRepresentatives | Where-Object {
			$productGates = @($_.productGates)
			$productGates.Count -ne [int]$_.targetCount -or
			@($productGates | Where-Object { $_.status -ne 'passed' }).Count -ne 0
		}).Count -eq 0
		$blockedRowsValid = @($blockedRepresentatives | Where-Object {
			@($_.blockers).Count -eq 0 -or
			@($_.productGates).Count -ne 0 -or
			$null -eq $_.sourceDiagnostics
		}).Count -eq 0
		$dimensionMasterRepresentative = @($representativeSkills | Where-Object {
			$_.characterClass -eq 'DIMENSIONMASTER' -and
			[int]$_.skillId -eq 2050210
		})
		$dimensionMasterGate = @()
		if ($dimensionMasterRepresentative.Count -eq 1) {
			$dimensionMasterGate =
				@($dimensionMasterRepresentative[0].productGates)
		}
		$dimensionMasterBaselineValid =
			$dimensionMasterRepresentative.Count -eq 1 -and
			$dimensionMasterRepresentative[0].status -eq 'preserveExisting' -and
			$dimensionMasterGate.Count -eq 1 -and
			$dimensionMasterGate[0].status -eq 'passed' -and
			[int]$dimensionMasterGate[0].elementCount -eq 24 -and
			[int]$dimensionMasterGate[0].occurrenceCount -eq 4 -and
			[int]$dimensionMasterGate[0].kindCounts.mesh -eq 20 -and
			[int]$dimensionMasterGate[0].kindCounts.sprite -eq 4 -and
			[int]$dimensionMasterGate[0].kindCounts.particle -eq 0 -and
			[double]$dimensionMasterGate[0].spriteBillboardRollDegrees -eq -90.0 -and
			[bool]$dimensionMasterGate[0].catalogRegistered -and
			[bool]$dimensionMasterGate[0].exactCueRegistered
		$summary = $representativeMaterialization.summary
		$representativeMaterializationPassed =
			$representativeMaterialization.schema -eq
				'lostark.effect-authored-materialization-status' -and
			[int]$representativeMaterialization.version -eq 1 -and
			$representativeMaterialization.setId -eq
				'representative-four.authored-baselines' -and
			[int]$summary.skillCount -eq 4 -and
			[int]$summary.readyCount -eq 0 -and
			[int]$summary.pendingOutputCount -eq 0 -and
			[int]$summary.preservedCount + [int]$summary.blockedCount -eq 4 -and
			$representativeRowsValid -and
			$preservedProductGatesValid -and
			$blockedRowsValid -and
			$dimensionMasterBaselineValid
		$representativeMaterializationDetail =
			"preserved=$($summary.preservedCount) blocked=$($summary.blockedCount) ready=$($summary.readyCount) pending=$($summary.pendingOutputCount) dmBaseline=$dimensionMasterBaselineValid"
	}
	catch {
		$representativeMaterializationDetail = $_.Exception.Message
	}
	Add-Check 'effect.representative-authored-readiness' `
		$representativeMaterializationPassed `
		$representativeMaterializationDetail

	$fourClassAuthoredRolloutPassed = $false
	$fourClassAuthoredRolloutDetail = ''
	try {
		$fourClassAuthoredRolloutDetail = (& `
			'.\Tools\ProjectAudit\Test-FourClassAuthoredRollout.ps1' `
			2>&1 | Out-String).Trim()
		$fourClassAuthoredRolloutPassed =
			$fourClassAuthoredRolloutDetail -match
			'PASS: Four-class Authored rollout skills=51 stages=74 clips=113 effectBearing=73 silent=1 visualClips=102 derived=48 retained=53 targets=101 cues=101 Particle=0 components=[1-9][0-9]* emitters=[1-9][0-9]*'
	}
	catch {
		$fourClassAuthoredRolloutDetail = $_.Exception.Message
	}
	Add-Check 'effect.four-class-authored-clip-product-exact101' `
		$fourClassAuthoredRolloutPassed `
		$fourClassAuthoredRolloutDetail

    $wrapperHits = @($activeFiles | Select-String -Pattern 'Resources[\\/]LostArk')
    Add-Check 'source.resource-wrapper' ($wrapperHits.Count -eq 0) "hits=$($wrapperHits.Count)"

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
	$cameraGameplayGateHits = @($clientSourceFiles |
		Select-String -Pattern 'Set_GameplayInputEnabled|m_isGameplayInputEnabled')
    Add-Check 'input.official-function-keys' (
		$functionKeyHits.Count -eq 0 -and
		$f1Hits.Count -eq 1 -and
		$f6Hits.Count -eq 1) "forbidden=$($functionKeyHits.Count) f1=$($f1Hits.Count) f6=$($f6Hits.Count)"
	Add-Check 'input.camera-mode-independent-gameplay' (
		$cameraGameplayGateHits.Count -eq 0) "cameraGameplayGates=$($cameraGameplayGateHits.Count)"

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
	$renderingProfileServiceSource = Get-Content -LiteralPath `
		'Client\Private\RenderingProfileService.cpp' -Raw
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
	$characterSelectionStateSource = Get-Content -LiteralPath 'Client\Private\CharacterSelectionState.cpp' -Raw
	$packetTypeSource = Get-Content -LiteralPath 'Shared\Public\Network\PacketType.h' -Raw
	$packetMessagesSource = Get-Content -LiteralPath 'Shared\Public\Network\PacketMessages.h' -Raw
	$gameRoomSource = Get-Content -LiteralPath 'Server\Private\GameRoom.cpp' -Raw
	$networkManagerSource = Get-Content -LiteralPath 'Client\Private\NetworkManager.cpp' -Raw
	$playerControllerSourceForClassSwitch = Get-Content -LiteralPath 'Client\Private\PlayerController.cpp' -Raw
    Add-Check 'levels.character-select-contract' (
        $levelRegistrySource -match 'LEVEL::CHARACTER_SELECT' -and
		$levelRegistrySource -match 'LV_LOBBY_CLASSSELECT_SL00' -and
		$levelRegistrySource -match '\{ true, true, -792\.f, 158\.f, -750\.f, 218\.f \}' -and
		$levelRegistrySource -match 'Ready_For_CharacterSelect' -and
		$characterSelectLoaderFunction -match 'Ready_MapArea\(' -and
		$characterSelectLoaderFunction -notmatch 'Ready_Camera_Prototype\(' -and
		$characterSelectLoaderFunction -match 'Ready_Character_Rendering\(' -and
		$characterSelectLoaderFunction -notmatch 'Ready_AnimationPreviewModels\(' -and
		$characterSelectLoaderFunction -match 'CCharacterSelectionState::Try_Get_SelectedClass' -and
		$characterSelectLoaderFunction -match 'const std::array characterClasses = \{ initialClass \}' -and
		$characterSelectSource -match 'CPlayableCharacterAssetService::Ensure_Prototypes' -and
		$characterSelectSource -match 'm_MapRuntime\.Load_Area' -and
		$levelRegistrySource -match 'scene\.character-select\.warm-high-key\.v1' -and
		$mainAppSource -match 'm_RenderingProfiles\.Activate_Profile' -and
		$characterSelectSource -match 'CCharacterSelectionState::Select' -and
		$characterSelectSource -match 'MODE::CONNECTING' -and
		$characterSelectSource -notmatch 'MODE::PREVIEW|Stage_Preview|Select_Preview|Ready_Preview' -and
		$characterSelectionStateSource -notmatch 'TestEntryMode|TEST_ENTRY_MODE' -and
		$characterSelectSource -notmatch 'Request_ServerArena\(' -and
		$characterSelectSource -notmatch 'Connect_To_Server\(' -and
		$characterSelectSource -notmatch 'Send_EnterWorld\(' -and
		$characterSelectSource -notmatch 'Try_Consume_EnterAccepted' -and
		$characterSelectSource -match 'CONNECTION_TIMEOUT' -and
		$characterSelectSource -notmatch 'Return_ToPreview\(' -and
		$characterSelectSource -match 'character-select\.server-disconnect' -and
		$lobbyCommandHeaderSource -match 'LOBBY_COMMAND_PURPOSE' -and
		$lobbyCommandHeaderSource -match 'MAP_EDITOR_WORKSPACE' -and
		$characterSelectSource -match 'character-select\.enter-bern' -and
		$characterSelectSource -match 'character-select\.enter-valtan' -and
		$characterSelectSource -notmatch 'ImGui::RadioButton\("Preview"|Server Play \(Lobby-approved\)' -and
		$characterSelectSource -match 'Request_ClassChange' -and
		$characterSelectSource -match 'Request_ChangeCharacterClass' -and
		$characterSelectSource -match 'Try_Consume_CharacterClassChangeResult' -and
		$characterSelectSource -match 'm_iPendingClassChangeSequence' -and
		$characterSelectSource -notmatch 'ImGui::BeginDisabled\(true\)' -and
		$characterSelectSource -match 'ARENA_SPAWN_OPTIONS' -and
		$characterSelectSource -match 'spawn\.character-select\.monster' -and
		$characterSelectSource -match 'spawn\.character-select\.miniboss' -and
		$characterSelectSource -match 'boss\.valtan\.character-select\.lazy' -and
		$characterSelectSource -match 'ImGui::Button\("Spawn Selected"\)' -and
		$characterSelectSource -match 'Show Combat Colliders' -and
		$characterSelectSource -notmatch 'ImGui::Button\("Enter Test"\)' -and
		$characterSelectSource -match 'ImGui::Button\("Enter Bern"\)' -and
		$characterSelectSource -match 'ImGui::Button\("Enter Valtan Map"\)' -and
		$characterSelectSource -match 'CClientReplication' -and
		$characterSelectSource -match 'CNetworkPlayerCommandSink' -and
		$characterSelectSource -match 'CNetworkWorldEntityCommandSink' -and
		$characterSelectSource -match 'm_PlayerController\.Update\(' -and
		$characterSelectSource -match 'm_pCamera->Is_FollowEnabled\(\)' -and
		$characterSelectSource -notmatch 'Ready_ServerGameplayCamera\(\)' -and
		$characterSelectSource -match 'Render_SelectionPanel\(\);' -and
		$lobbySource -match 'DEFAULT_ENTRY_CLASS' -and
		$lobbySource -match 'Resolve_EntryCharacterClass' -and
		$lobbySource -match 'Send_EnterWorld\(' -and
		$lobbySource -match 'case LOBBY_STAGE::CHARACTER_SELECT:[\s\S]{0,180}WORLD_ID::CHARACTER_SELECT_ARENA' -and
		$lobbySource -notmatch 'Stage_TestEntryMode|Clear_TestEntryMode' -and
		$lobbySource -match 'accepted\.iProtocolVersion' -and
		$lobbySource -match 'accepted\.iPlayerId' -and
		$lobbySource -match 'accepted\.iNetEntityId' -and
		$frontendHarnessProject -match 'CharacterSelectionState\.cpp' -and
		$frontendHarnessProject -match 'NetObjectRegistry\.cpp' -and
		$frontendHarnessSource -match 'Test_CharacterSelectAuthorizedSelection' -and
		$frontendHarnessSource -match 'Test_NetObjectRegistryClassReplacement' -and
		$packetTypeSource -match 'NETWORK_PROTOCOL_VERSION = 14' -and
		$packetTypeSource -match 'C2S_CHANGE_CHARACTER_CLASS' -and
		$packetMessagesSource -match 'PLAYER_SNAPSHOT[\s\S]{0,180}eCharacterClass' -and
		$gameRoomSource -match 'Apply_CharacterClassChange' -and
		$gameRoomSource -match 'snapshot\.eCharacterClass = player\.eCharacterClass' -and
		$networkManagerSource -match 'S2C_CHARACTER_CLASS_CHANGE_RESULT' -and
		$replicationSource -match 'Replace_CharacterClass' -and
		$replicationSource -match 'RECOVERED_FAILURE' -and
		$playerControllerSourceForClassSwitch -match 'Rebind_LocalCharacter' -and
        $lobbySource -match '"Test"' -and
		$lobbySource -match '"Character Select"' -and
		$lobbySource -match '"Valtan"' -and
		$lobbySource -match '"Bern"') 'Character Select requires Lobby Server approval, changes class through a typed Server command, provides Server-only monster/miniboss/Valtan spawn controls, and transactionally replaces replicated presentation'
	Add-Check 'levels.character-select-camera-framing' (
		$characterSelectSource -match 'CHARACTER_SELECT_CAMERA_SIDE = 0\.4f' -and
		$characterSelectSource -match 'CHARACTER_SELECT_CAMERA_HEIGHT = 7\.5f' -and
		$characterSelectSource -match 'CHARACTER_SELECT_CAMERA_DISTANCE = 4\.5f' -and
		$characterSelectSource -match 'CHARACTER_SELECT_CAMERA_LOOK_HEIGHT = 1\.05f' -and
		$characterSelectSource -match 'CHARACTER_SELECT_CAMERA_FOV_Y = 45\.f' -and
		$characterSelectSource -match 'desc\.vLookOffset = lookOffset' -and
		$characterSelectSource -match 'Bind_CameraTarget\([\s\S]{0,100}CharacterSelectCameraPositionOffset\(\)' -and
		$characterSelectSource -notmatch 'PREVIEW_POSITION|PREVIEW_CAMERA_HEIGHT|SERVER_CAMERA_HEIGHT') 'Character Select Server Arena uses one initial framing preset and rebinds the replicated local target'
	$renderingProfiles = Read-Json `
		'Data\Rendering\Authored\RenderingProfiles.json'
	$renderingProfileIds = @($renderingProfiles.profiles | ForEach-Object {
		[string]$_.profileId
	})
	$unsupportedSceneFields = @($renderingProfiles.profiles |
		ForEach-Object { $_.PSObject.Properties.Name } |
		Where-Object { $_ -notin @(
			'profileId', 'exposureMultiplier',
			'bloomIntensityMultiplier', 'light', 'shadow') })
	Add-Check 'rendering.scene-profile-runtime-contract' (
		$renderingProfiles.schema -eq 'lostark.rendering-profiles' -and
		[int]$renderingProfiles.formatVersion -eq 1 -and
		$renderingProfileIds -contains 'scene.character-select.warm-high-key.v1' -and
		$renderingProfileIds -contains 'scene.valtan.cool-low-key.v1' -and
		$unsupportedSceneFields.Count -eq 0 -and
		$mainAppSource -match 'pRenderingProfileId' -and
		$mainAppSource -match 'Save Authored' -and
		$mainAppSource -match 'Publish Runtime' -and
		$mainAppSource -match 'Reload Runtime' -and
		$renderingProfileServiceSource -match 'OutEffective = GlobalQuality' -and
		$renderingProfileServiceSource -match 'fExposure \* Profile\.fExposureMultiplier' -and
		(Test-Path -LiteralPath 'Tools\RenderingPipeline\Publish-RenderingProfiles.ps1') -and
		(Test-Path -LiteralPath 'Tools\ProjectAudit\Test-RenderingProfiles.ps1')) `
		'descriptor-owned scene profiles use one renderer/light path with authored publish and non-cumulative multipliers'
	$renderingProfileAuditPassed = $false
	$renderingProfileAuditDetail = ''
	try {
		$renderingProfileAuditDetail = (& .\Tools\ProjectAudit\Test-RenderingProfiles.ps1 `
			-RepoRoot (Get-Location).Path 2>&1) -join ' '
		$renderingProfileAuditPassed = $true
	}
	catch {
		$renderingProfileAuditDetail = $_.Exception.Message
	}
	Add-Check 'rendering.profile-parser-contract' `
		$renderingProfileAuditPassed $renderingProfileAuditDetail
	$renderQualityAuditPassed = $false
	$renderQualityAuditDetail = ''
	try {
		$renderQualityAuditDetail = (& .\Tools\ProjectAudit\Test-RenderQualityWorkbench.ps1 `
			-RepoRoot (Get-Location).Path 2>&1) -join ' '
		$renderQualityAuditPassed = $true
	}
	catch {
		$renderQualityAuditDetail = $_.Exception.Message
	}
	Add-Check 'rendering.quality-workbench-contract' `
		$renderQualityAuditPassed $renderQualityAuditDetail
	$pointLightFalloffAuditPassed = $false
	$pointLightFalloffAuditDetail = ''
	try {
		$pointLightFalloffAuditDetail = (& .\Tools\ProjectAudit\Test-PointLightFalloff.ps1 `
			-RepoRoot (Get-Location).Path 2>&1) -join ' '
		$pointLightFalloffAuditPassed = $true
	}
	catch {
		$pointLightFalloffAuditDetail = $_.Exception.Message
	}
	if ([string]::IsNullOrWhiteSpace($pointLightFalloffAuditDetail)) {
		$pointLightFalloffAuditPassed = $false
		$pointLightFalloffAuditDetail =
			'Focused PointLight falloff audit returned no evidence detail.'
	}
	Add-Check 'rendering.point-light-falloff-contract' `
		$pointLightFalloffAuditPassed $pointLightFalloffAuditDetail
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
	$networkManagerHeader = Get-Content -LiteralPath 'Client\Public\NetworkManager.h' -Raw
    $serverMainSource = Get-Content -LiteralPath 'Server\Private\Main.cpp' -Raw
	$serverAppHeader = Get-Content -LiteralPath 'Server\Public\ServerApp.h' -Raw
	$tcpListenerHeader = Get-Content -LiteralPath 'Server\Public\TcpListener.h' -Raw
    $tcpListenerSource = Get-Content -LiteralPath 'Server\Private\TcpListener.cpp' -Raw
	$serverProjectText = Get-Content -LiteralPath 'Server\Default\Server.vcxproj' -Raw
	$teamLanEndpoint = Read-Json 'Tools\Network\TeamLanEndpoint.json'
	$teamLanSyncSource = Get-Content -LiteralPath `
		'Tools\Network\Sync-TeamLanEndpoint.ps1' -Raw
	$agentsSource = Get-Content -LiteralPath 'AGENTS.md' -Raw
	$teamLanHostPattern = [regex]::Escape(
		[string]$teamLanEndpoint.serverHost)
	$teamLanBindPattern = [regex]::Escape(
		[string]$teamLanEndpoint.serverBindAddress)
	$teamLanActiveThrough = [DateTimeOffset]::Parse(
		[string]$teamLanEndpoint.activeThroughKst,
		[Globalization.CultureInfo]::InvariantCulture)
	Add-Check 'network.team-lan-session-sync' (
		$teamLanEndpoint.schema -eq 'lostark.team-lan-endpoint' -and
		[int]$teamLanEndpoint.version -eq 1 -and
		[int]$teamLanEndpoint.port -eq 7777 -and
		[string]$teamLanEndpoint.serverBindAddress -eq '0.0.0.0' -and
		[DateTimeOffset]::Now -le $teamLanActiveThrough -and
		$teamLanSyncSource -match 'Set-ProjectUserProperty' -and
		$teamLanSyncSource -match 'Sync-HostFirewall' -and
		$teamLanSyncSource -match "ValidateSet\('Auto', 'Server', 'Client'\)" -and
		$teamLanSyncSource -match 'Machine role: \$effectiveRole' -and
		$teamLanSyncSource -match 'activeThroughKst' -and
		$agentsSource -match 'Sync-TeamLanEndpoint\.ps1' -and
		$agentsSource -match '2026-08-20 23:59 KST') `
		"host=$($teamLanEndpoint.serverHost), activeThrough=$($teamLanActiveThrough.ToString('o'))"
	Add-Check 'network.server-authorized-entry' (
        $networkManagerSource -match 'Connect_To_Server\(' -and
        $networkManagerSource -match 'InetPtonA' -and
		$networkManagerSource -match (
			'DEFAULT_SERVER_HOST\[\] = "' + $teamLanHostPattern + '"') -and
		$networkManagerHeader -match (
			'Connect_To_Server\("' + $teamLanHostPattern + '", port\)') -and
        $serverMainSource -match '--bind-address' -and
		$serverMainSource -match (
			'bindAddress = "' + $teamLanBindPattern + '"') -and
		$serverAppHeader -match (
			'bindAddress = "' + $teamLanBindPattern + '"') -and
		$tcpListenerHeader -match (
			'Open\("' + $teamLanBindPattern + '", port\)') -and
		$serverProjectText -match (
			'<LocalDebuggerCommandArguments>--bind-address ' +
			$teamLanBindPattern + '</LocalDebuggerCommandArguments>') -and
		$clientProjectText -match (
			'<LocalDebuggerEnvironment>LOSTARK_SERVER_HOST=' +
			$teamLanHostPattern + '</LocalDebuggerEnvironment>') -and
        $tcpListenerSource -match 'INADDR_ANY' -and
        $tcpListenerSource -match 'INADDR_LOOPBACK' -and
		$networkManagerSource -match 'LOSTARK_SERVER_HOST' -and
		$lobbySource -match 'CNetworkManager::Resolve_ServerHost' -and
		$lobbySource -match 'CNetworkManager::DEFAULT_SERVER_PORT' -and
		$lobbySource -notmatch '192\.168\.' -and
        $lobbySource -match 'Send_EnterWorld\(' -and
        $lobbySource -match 'Try_Consume_EnterAccepted' -and
        $lobbySource -match 'seconds\(5\)' -and
		$bernLevelSource -match 'network\.connection-lost' -and
		$valtanLevelSource -match 'network\.connection-lost' -and
		$developmentLevelSource -match 'network\.connection-lost' -and
		$characterSelectSource -match 'character-select\.server-disconnect' -and
		$characterSelectSource -match 'RETURNING_TO_LOBBY') 'Server approval is mandatory; Character Select consumes the Lobby-approved socket and every product world returns to Lobby on disconnect'

    $playerControllerSource = Get-Content -LiteralPath 'Client\Private\PlayerController.cpp' -Raw
    Add-Check 'player.command-sink-boundary' ($playerControllerSource -notmatch 'NetworkManager' -and $playerControllerSource -match 'IPlayerCommandSink') 'PlayerController depends on command sink'
	Add-Check 'player.free-camera-command-gate' (
		$playerControllerSource -match 'Update\(const bool_t gameplayCommandsEnabled\)' -and
		$playerControllerSource -match 'gameplayCommandsEnabled && isRightMouseDown' -and
		$playerControllerSource -match 'suppressKeyboard \|\| !gameplayCommandsEnabled' -and
		$bernLevelSource -match 'm_pCamera->Is_FollowEnabled\(\)' -and
		$valtanLevelSource -match 'm_pCamera->Is_FollowEnabled\(\)' -and
		$developmentLevelSource -match 'camera->Is_FollowEnabled\(\)' -and
		$characterSelectSource -match 'm_pCamera->Is_FollowEnabled\(\)') `
		'free camera synchronizes physical edges but blocks move and skill command submission in every gameplay level'
	$cameraFreeSource = Get-Content -LiteralPath 'Client\Private\Camera_Free.cpp' -Raw
	$cameraBaseSource = Get-Content -LiteralPath 'Engine\Private\Camera.cpp' -Raw
	$gameInstanceSource = Get-Content -LiteralPath 'Engine\Private\GameInstance.cpp' -Raw
	Add-Check 'camera.follow-same-frame-transform' (
		$cameraFreeSource -notmatch '(?s)Priority_Update\(f32_t fTimeDelta\).*?Update_FollowCamera\(fTimeDelta\).*?void CCamera_Free::Update\(' -and
		$cameraFreeSource -match '(?s)Late_Update\(f32_t fTimeDelta\).*?Update_FollowCamera\(fTimeDelta\).*?Update_PipeLine' -and
		$cameraFreeSource -match '(?s)Set_FollowEnabled\(bool_t isEnabled\).*?Update_FollowCamera\(0\.f\).*?Update_PipeLine' -and
		$cameraBaseSource -match 'Refresh_CameraState\(\)' -and
		$gameInstanceSource -match '(?s)Refresh_CameraState\(\).*?m_pPipeLine->Update\(\).*?m_pFrustum->Update_InWorldSpace\(\)' -and
		$networkManagerSource -match 'Try_Get_LocalSpawn' -and
		$networkManagerSource -match 'spawned\.iPlayerId == m_iLocalPlayerId' -and
		([regex]::Matches($networkManagerSource, 'm_hasLocalSpawn = false').Count -ge 4) -and
		$bernLevelSource -match 'Try_Get_LocalSpawn' -and
		$bernLevelSource -match 'cameraDesc\.fFollowResponse = 0\.f') `
		'follow camera commits the updated character transform and camera-derived state in the same frame; Bern starts from the approved local spawn when available'

    $characterLogicFiles = @(Get-ChildItem -LiteralPath 'Client\Private' -Filter 'Logic_*.cpp' -File)
    $characterLogicBoundaryHits = @($characterLogicFiles | Select-String -Pattern 'Get_DIKey|Get_DIMouse|NetworkManager|Play_Skill\(')
    $characterSpecSource = Get-Content -LiteralPath 'Client\Public\CharacterSpec.h' -Raw
    Add-Check 'character.presentation-boundary' (
        $characterLogicBoundaryHits.Count -eq 0 -and
        $characterSpecSource -match 'Update_Presentation') "forbiddenCalls=$($characterLogicBoundaryHits.Count)"

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
		$playableRoster -eq 'LANCE_MASTER,GUNSLINGER,SLAYER,ARTIST,DIMENSIONMASTER,WARLORD' -and
		$rosterStatus -eq 'supported,supported,supported,supported,supported,supported' -and
		$lobbySource -match 'CHARACTER_CLASS_ID::LANCE_MASTER' -and
		$lobbySource -match 'CHARACTER_CLASS_ID::GUNSLINGER' -and
		$lobbySource -match 'CHARACTER_CLASS_ID::SLAYER' -and
		$lobbySource -match 'CHARACTER_CLASS_ID::ARTIST' -and
		$lobbySource -match 'CHARACTER_CLASS_ID::WARLORD' -and
		$lobbySource -match 'CHARACTER_CLASS_ID::DIMENSIONMASTER' -and
		$lobbySource -notmatch 'CHARACTER_CLASS_ID::DESTROYER' -and
		$characterSelectionStateSource -match 'Is_Supported_Playable_Character_Class') "roster=$playableRoster status=$rosterStatus"
	$dimensionmasterActor = @($characterCatalog.characters | Where-Object networkClassId -eq 'DIMENSIONMASTER')
	$dimensionmasterAnimationContracts = @(
		[pscustomobject]@{ Path = 'Data\Animation\Authored\DimensionMaster\DimensionMaster.animevents'; Header = 'LOSTARK_ANIM_EVENTS 5 "DimensionMaster"' },
		[pscustomobject]@{ Path = 'Data\Animation\Reference\DimensionMaster\DimensionMaster.animnotify'; Header = 'LOSTARK_ANIM_NOTIFY 1 "DimensionMaster"' },
		[pscustomobject]@{ Path = 'Data\Animation\Reference\DimensionMaster\DimensionMaster.clipmap'; Header = 'LOSTARK_CLIP_MAP 1 "DimensionMaster"' },
		[pscustomobject]@{ Path = 'Data\Animation\Reference\DimensionMaster\DimensionMaster.clipseq'; Header = 'LOSTARK_CLIP_SEQ 2 "DimensionMaster"' },
		[pscustomobject]@{ Path = 'Data\Animation\Reference\DimensionMaster\DimensionMaster.skilltiming'; Header = 'LOSTARK_SKILL_TIMING 2 "DimensionMaster"' })
	$invalidDimensionMasterAnimationDocuments = @(
		foreach ($contract in $dimensionmasterAnimationContracts) {
			if (-not (Test-Path -LiteralPath $contract.Path -PathType Leaf)) {
				$contract.Path
				continue
			}
			if ((Get-Content -LiteralPath $contract.Path -TotalCount 1) -cnotmatch (
				'^' + [regex]::Escape($contract.Header) + ' \d+$')) {
				$contract.Path
			}
		})
	Add-Check 'actors.dimensionmaster-runtime-animation' (
		$dimensionmasterActor.Count -eq 1 -and
		[int]$characterCatalog.formatVersion -eq 2 -and
		$dimensionmasterActor[0].bodyModel -eq 'Character/DimensionMaster/DimensionMaster_Character.wmodel' -and
		@($dimensionmasterActor[0].equipmentModels).Count -eq 0 -and
		(@($dimensionmasterActor[0].weaponModels) -join ',') -eq (
			'Character/WP_WSWP_M_06/WP_WSWP_M_06L.wmodel,' +
			'Character/WP_WSWP_M_06/WP_WSWP_M_06S.wmodel,' +
			'Character/WP_WSWP_M_06/WP_WSWP_M_06P.wmodel,' +
			'Character/WP_WSWP_M_06/WP_WSWP_M_06E.wmodel') -and
		(Test-Path -LiteralPath (Join-Path $resourceRoot 'Character\DimensionMaster\DimensionMaster_Character.wmodel') -PathType Leaf) -and
		$invalidDimensionMasterAnimationDocuments.Count -eq 0) "combined body, four socketed weapon assets and owner-matched Animation Tool documents exist; invalid=$($invalidDimensionMasterAnimationDocuments -join ',')"
	$playableAssetServiceSource = Get-Content -LiteralPath 'Client\Private\PlayableCharacterAssetService.cpp' -Raw
	$valtanAssetServiceSource = Get-Content -LiteralPath 'Client\Private\ValtanPresentationAssetService.cpp' -Raw
	$npcAssetServiceSource = Get-Content -LiteralPath 'Client\Private\NpcPresentationAssetService.cpp' -Raw
	$dimensionmasterLogicSource = Get-Content -LiteralPath 'Client\Private\Logic_DimensionMaster.cpp' -Raw
	Add-Check 'actors.dimensionmaster-four-part-weapon' (
		$playableAssetServiceSource -match 'Prototype_Component_Model_DimensionMaster_Weapon_L' -and
		$playableAssetServiceSource -match 'Prototype_Component_Model_DimensionMaster_Weapon_S' -and
		$playableAssetServiceSource -match 'Prototype_Component_Model_DimensionMaster_Weapon_P' -and
		$playableAssetServiceSource -match 'Prototype_Component_Model_DimensionMaster_Weapon_E' -and
		$dimensionmasterLogicSource -match 'b_wp_swm_m_1' -and
		$dimensionmasterLogicSource -match 'b_wp_swm_m_2' -and
		$dimensionmasterLogicSource -match 'b_wp_swm_m_3' -and
		$dimensionmasterLogicSource -match 'b_wp_swm_m_4_02' -and
		$dimensionmasterLogicSource -match '180\.f') 'DimensionMaster L/S/P/E prototype tags and exact battle sockets are connected'
	Add-Check 'actors.dimensionmaster-actorx-scale' (
		$playableAssetServiceSource -match 'CHARACTER_CLASS_ID::DIMENSIONMASTER == characterClass[\s\S]{0,80}0\.01f : 0\.0001f') 'ActorX DimensionMaster uses 0.01 while legacy character packages retain 0.0001'
	$hudViewModelHeader = Get-Content -LiteralPath 'Client\Public\CombatHUDViewModel.h' -Raw
	$hudViewModelSource = Get-Content -LiteralPath 'Client\Private\CombatHUDViewModel.cpp' -Raw
	$characterSelectSource = Get-Content -LiteralPath 'Client\Private\Level_CharacterSelect.cpp' -Raw
	Add-Check 'hud.selected-class-boundary' (
		$hudViewModelHeader -match 'HUD_PLAYER_STATE[\s\S]{0,300}eCharacterClass' -and
		$hudViewModelHeader -match 'Apply_LocalPlayer' -and
		$hudViewModelSource -match 'definition\.eCharacterClass != characterClass' -and
		$hudViewModelSource -match 'Balance/PlayerProfiles\.json' -and
		$replicationSource -match 'Apply_LocalPlayer\([\s\S]{0,160}localRecord->eCharacterClass' -and
		$replicationSource -match 'Replace_CharacterClass' -and
		$mainAppSource -match 'LEVEL::CHARACTER_SELECT' -and
		$mainAppSource -match 'RenderCombatHUD\(\);') 'runtime HUD follows the Server snapshot class while filtering skill definitions by class'
	$playerSkillDocument = Read-Json 'Data\Balance\PlayerSkills.json'
	$missingQuickSlots = [Collections.Generic.List[string]]::new()
	$classQuickSlotContracts = [ordered]@{
		'LANCE_MASTER' = @('Q','W','E','R','A','S','D','F','Z','SPACE','T','V','ALT_V','LMB')
		'GUNSLINGER' = @('Q','W','E','R','A','S','D','F','T','V','ALT_V','LMB')
		'SLAYER' = @('Q','W','E','R','A','S','D','F','V','ALT_V','LMB')
		'ARTIST' = @('Q','W','E','R','A','S','D','F','T','X','Z','V','ALT_V','SPACE','LMB')
		'DIMENSIONMASTER' = @('Q','W','E','R','A','S','D','F','T','V','ALT_V','SPACE','LMB')
		'WARLORD' = @('Q','W','E','R','A','S','D','F','T','X','Z','V','ALT_V','SPACE','LMB')
	}
	foreach ($className in $classQuickSlotContracts.Keys) {
		foreach ($slotName in $classQuickSlotContracts[$className]) {
			$bindings = @($playerSkillDocument.skills | Where-Object {
				$_.characterClass -eq $className -and $_.inputSlot -eq $slotName
			})
			$distinctStances = @($bindings | Select-Object -ExpandProperty requiredStance -Unique)
			if ($bindings.Count -eq 0 -or $bindings.Count -ne $distinctStances.Count) {
				$missingQuickSlots.Add("${className}:$slotName")
			}
		}
	}
	$dimensionmasterSkillRows = @($playerSkillDocument.skills |
		Where-Object characterClass -eq 'DIMENSIONMASTER')
	Add-Check 'gameplay.playable-qw-contract' (
		$missingQuickSlots.Count -eq 0 -and
		$dimensionmasterSkillRows.Count -eq 13) "missing=$($missingQuickSlots -join ',') dimensionmasterRows=$($dimensionmasterSkillRows.Count)"

	$skillBindingOwners = [ordered]@{
		'LANCE_MASTER' = 'LanceMaster'
		'GUNSLINGER' = 'GunSlinger'
		'SLAYER' = 'Slayer'
		'ARTIST' = 'Artist'
		'DIMENSIONMASTER' = 'DimensionMaster'
		'WARLORD' = 'Warlord'
	}
	$quickSkillAnimationErrors = [Collections.Generic.List[string]]::new()
	$totalAuthoredBindings = 0
	foreach ($className in $skillBindingOwners.Keys) {
		$assetName = $skillBindingOwners[$className]
		$bindingPath = "Data\Animation\Authored\$assetName\$assetName.skillbindings.json"
		if (-not (Test-Path -LiteralPath $bindingPath -PathType Leaf)) {
			$quickSkillAnimationErrors.Add("${className}:missing authored binding document")
			continue
		}
		try {
			$bindingDocument = Read-Json $bindingPath
		}
		catch {
			$quickSkillAnimationErrors.Add("${className}:malformed authored binding document")
			continue
		}
		if ($bindingDocument.schema -ne 'lostark.animation-skill-bindings' -or
			[int]$bindingDocument.formatVersion -ne 3 -or
			$bindingDocument.animationAssetId -ne $assetName -or
			$bindingDocument.characterClass -ne $className) {
			$quickSkillAnimationErrors.Add("${className}:owner/schema mismatch")
			continue
		}
		$classSkills = @($playerSkillDocument.skills |
			Where-Object characterClass -eq $className)
		$bindings = @($bindingDocument.bindings)
		$totalAuthoredBindings += $bindings.Count
		if ($bindings.Count -ne $classSkills.Count) {
			$quickSkillAnimationErrors.Add("${className}:binding count $($bindings.Count)/$($classSkills.Count)")
		}
		foreach ($binding in $bindings) {
			if ($null -ne $binding.inputSlot -or $null -ne $binding.mode) {
				$quickSkillAnimationErrors.Add("${className}:binding duplicates gameplay authority")
			}
			$skillRows = @($classSkills | Where-Object skillId -eq $binding.skillId)
			$elements = @($binding.clips)
			# A flat clips array is one stage; a nested one is a stage per element.
			# Mixing the shapes leaves the stage count ambiguous.
			$nestedCount = @($elements | Where-Object {
				$_ -is [Array] -or $_ -is [Collections.IEnumerable] -and
					$_ -isnot [string] -and $_ -isnot [Management.Automation.PSCustomObject] }).Count
			$isNested = $nestedCount -eq $elements.Count -and $elements.Count -gt 0
			if ($nestedCount -ne 0 -and -not $isNested) {
				$quickSkillAnimationErrors.Add("${className}:$($binding.skillId) mixed stage shape")
				continue
			}
			# Built by explicit append: returning a nested array from if/else lets
			# the output stream unroll it back into a flat one.
			$stages = @()
			if ($isNested) {
				foreach ($element in $elements) { $stages += , @($element) }
			}
			else { $stages += , @($elements) }
			$clips = @($stages | ForEach-Object { $_ })
			$clipNames = @($clips | ForEach-Object {
				if ($_ -is [string]) { $_ } else { [string]$_.clip } })
			$invalidClipRows = @($clips | Where-Object {
				$_ -isnot [string] -and (
					$null -eq $_.clip -or
					($null -eq $_.playMs -and $null -eq $_.playRate) -or
					($null -ne $_.playMs -and
						([int]$_.playMs -lt 1 -or [int]$_.playMs -gt 60000)) -or
					($null -ne $_.playRate -and
						([double]$_.playRate -lt 0.05 -or [double]$_.playRate -gt 16))) })
			if ($skillRows.Count -ne 1 -or $clips.Count -lt 1 -or $clips.Count -gt 16 -or
				@($stages | Where-Object { @($_).Count -lt 1 }).Count -ne 0 -or
				$invalidClipRows.Count -ne 0 -or
				@($clipNames | Where-Object { $_ -notmatch '^[A-Za-z0-9_.-]{1,255}$' }).Count -ne 0) {
				$quickSkillAnimationErrors.Add("${className}:$($binding.skillId) invalid row")
				continue
			}
			$isStaged = $skillRows[0].skillKind -in @('COMBO', 'HOLD', 'COUNTER')
			if ($isStaged -and
				$stages.Count -ne @($skillRows[0].comboStages).Count) {
				$quickSkillAnimationErrors.Add("${className}:$($binding.skillId) combo stage count")
			}
			if (-not $isStaged -and $stages.Count -ne 1) {
				$quickSkillAnimationErrors.Add("${className}:$($binding.skillId) active must be one stage")
			}
			if ($skillRows[0].skillKind -eq 'HOLD' -and $stages.Count -ne 3) {
				$quickSkillAnimationErrors.Add("${className}:$($binding.skillId) hold needs start/loop/end")
			}
		}
		foreach ($skill in $classSkills) {
			if (@($bindings | Where-Object skillId -eq $skill.skillId).Count -ne 1) {
				$quickSkillAnimationErrors.Add("${className}:$($skill.skillId) missing/duplicate")
			}
		}
	}
	$characterRuntimeSource = Get-Content -LiteralPath 'Client\Private\Character.cpp' -Raw
	$animationToolSource = Get-Content -LiteralPath 'Client\Private\Animation_Tool.cpp' -Raw
	$animationToolHeader = Get-Content -LiteralPath 'Client\Public\Animation_Tool.h' -Raw
	Add-Check 'gameplay.playable-skill-animation-authoring-contract' (
		$quickSkillAnimationErrors.Count -eq 0 -and
		$totalAuthoredBindings -eq @($playerSkillDocument.skills).Count -and
		$characterRuntimeSource -match 'CAnimationSkillBindingDocument::Load' -and
		$characterRuntimeSource -notmatch 'Animation/Reference|\.clipseq|\.clipmap' -and
		$characterRuntimeSource -match 'm_PendingChains' -and
		$characterRuntimeSource -match 'Commit_PendingClipChains' -and
		$characterRuntimeSource -match 'Character skill presentation unavailable' -and
		$animationToolSource -match 'Create_SkillBindingDraft' -and
		$animationToolSource -match 'CPlayerSkillCatalog::Get_Skills' -and
		$animationToolSource -match 'CAnimationSkillBindingDocument::Save_Atomic' -and
		$animationToolHeader -match 'm_bSkillBindingDirty' -and
		$animationToolHeader -match 'm_bDirty') "errors=$($quickSkillAnimationErrors -join ',') authored=$totalAuthoredBindings skills=$(@($playerSkillDocument.skills).Count)"
	$actorCatalogSource = Get-Content -LiteralPath 'Client\Private\ActorCatalog.cpp' -Raw
	$actorLoaderSource = Get-Content -LiteralPath 'Client\Private\Loader.cpp' -Raw
	$playableAssetServiceSource = Get-Content -LiteralPath 'Client\Private\PlayableCharacterAssetService.cpp' -Raw
	$replicationSource = Get-Content -LiteralPath 'Client\Private\ClientReplication.cpp' -Raw
	$hardcodedActorModelHits = @(($actorLoaderSource + $playableAssetServiceSource) |
		Select-String -AllMatches -Pattern 'Character/[A-Za-z0-9_./-]+\.wmodel')
	Add-Check 'actors.runtime-catalog-boundary' (
		$actorCatalogSource -match 'Actors/CharacterCatalog\.json' -and
		$actorCatalogSource -match 'Actors/BossCatalog\.json' -and
		$actorCatalogSource -match 'Actors/NpcCatalog\.json' -and
		$playableAssetServiceSource -match 'CActorCatalog::Find_Character' -and
		$valtanAssetServiceSource -match 'CActorCatalog::Find_Boss' -and
		$valtanAssetServiceSource -match 'Add_Prototypes' -and
		$npcAssetServiceSource -match 'CActorCatalog::Find_Npc' -and
		$npcAssetServiceSource -match 'Add_Prototypes' -and
		$replicationSource -match 'CActorCatalog::Find_Boss\(spawned\.strArchetypeId\)' -and
		$replicationSource -match 'CActorCatalog::Find_Npc\(spawned\.strArchetypeId\)' -and
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
	$worldAuthoringDocuments = @(
		'Data\Worlds\LV_BER_BERNCASTLE\Gameplay.world.json',
		'Data\Worlds\LV_LUT_HEARTRB_ED\Gameplay.world.json',
		'Data\Worlds\LV_DEV_TRAINING_GROUND\Gameplay.world.json',
		'Data\Worlds\LV_LOBBY_CLASSSELECT_SL00\Gameplay.world.json')
	$invalidWorldAuthoring = @($worldAuthoringDocuments | Where-Object {
		$document = Read-Json $_
		$document.schema -ne 'lostark.world-gameplay' -or [int]$document.formatVersion -ne 4
	})
	Add-Check 'world.authoring-format-v4' ($invalidWorldAuthoring.Count -eq 0) `
		"invalid=$($invalidWorldAuthoring -join ',')"

	$bernWorldDocument = Read-Json 'Data\Worlds\LV_BER_BERNCASTLE\Gameplay.world.json'
	$bernChangeLevelTriggers = @($bernWorldDocument.placements | Where-Object {
		$_.kind -eq 'triggerBox' -and
		$_.enabled -eq $true -and
		@($_.events).Count -eq 1 -and
		$_.events[0].type -eq 'changeLevel' -and
		$_.events[0].targetWorldId -eq 'VALTAN_ARENA'
	})
	$bernLevelSource = Get-Content -LiteralPath 'Client\Private\Level_Bern.cpp' -Raw
	$loaderSource = Get-Content -LiteralPath 'Client\Private\Loader.cpp' -Raw
	Add-Check 'world.debug-change-level-trigger-presentation' (
		$bernChangeLevelTriggers.Count -eq 1 -and
		$bernChangeLevelTriggers[0].placementId -eq 'trigger.bern.to-valtan' -and
		$loaderSource -match 'LEVEL::DEVELOPMENT\)[\s\S]*LEVEL::BERN\)[\s\S]*Prototype_GameObject_TriggerBox' -and
		$bernLevelSource -match 'Ready_DebugLevelChangeTriggers' -and
		$bernLevelSource -match 'WORLD_TRIGGER_EVENT_KIND::CHANGE_LEVEL' -and
		$bernLevelSource -match 'Layer_DebugWorldGameplay' -and
		$bernLevelSource -match 'rollback\(\)') `
		"bernChangeLevelTriggers=$($bernChangeLevelTriggers.Count)"

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
	$runtimeSetRollbackPassed = $false
	$runtimeSetRollbackDetail = ''
	$runtimeSetFixtureRelative = ".codex_tmp/ProjectAuditBalanceRuntimeSet-$PID"
	$runtimeSetFixturePath = Join-Path $repoRoot $runtimeSetFixtureRelative
	try {
		$runtimeSetFiles = @(
			'Gameplay\Gameplay.bootstrap',
			'World\BERN.worldbootstrap',
			'World\VALTAN_ARENA.worldbootstrap',
			'World\TRAINING_GROUND.worldbootstrap',
			'World\CHARACTER_SELECT_ARENA.worldbootstrap')
		$baselineText = @{}
		foreach ($relative in $runtimeSetFiles) {
			$path = Join-Path $runtimeSetFixturePath $relative
			[IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($path)) | Out-Null
			$sentinel = "old-$($relative.Replace('\', '-'))"
			[IO.File]::WriteAllText($path, $sentinel)
			$baselineText[$relative] = $sentinel
		}
		$runtimeSetRollbackPassed = $true
		foreach ($failurePoint in @(3, 5)) {
			try {
				& .\Tools\GameplayPipeline\Publish-BalanceRuntimeSet.ps1 `
					-Mode Publish -OutputRoot $runtimeSetFixtureRelative `
					-FailureAfterPromote $failurePoint | Out-Null
				throw 'Balance runtime set failure injection unexpectedly succeeded.'
			}
			catch {
				if ($_.Exception.Message -eq 'Balance runtime set failure injection unexpectedly succeeded.') { throw }
			}
			foreach ($relative in $runtimeSetFiles) {
				$path = Join-Path $runtimeSetFixturePath $relative
				if ([IO.File]::ReadAllText($path) -cne $baselineText[$relative]) {
					$runtimeSetRollbackPassed = $false
				}
			}
		}
		& .\Tools\GameplayPipeline\Publish-BalanceRuntimeSet.ps1 `
			-Mode Publish -OutputRoot $runtimeSetFixtureRelative | Out-Null
		foreach ($relative in $runtimeSetFiles) {
			$path = Join-Path $runtimeSetFixturePath $relative
			if (-not [IO.File]::Exists($path) -or
				[IO.File]::ReadAllText($path) -ceq $baselineText[$relative]) {
				$runtimeSetRollbackPassed = $false
			}
		}
		$runtimeSetLeftovers = @(Get-ChildItem -LiteralPath $runtimeSetFixturePath `
			-Recurse -Force | Where-Object Name -Match 'balance-runtime-set|rollback')
		$runtimeSetRollbackPassed = $runtimeSetRollbackPassed -and
			$runtimeSetLeftovers.Count -eq 0
		$runtimeSetRollbackDetail = "leftovers=$($runtimeSetLeftovers.Count)"
	}
	catch {
		$runtimeSetRollbackDetail = $_.Exception.Message
	}
	finally {
		if ([IO.Directory]::Exists($runtimeSetFixturePath)) {
			Remove-Item -LiteralPath $runtimeSetFixturePath -Recurse -Force
		}
	}
	Add-Check 'gameplay.balance-runtime-set-rollback' `
		$runtimeSetRollbackPassed $runtimeSetRollbackDetail
	$provenanceReceipt = Read-Json 'Data\Balance\Reference\Official\2026-08-05.balance-provenance.receipt.json'
	Add-Check 'gameplay.balance-field-provenance' (
		$provenanceReceipt.schema -eq 'lostark.balance-provenance-receipt' -and
		[int]$provenanceReceipt.referenceSkillLevel -eq 10 -and
		[int]$provenanceReceipt.coverage.skillDefinitionCount -eq @($playerSkillDocument.skills).Count -and
		[int]$provenanceReceipt.coverage.fieldEntryCount -eq @($provenanceReceipt.entries).Count -and
		(Test-Path -LiteralPath 'Tools\GameplayPipeline\Export-OfficialBalanceReceipt.py') -and
		(Test-Path -LiteralPath 'Tools\GameplayPipeline\Update-BalanceProvenanceReceipt.ps1')) `
		"skills=$($provenanceReceipt.coverage.skillDefinitionCount) fields=$(@($provenanceReceipt.entries).Count)"

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
		$characterSelectFixture = Join-Path $worldFixturePath 'CHARACTER_SELECT_ARENA.worldbootstrap'
		[IO.File]::WriteAllText($bernFixture, 'original-bern')
		[IO.File]::WriteAllText($valtanFixture, 'original-valtan')
		[IO.File]::WriteAllText($trainingFixture, 'original-training')
		[IO.File]::WriteAllText($characterSelectFixture, 'original-character-select')
		foreach ($failurePoint in @(1, 4)) {
			try {
				& .\Tools\WorldPipeline\Publish-WorldGameplay.ps1 `
					-Mode Publish `
					-OutputRoot $worldFixtureRelative `
					-FailureAfterPromote $failurePoint | Out-Null
				throw 'Failure injection unexpectedly succeeded.'
			}
			catch {
				if ($_.Exception.Message -eq 'Failure injection unexpectedly succeeded.') {
					throw
				}
			}
		}
		$staleTransactionFiles = @(Get-ChildItem -LiteralPath $worldFixturePath -Force |
			Where-Object Name -Match '^\.staging\.|\.rollback\.')
		$worldRollbackPassed =
			[IO.File]::ReadAllText($bernFixture) -eq 'original-bern' -and
			[IO.File]::ReadAllText($valtanFixture) -eq 'original-valtan' -and
			[IO.File]::ReadAllText($trainingFixture) -eq 'original-training' -and
			[IO.File]::ReadAllText($characterSelectFixture) -eq 'original-character-select' -and
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

    $monsterContractFiles = @(
        'Data\Actors\MonsterCatalog.json',
        'Data\Balance\MonsterProfiles.json',
        'Data\Worlds\LV_LUT_HEARTRB_ED\SpawnGroups.world.json',
		'Data\Worlds\LV_LOBBY_CLASSSELECT_SL00\SpawnGroups.world.json',
		'Shared\Public\Gameplay\CombatCollisionContract.h',
		'Shared\Private\Gameplay\CombatCollisionContract.cpp',
        'Server\Public\SpawnGroupRuntime.h',
        'Server\Private\SpawnGroupRuntime.cpp',
        'Server\Public\MonsterBrain.h',
        'Server\Private\MonsterBrain.cpp',
        'Client\Public\MonsterPresentationAssetService.h',
        'Client\Private\MonsterPresentationAssetService.cpp')
    $missingMonsterContractFiles = @($monsterContractFiles |
        Where-Object { -not (Test-Path -LiteralPath $_) })
    $legacyMonsterRuntimeHits = @(
        Select-String -LiteralPath @(
            'Server\Private\GameRoom.cpp',
            'Client\Private\ClientReplication.cpp',
            'Client\Private\MonsterPresentationAssetService.cpp') `
            -Pattern '#include\s+"Monster\.h"|CMonster::Create|Logic_Monster')
    $spawnGroupPublisherSource = Get-Content -LiteralPath 'Tools\WorldPipeline\Publish-WorldGameplay.ps1' -Raw
	$spawnGroupRuntimeSource = Get-Content -LiteralPath 'Server\Private\SpawnGroupRuntime.cpp' -Raw
    $staleWorldPublishFiles = @(Get-ChildItem -LiteralPath 'Server\Bin\DataFiles\World' -File -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -match '\.(tmp|rollback)\.' })
    Add-Check 'world.monster-spawn-group-contract' (
        $missingMonsterContractFiles.Count -eq 0 -and
        $legacyMonsterRuntimeHits.Count -eq 0 -and
        $spawnGroupPublisherSource -match 'SpawnGroups\.world\.json' -and
		$spawnGroupPublisherSource -match 'spawngroupsbootstrap' -and
		$gameRoomSource -match 'WORLD_ENTITY_SPAWN_RESULT::ACTIVATED' -and
		$gameRoomSource -match 'Reset_CharacterSelectArenaWhenEmpty' -and
		$spawnGroupRuntimeSource -match 'Activate_Immediate' -and
		$replicationSource -match 'Set_CombatColliderDebugVisible') "missing=$($missingMonsterContractFiles -join ',') legacyRuntimeHits=$($legacyMonsterRuntimeHits.Count)"
    Add-Check 'world.publish-cleanup' ($staleWorldPublishFiles.Count -eq 0) "stale=$($staleWorldPublishFiles.Name -join ',')"

    $serverRoomSource = Get-Content -LiteralPath 'Server\Private\GameRoom.cpp' -Raw
    $serverProjectSource = Get-Content -LiteralPath 'Server\Default\Server.vcxproj' -Raw
	$mapToolSource = Get-Content -LiteralPath 'Client\Private\MapTool.cpp' -Raw
	Add-Check 'world.player-spawn-position-authoring' (
		$mapToolSource -match 'Player Spawn Position Offset' -and
		$mapToolSource -match 'Apply Delta To Spawn Position' -and
		$mapToolSource -match 'staged\.position\.x \+= m_WorldPlacementPositionDelta\.x' -and
		$serverRoomSource -match 'player\.fPositionX = spawn->fPositionX' -and
		$serverRoomSource -match 'player\.fPositionY = spawn->fPositionY' -and
		$serverRoomSource -match 'player\.fPositionZ = spawn->fPositionZ') 'MapTool resolves authored position delta and Server consumes the saved spawn transform'
	Add-Check 'server.world-bootstrap-boundary' (
		$serverRoomSource -match 'Find_AvailablePlayerSpawn' -and
		$serverRoomSource -match 'Update_WorldEntities' -and
		$serverRoomSource -match 'Find_BossPatterns' -and
		$serverRoomSource -notmatch 'placement\.iPatternTelegraphMs' -and
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
	Add-Check 'debug.character-select-skill-audition' (
		$serverRoomSource -match 'CHARACTER_SELECT_AUDITION_COOLDOWN_TICKS\s*=\s*90u' -and
		$serverRoomSource -match 'Add_ServerTicksSkippingReservedZero[\s\S]*?SERVER_TICK_CARDINALITY[\s\S]*?startTick - 1u[\s\S]*?elapsedTicks[\s\S]*?\+ 1u' -and
		$serverRoomSource -match 'static_assert\(90u\s*==\s*Add_ServerTicksSkippingReservedZero\([\s\S]*?numeric_limits<std::uint32_t>::max[\s\S]*?90u\)\)' -and
		$serverRoomSource -match 'static_assert\(1u\s*==\s*Add_ServerTicksSkippingReservedZero\([\s\S]*?numeric_limits<std::uint32_t>::max[\s\S]*?- 89u[\s\S]*?90u\)\)' -and
		$serverRoomSource -match '#ifdef _DEBUG[\s\S]*?WORLD_ID::CHARACTER_SELECT_ARENA == m_eWorldId[\s\S]*?iCurrentResource\s*=\s*[\s\S]*?iMaximumResource[\s\S]*?#endif[\s\S]*?m_PlayerSkillSystem\.Try_Start[\s\S]*?CooldownEndTickBySkillId\.insert_or_assign\([\s\S]*?Add_ServerTicksSkippingReservedZero\([\s\S]*?CHARACTER_SELECT_AUDITION_COOLDOWN_TICKS' -and
		$serverRoomSource -notmatch 'CooldownEndTickBySkillId\.erase\(useSkill\.iSkillId\)' -and
		$serverRoomSource -notmatch 'command\.iSkillId\s*==\s*2050210') `
		'Debug Character Select Server Arena uses a Server-authoritative three-second audition cooldown with full resources'
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
		$clientReplicationSource -match 'Apply_Boss' -and
		$clientReplicationSource -match 'Apply_DamageEvents') 'HUD consumes server snapshot plus validated balance definitions without packets'
	$clientProjectSource = Get-Content -LiteralPath 'Client\Default\Client.vcxproj' -Raw
	$balanceToolSource = Get-Content -LiteralPath 'Client\Private\BalanceTool.cpp' -Raw
	$balanceRuntimePublisherSource = Get-Content -LiteralPath 'Tools\GameplayPipeline\Publish-BalanceRuntimeSet.ps1' -Raw
	$gameplayCatalogSource = Get-Content -LiteralPath 'Server\Private\GameplayCatalog.cpp' -Raw
	$valtanBrainSource = Get-Content -LiteralPath 'Server\Private\ValtanBrain.cpp' -Raw
	Add-Check 'debug.balance-tool-contract' (
		$clientProjectSource -match 'BalanceTool\.cpp' -and
		$balanceToolSource -match 'Publish-BalanceRuntimeSet\.ps1' -and
		$balanceToolSource -match 'Update-BalanceProvenanceReceipt\.ps1' -and
		$balanceRuntimePublisherSource -match 'Publish-GameplayBalance\.ps1' -and
		$balanceRuntimePublisherSource -match 'Publish-WorldGameplay\.ps1' -and
		$balanceRuntimePublisherSource -match 'FailureAfterPromote' -and
		$balanceToolSource -match 'CREATE_NO_WINDOW' -and
		$balanceToolSource -match 'TerminateProcess') `
		'F1 Balance Tool saves authoring, synchronizes provenance, validates/publishes, and bounds its owned subprocess'
	Add-Check 'gameplay.defense-consumer' (
		$gameplayCatalogSource -match 'Apply_Defense' -and
		$valtanBrainSource -match 'playerProfile->iDefense') `
		'player defense is consumed by the centralized incoming damage curve'
	$bossProfileDocument = Read-Json 'Data\Balance\BossProfiles.json'
	$valtanEncounterDocument = Read-Json 'Data\Encounters\Valtan\ValtanEncounter.json'
	$valtanPatternIds = @($valtanEncounterDocument.patterns | ForEach-Object { [string]$_.patternId })
	Add-Check 'gameplay.valtan-health-bar-pattern-contract' (
		[int]$bossProfileDocument.formatVersion -eq 3 -and
		[int]$bossProfileDocument.bosses[0].maximumHealthBars -eq 160 -and
		[int]$valtanEncounterDocument.formatVersion -eq 3 -and
		$valtanPatternIds.Count -eq 31 -and
		$valtanPatternIds -contains 'VALTAN_FLOOR_WIPE_130' -and
		$valtanPatternIds -contains 'VALTAN_FOUR_PILLARS_105' -and
		$valtanPatternIds -contains 'VALTAN_ARENA_BREAK_80' -and
		$valtanPatternIds -contains 'VALTAN_MAGIC_ORB_STAGGER_76' -and
		$valtanPatternIds -contains 'VALTAN_CENTER_GRAB_COUNTER_64' -and
		$valtanPatternIds -contains 'VALTAN_ARENA_BREAK_33' -and
		$valtanPatternIds -contains 'VALTAN_GHOST_TRANSITION_15' -and
		$gameplayCatalogSource -match 'PATTERNSTAGE' -and
		$valtanBrainSource -match 'QueueCrossedHealthBarPatterns' -and
		$valtanBrainSource -match 'BOSS_PATTERN_HIT_SHAPE::RING' -and
		$valtanBrainSource -match 'BOSS_PATTERN_HIT_SHAPE::CONE' -and
		$valtanBrainSource -match 'BOSS_PATTERN_HIT_SHAPE::BOX' -and
		$valtanBrainSource -match 'BOSS_PATTERN_HIT_SHAPE::CROSS') `
		'Valtan 160-bar authoring, 31 staged patterns, ordered thresholds, and Server collider hits share one runtime path'
	$serverPlayerSource = Get-Content -LiteralPath 'Server\Public\ServerPlayer.h' -Raw
	$gameRoomSource = Get-Content -LiteralPath 'Server\Private\GameRoom.cpp' -Raw
	Add-Check 'gameplay.valtan-entry-protection-revive-contract' (
		$serverPlayerSource -match 'isCombatReady' -and
		$gameRoomSource -match 'Handle_RevivePlayer' -and
		$gameRoomSource -match 'WORLD_ID::VALTAN_ARENA' -and
		$packetMessagesSource -match 'C2S_REVIVE_PLAYER' -and
		$balanceToolSource -match 'Revive at death position') `
		'Valtan entry protection and same-position revive remain Server-authoritative and Balance Tool-addressable'
	Add-Check 'ui.combat-font-hud-contract' (
		$mainAppSource -match 'RenderCombatHUDText' -and
		$mainAppSource -match 'Font_YG330' -and
		$mainAppSource -match 'iMaximumHealthBars' -and
		$mainAppSource -notmatch 'ImGui::ProgressBar' -and
		$mainAppSource -notmatch '##RuntimeCombatHUD') `
		'player HP/Mana and Valtan HP/bar text use Font Manager after the authored HUD without the old ImGui meter window'

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
